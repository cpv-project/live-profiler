#pragma once
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <array>
#include <memory>
#include <functional>
#include "../Exceptions/ProfilerException.hpp"

namespace LiveProfiler {
	/** Static utility functions releated to linux processes */
	struct LinuxProcessUtils {
		/** List all processes and determine which processes are of interest */
		static void listProcesses(
			std::vector<pid_t>& pids, const std::function<bool(pid_t)>& filter) {
			// open /proc
			::DIR* procDir = ::opendir("/proc");
			if (procDir == nullptr) {
				throw ProfilerException("open /proc failed");
			}
			std::unique_ptr<::DIR, int(*)(DIR*)> procDirPtr(procDir, ::closedir);
			// clear previous processes
			pids.clear();
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
				if (filter && filter(pid)) {
					pids.emplace_back(pid);
				}
			}
		}

		/** Build a filter function that filters processes by name */
		static std::function<bool(pid_t)> getProcessFilterByName(const std::string& name) {
			// reuse some variables to avoid memory allocation
			return [name,
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
			};
		}
	};
}

