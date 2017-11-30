#pragma once
#include <linux/perf_event.h>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include "BaseCollector.hpp"
#include "../Models/CpuSampleModel.hpp"
#include "../Utils/LinuxProcessUtils.hpp"
#include "../Utils/LinuxPerfUtils.hpp"

namespace LiveProfiler {
	/**
	 * Collector for collecting cpu samples on linux, based on perf_events
	 */
	class CpuSampleLinuxCollector : public BaseCollector<CpuSampleModel> {
	public:
		/** Default mmap buffer pages for perf_events */
		static const std::size_t DefaultMmapBufferPages = 8;

		/** Reset the state to it's initial state */
		void reset() override {
			// TODO
		}

		/** Enable performance data collection */
		void enable() override {
			// TODO
		}

		/** Collect performance data for the specified timeout period */
		const std::vector<CpuSampleModel>& collect(
			std::chrono::high_resolution_clock::duration timeout) override {
			// update the processes to monitor every specified interval
			auto now = std::chrono::high_resolution_clock::now();
			if (now - processesUpdated_ > processesUpdateInterval_) {
				LinuxProcessUtils::listProcesses(processes_, filter_);
				updatePerfEvents();
				processesUpdated_ = now;
			}
			// poll events
			// - read from mmap
			// - convert to model
			// TODO
			return result_;
		}

		/** Disable performance data collection */
		void disable() override {
			// TODO
		}

		/** Constructor */
		CpuSampleLinuxCollector() :
			result_(),
			filter_(),
			processes_(),
			processesUpdated_(),
			processesUpdateInterval_(std::chrono::milliseconds(100)),
			pidToPerfEntry_(),
			perfEntryAllocator_(DefaultMmapBufferPages) { }

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

	protected:
		/** Update the processes to monitor based on the latest list */
		void updatePerfEvents() {
			std::sort(processes_.begin(), processes_.end());
			// find out which processes newly created
			for (pid_t pid : processes_) {
				if (pidToPerfEntry_.find(pid) != pidToPerfEntry_.end()) {
					continue;
				}
				// monitor this process
			}
			// find out which processes no longer exist
		}

	protected:
		std::vector<CpuSampleModel> result_;
		std::function<bool(pid_t)> filter_;
		std::vector<pid_t> processes_;
		std::chrono::high_resolution_clock::time_point processesUpdated_;
		std::chrono::high_resolution_clock::duration processesUpdateInterval_;
		std::unordered_map<pid_t, std::unique_ptr<LinuxPerfEntry>> pidToPerfEntry_;
		LinuxPerfEntryAllocator perfEntryAllocator_;
	};
}

