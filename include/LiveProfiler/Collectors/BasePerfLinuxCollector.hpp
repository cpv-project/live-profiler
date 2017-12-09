#pragma once
#include <linux/perf_event.h>
#include <cassert>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include "BaseCollector.hpp"
#include "../Utils/Allocators/FreeListAllocator.hpp"
#include "../Utils/Allocators/SingletonAllocator.hpp"
#include "../Utils/Platform/Linux/LinuxEpollDescriptor.hpp"
#include "../Utils/Platform/Linux/LinuxPerfEntry.hpp"
#include "../Utils/Platform/Linux/LinuxPerfUtils.hpp"
#include "../Utils/Platform/Linux/LinuxProcessUtils.hpp"

namespace LiveProfiler {
	/**
	 * The base class for the collector that use perf_events on linux to monitor processes.
	 * Child class should provide perfType, perfConfig, sampleType to base constructor.
	 * Child class should implement function takeSamples.
	 */
	template <class Model>
	class BasePerfLinuxCollector : public BaseCollector<Model> {
	public:
		/** Default parameters */
		static const std::size_t DefaultMaxFreeResult = 1024;
		static const std::size_t DefaultMaxFreePerfEntry = 1024;
		static const std::size_t DefaultThreadsUpdateInterval = 100;
		static const std::size_t DefaultSamplePeriod = 100000;
		static const std::size_t DefaultMmapPageCount = 8;
		static const std::size_t DefaultWakeupEvents = 8;

		/** Reset the state to it's initial state */
		void reset() override {
			// clear all monitoring threads
			for (auto& pair : tidToPerfEntry_) {
				unmonitorThread(std::move(pair.second));
			}
			tidToPerfEntry_.clear();
			threads_.clear();
			// reset last threads updated time
			threadsUpdated_ = {};
			// reset enabled
			enabled_ = false;
			// the filter will remain because it's set externally
		}

		/** Enable performance data collection */
		void enable() override {
			// reset and enable all perf events, ignore any errors
			for (auto& pair : tidToPerfEntry_) {
				assert(pair.second != nullptr);
				LinuxPerfUtils::perfEventEnable(pair.second->getFd(), true);
			}
			// all newly monitored threads should call perfEventEnable
			enabled_ = true;
		}

		/** Collect performance data for the specified timeout period */
		std::vector<std::unique_ptr<Model>>& collect(
			std::chrono::high_resolution_clock::duration timeout) & override {
			// update the threads to monitor every specified interval
			auto now = std::chrono::high_resolution_clock::now();
			if (now - threadsUpdated_ > threadsUpdateInterval_) {
				threads_.clear();
				LinuxProcessUtils::listProcesses(threads_, filter_, true);
				updatePerfEvents();
				threadsUpdated_ = now;
			}
			// clear results
			for (auto& result : results_) {
				resultAllocator_.deallocate(std::move(result));
			}
			results_.clear();
			// poll events
			auto& events = epoll_.wait(timeout);
			for (auto& event : events) {
				// get entry by tid
				pid_t tid = static_cast<pid_t>(event.data.u64);
				auto it = tidToPerfEntry_.find(tid);
				if (it == tidToPerfEntry_.end()) {
					// thread no longer be monitored
					continue;
				}
				// check events
				if ((event.events & EPOLLIN) == EPOLLIN) {
					// take samples
					takeSamples(it->second);
				} else if ((event.events & (EPOLLERR | EPOLLHUP)) != 0) {
					// thread no longer exist
					unmonitorThread(std::move(it->second));
					tidToPerfEntry_.erase(it);
				}
			}
			return results_;
		}

		/** Disable performance data collection */
		void disable() override {
			// disable all perf events, ignore any errors
			for (auto& pair : tidToPerfEntry_) {
				assert(pair.second != nullptr);
				LinuxPerfUtils::perfEventDisable(pair.second->getFd());
			}
			// reset enabled
			enabled_ = false;
		}

	public:
		/** Set how often to update the list of processes */
		template <class Rep, class Period>
		void setProcessesUpdateInterval(std::chrono::duration<Rep, Period> interval) {
			threadsUpdateInterval_ = std::chrono::duration_cast<
				std::decay_t<decltype(threadsUpdateInterval_)>>(interval);
		}

		/** Use the specified function to decide which processes to monitor */
		void filterProcessBy(const std::function<bool(pid_t)>& filter) {
			filter_ = filter;
		}

		/** Use the specified process name to decide which processes to monitor */
		void filterProcessByName(const std::string& name) {
			filterProcessBy(LinuxProcessUtils::getProcessFilterByName(name));
		}

		/**
		 * Set how often to take a sample, the unit is cpu clock.
		 * Default value is DefaultSamplePeriod.
		 */
		void setSamplePeriod(std::uint64_t samplePeriod) {
			samplePeriod_ = samplePeriod;
		}

		/**
		 * Set how many pages for the mmap ring buffer,
		 * this count is not contains metadata page, and should be power of 2.
		 * Default value is DefaultMmapPageCount.
		 */
		void setMmapPageCount(std::size_t mmapPageCount) {
			mmapPageCount_ = mmapPageCount;
		}

