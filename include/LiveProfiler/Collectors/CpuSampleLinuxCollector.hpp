#pragma once
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <cstdlib>
#include <cstdio>
#include <array>
#include <atomic>
#include <chrono>
#include <functional>
#include "BaseCollector.hpp"
#include "../Models/CpuSampleModel.hpp"
#include "../Exceptions/ProfilerException.hpp"

#include <thread>

namespace LiveProfiler {
	/**
	 * Collector for collecting cpu samples on linux, based on perf_events
	 */
	class CpuSampleLinuxCollector : public BaseCollector<CpuSampleModel> {
	public:
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
			updateProcesses();
			// list all process and apply filter
			// find process difference
			// - remove process that not exist
			// - add process newly created
			// poll event
			// - read from mmap
			// - convert to model
			// TODO
			std::this_thread::sleep_for(std::chrono::seconds(1));
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
			processesUpdateInterval_() { }

		/** Use the specified function to decide which processes to monitor */
		void filterProcessBy(const std::function<bool(pid_t)>& filter) {
			filter_ = filter;
		}

		/** Use the specified process name to decide which processes to monitor */
		void filterProcessByName(const std::string& name) {
			// reuse some variables to avoid memory allocation
			filterProcessBy([name,
				path=std::string(),
				target=std::string(),
				buf=std::array<char, 100>()](pid_t pid) mutable {
				// build string /proc/pid/exe
				std::snprintf(buf.data(), buf.size(), "%d", pid);
				path.clear();
				path.append("/proc/");
				path.append(buf.data());
				path.append("/exe");
				// read link target
				target.resize(PATH_MAX);
				auto len = ::readlink(path.c_str(), &target.front(), target.size() - 1);
				if (len <= 0) {
					return false;
				}
				target.resize(len);
				// determine is process name matched
				// name should be case sensitive and complete
				auto index = target.find(name);
				if (index == target.npos || index == 0 ||
					target[index-1] != '/' || index+name.size() != target.size()) {
					return false;
				}
				return true;
			});
		}

	protected:
		/** List all processes and determine which processes are of interest */
		void updateProcesses() {
			// open /proc
			::DIR* procDir = ::opendir("/proc");
			if (procDir == nullptr) {
				throw ProfilerException("open /proc failed");
			}
			std::unique_ptr<::DIR, int(*)(DIR*)> procDirPtr(procDir, ::closedir);
			// clear previous processes
			processes_.clear();
			// list directories under /proc
			::dirent* entry = nullptr;
			while ((entry = ::readdir(procDirPtr.get())) != nullptr) {
				if ((entry->d_type & DT_DIR) == 0) {
					continue;
				}
				// check the directory name is valid pid
				char* endptr = nullptr;
				pid_t pid = std::strtol(entry->d_name, &endptr, 10);
				if (endptr == nullptr || endptr == entry->d_name) {
					continue;
				}
				// apply filter
				if (filter_ && filter_(pid)) {
					processes_.emplace_back(pid);
					std::cout << pid << std::endl;
				}
			}
			processesUpdated_ = std::chrono::high_resolution_clock::now();
		}

	protected:
		std::vector<CpuSampleModel> result_;
		std::function<bool(pid_t)> filter_;
		std::vector<pid_t> processes_;
		std::chrono::high_resolution_clock::time_point processesUpdated_;
		std::chrono::high_resolution_clock::duration processesUpdateInterval_;

	};
}

