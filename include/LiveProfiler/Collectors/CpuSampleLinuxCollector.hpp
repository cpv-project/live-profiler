#pragma once
#include <linux/perf_event.h>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include "BaseCollector.hpp"
#include "../Models/CpuSampleModel.hpp"
#include "../Utils/Allocators/FreeListAllocator.hpp"
#include "../Utils/LinuxProcessUtils.hpp"
#include "../Utils/LinuxPerfEntry.hpp"
#include "../Utils/LinuxPerfUtils.hpp"
#include "../Utils/LinuxEpollDescriptor.hpp"

namespace LiveProfiler {
	/**
	 * Collector for collecting cpu samples on linux, based on perf_events
	 */
	class CpuSampleLinuxCollector : public BaseCollector<CpuSampleModel> {
	public:
		/** Default parameters */
		static const std::size_t DefaultMmapPageCount = 16;
		static const std::size_t DefaultSamplePeriod = 100000;
		static const std::size_t DefaultMaxFreePerfEntry = 1024;

		/** Reset the state to it's initial state */
		void reset() override {
			// clear all monitoring processes
			for (auto& pair : pidToPerfEntry_) {
				unmonitorProcess(std::move(pair.second));
			}
			pidToPerfEntry_.clear();
			processes_.clear();
			// reset last processes updated time
			processesUpdated_ = {};
			// reset enabled
			enabled_ = false;
			// the filter will remain because it's set externally
		}

		/** Enable performance data collection */
		void enable() override {
			// reset and enable all perf events, ignore any errors
			for (auto& pair : pidToPerfEntry_) {
				LinuxPerfUtils::perfEventEnable(pair.second->getFd(), true);
			}
			// all newly monitored processes should call perfEventEnable
			enabled_ = true;
		}

		/** Collect performance data for the specified timeout period */
		const std::vector<CpuSampleModel>& collect(
			std::chrono::high_resolution_clock::duration timeout) & override {
			// update the processes to monitor every specified interval
			auto now = std::chrono::high_resolution_clock::now();
			if (now - processesUpdated_ > processesUpdateInterval_) {
				processes_.clear();
				LinuxProcessUtils::listProcesses(processes_, filter_, true);
				updatePerfEvents();
				processesUpdated_ = now;
			}
			// poll events
			auto& events = epoll_.wait(timeout);
			for (auto& event : events) {
				// get entry by pid
				pid_t pid = static_cast<pid_t>(event.data.u64);
				auto it = pidToPerfEntry_.find(pid);
				if (it == pidToPerfEntry_.end()) {
					// process no longer be monitored
					continue;
				}
				// check events
				if ((event.events & EPOLLIN) == EPOLLIN) {
					// take a sample
					takeSample(it->second);
				} else if ((event.events & (EPOLLERR | EPOLLHUP)) != 0) {
					// process no longer exist
					unmonitorProcess(std::move(it->second));
					pidToPerfEntry_.erase(it);
				}
			}
			return result_;
		}

		/** Disable performance data collection */
		void disable() override {
			// disable all perf events, ignore any errors
			for (auto& pair : pidToPerfEntry_) {
				LinuxPerfUtils::perfEventDisable(pair.second->getFd());
			}
			// reset enabled
			enabled_ = false;
		}

		/** Set how often to take a sample, the unit is cpu clock */
		void setSamplePeriod(std::size_t samplePeriod) {
			samplePeriod_ = samplePeriod;
		}

		/** Set how many pages for mmap ring buffer,
		 * this count is not contains metadata page, and should be power of 2.
		 */
		void setMmapPageCount(std::size_t mmapPageCount) {
			mmapPageCount_ = mmapPageCount;
		}

		/** Set how often to update the list of processes */
		template <class Rep, class Period>
		void setProcessesUpdateInterval(std::chrono::duration<Rep, Period> interval) {
			processesUpdateInterval_ = std::chrono::duration_cast<
				std::decay_t<decltype(processesUpdateInterval_)>>(interval);
		}

		/** Use the specified function to decide which processes to monitor */
		void filterProcessBy(const std::function<bool(pid_t)>& filter) {
			filter_ = filter;
		}