		/**
		 * Set the number of records required to raise an event.
		 * A larger value may improve performance but delay the collection.
		 * Default value is DefaultWakeupEvents.
		 */
		void setWakeupEvents(std::uint32_t wakeupEvents) {
			wakeupEvents_ = wakeupEvents;
		}

		/**
		 * Set whether to exclude samples in user space.
		 * Default value is false.
		 */
		void setExcludeUser(bool excludeUser) {
			excludeUser_ = excludeUser;
		}

		/**
		 * Set whether to exclude samples in kernel space.
		 * Default value is true.
		 */
		void setExcludeKernel(bool excludeKernel) {
			excludeKernel_ = excludeKernel;
		}

		/**
		 * Set whether to exclude samples in hypervisor.
		 * Default value is true.
		 */
		void setExcludeHypervisor(bool excludeHypervisor) {
			excludeHypervisor_ = excludeHypervisor;
		}

		/** Constructor */
		BasePerfLinuxCollector(
			std::uint32_t perfType,
			std::uint64_t perfConfig,
			std::uint64_t sampleType) :
			results_(),
			resultAllocator_(DefaultMaxFreeResult),
			filter_(),
			threads_(),
			threadsUpdated_(),
			threadsUpdateInterval_(
				std::chrono::milliseconds(+DefaultThreadsUpdateInterval)),
			tidToPerfEntry_(),
			perfEntryAllocator_(DefaultMaxFreePerfEntry),
			perfType_(perfType),
			perfConfig_(perfConfig),
			samplePeriod_(DefaultSamplePeriod),
			sampleType_(sampleType),
			mmapPageCount_(DefaultMmapPageCount),
			wakeupEvents_(DefaultWakeupEvents),
			excludeUser_(false),
			excludeKernel_(true),
			excludeHypervisor_(true),
			enabled_(false),
			epoll_() { }

	protected:
		/** Update the threads to monitor based on `threads_` */
		void updatePerfEvents() {
			std::sort(threads_.begin(), threads_.end());
			// find out which threads newly created
			for (pid_t tid : threads_) {
				if (tidToPerfEntry_.find(tid) != tidToPerfEntry_.end()) {
					continue;
				}
				tidToPerfEntry_.emplace(tid, monitorThread(tid));
			}
			// find out which threads no longer exist and clear tidToPerfEntry_
			for (auto it = tidToPerfEntry_.begin(); it != tidToPerfEntry_.end();) {
				pid_t tid = it->first;
				if (std::binary_search(threads_.cbegin(), threads_.cend(), tid)) {
					// thread still exist
					++it;
				} else {
					// thread no longer exist
					unmonitorThread(std::move(it->second));
					it = tidToPerfEntry_.erase(it);
				}
			}
		}

		/** Monitor specified thread, will not access tidToPerfEntry_ */
		std::unique_ptr<LinuxPerfEntry> monitorThread(pid_t tid) {
			// open perf event
			auto entry = perfEntryAllocator_.allocate();
			entry->setPid(tid);
			LinuxPerfUtils::monitorSample(
				entry,
				perfType_,
				perfConfig_,
				samplePeriod_,
				sampleType_,
				mmapPageCount_,
				wakeupEvents_,
				excludeUser_,
				excludeKernel_,
				excludeHypervisor_);
			// enable events if collecting
			if (enabled_) {
				LinuxPerfUtils::perfEventEnable(entry->getFd(), true);
			}
			// register to epoll, use edge trigger and associated data is tid
			epoll_.add(entry->getFd(), EPOLLIN | EPOLLET, static_cast<std::uint64_t>(tid));
			return entry;
		}

		/** Unmonitor specified thread, will not access tidToPerfEntry_ */
		void unmonitorThread(std::unique_ptr<LinuxPerfEntry>&& entry) {
			assert(entry != nullptr);
			// unregister from epoll
			epoll_.del(entry->getFd());
			// disable events
			LinuxPerfUtils::perfEventDisable(entry->getFd());
			// return instance to allocator
			perfEntryAllocator_.deallocate(std::move(entry));
		}

	protected:
		/** Take samples from perf entry and append result to results_ */
		virtual void takeSamples(std::unique_ptr<LinuxPerfEntry>& entry) = 0;

	protected:
		std::vector<std::unique_ptr<Model>> results_;
		FreeListAllocator<Model> resultAllocator_;

		std::function<bool(pid_t)> filter_;
		std::vector<pid_t> threads_;
		std::chrono::high_resolution_clock::time_point threadsUpdated_;
		std::chrono::high_resolution_clock::duration threadsUpdateInterval_;

		std::unordered_map<pid_t, std::unique_ptr<LinuxPerfEntry>> tidToPerfEntry_;
		FreeListAllocator<LinuxPerfEntry> perfEntryAllocator_;

		std::uint32_t perfType_;
		std::uint64_t perfConfig_;
		std::uint64_t samplePeriod_;
		std::uint64_t sampleType_;
		std::size_t mmapPageCount_;
		std::uint32_t wakeupEvents_;
		bool excludeUser_;
		bool excludeKernel_;
		bool excludeHypervisor_;
		bool enabled_;

		LinuxEpollDescriptor epoll_;
	};
}