		/** Use the specified process name to decide which processes to monitor */
		void filterProcessByName(const std::string& name) {
			filterProcessBy(LinuxProcessUtils::getProcessFilterByName(name));
		}

		/** Constructor */
		CpuSampleLinuxCollector() :
			result_(),
			filter_(),
			processes_(),
			processesUpdated_(),
			processesUpdateInterval_(std::chrono::milliseconds(100)),
			pidToPerfEntry_(),
			perfEntryAllocator_(DefaultMaxFreePerfEntry),
			samplePeriod_(DefaultSamplePeriod),
			mmapPageCount_(DefaultMmapPageCount),
			enabled_(false),
			epoll_() { }

	protected:
		/** Update the processes to monitor based on the latest list */
		void updatePerfEvents() {
			std::sort(processes_.begin(), processes_.end());
			// find out which processes newly created
			for (pid_t pid : processes_) {
				if (pidToPerfEntry_.find(pid) != pidToPerfEntry_.end()) {
					continue;
				}
				pidToPerfEntry_.emplace(pid, monitorProcess(pid));
			}
			// find out which processes no longer exist
			for (auto it = pidToPerfEntry_.begin(); it != pidToPerfEntry_.end();) {
				pid_t pid = it->first;
				if (std::binary_search(processes_.cbegin(), processes_.cend(), pid)) {
					// process still exist
					++it;
				} else {
					// process no longer exist
					unmonitorProcess(std::move(it->second));
					it = pidToPerfEntry_.erase(it);
				}
			}
		}

		/** Monitor specified process, will not access pidToPerfEntry_ */
		std::unique_ptr<LinuxPerfEntry> monitorProcess(pid_t pid) {
			// open perf event
			auto entry = perfEntryAllocator_.allocate();
			entry->setPid(pid);
			LinuxPerfUtils::monitorSample(
				entry, samplePeriod_, mmapPageCount_,
				PERF_SAMPLE_IP | PERF_SAMPLE_TID | PERF_SAMPLE_CALLCHAIN);
			// enable events if collecting
			if (enabled_) {
				LinuxPerfUtils::perfEventEnable(entry->getFd(), true);
			}
			// register to epoll, use edge trigger and associated data is pid
			epoll_.add(entry->getFd(), EPOLLIN | EPOLLET, static_cast<std::uint64_t>(pid));
			return entry;
		}

		/** Unmonitor specified process, will not access pidToPerfEntry_ */
		void unmonitorProcess(std::unique_ptr<LinuxPerfEntry>&& entry) {
			// unregister from epoll
			epoll_.del(entry->getFd());
			// disable events
			LinuxPerfUtils::perfEventDisable(entry->getFd());
			// return instance to allocator
			perfEntryAllocator_.deallocate(std::move(entry));
		}

		/** Take a sample for executing instruction (actually is the next instruction) */
		void takeSample(std::unique_ptr<LinuxPerfEntry>& entry) {
			auto data = entry->getData<CpuSampleRawData>();
			// check type
			if (data->header.type != PERF_RECORD_SAMPLE) {
				return;
			}
			// TODO
			// find function symbol from instruction pointer
			std::cout << "pid: " << data->pid <<
				" tid: " << data->tid <<
				" ip: 0x" << std::hex << data->ip << std::dec <<
				" nr: " << data->nr << std::endl;
		}

		/** See man perf_events, section PERF_RECORD_SAMPLE */
		struct CpuSampleRawData {
			::perf_event_header header;
			std::uint64_t ip;
			std::uint32_t pid;
			std::uint32_t tid;
			std::uint64_t nr;
			std::uint64_t ips[];
		};

	protected:
		std::vector<CpuSampleModel> result_;
		std::function<bool(pid_t)> filter_;
		std::vector<pid_t> processes_;
		std::chrono::high_resolution_clock::time_point processesUpdated_;
		std::chrono::high_resolution_clock::duration processesUpdateInterval_;
		std::unordered_map<pid_t, std::unique_ptr<LinuxPerfEntry>> pidToPerfEntry_;
		FreeListAllocator<LinuxPerfEntry> perfEntryAllocator_;
		std::size_t samplePeriod_;
		std::size_t mmapPageCount_;
		bool enabled_;
		LinuxEpollDescriptor epoll_;
	};
}

