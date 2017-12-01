#pragma once
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <vector>
#include <array>
#include <memory>
#include <functional>
#include "../Exceptions/ProfilerException.hpp"
#include "../Utils/Containers/StackBuffer.hpp"
#include "../Utils/TypeConvertUtils.hpp"

namespace LiveProfiler {
	/** Static utility functions releated to linux processes */
	struct LinuxProcessUtils {
		/**
		 * List all processes and determine which processes are of interest.
		 * The result will be appended to `pids`.
		 * If `includeThreads` is true then include all thread ids, otherwise only pids.
		 */
		static void listProcesses(
			std::vector<pid_t>& pids,
			const std::function<bool(pid_t)>& filter,
			bool includeThreads) {
			// open /proc
			::DIR* procDir = ::opendir("/proc");
			if (procDir == nullptr) {
				throw ProfilerException("open /proc failed");
			}
			std::unique_ptr<::DIR, int(*)(DIR*)> procDirPtr(procDir, ::closedir);
			// list directories under /proc
			::dirent* entry = nullptr;
			while ((entry = ::readdir(procDirPtr.get())) != nullptr) {
				if ((entry->d_type & DT_DIR) == 0) {
					continue;
				}
				// check the directory name is valid pid
				long long pidL = 0;
				if (!TypeConvertUtils::strToLongLong(entry->d_name, pidL)) {
					continue;
				}
				pid_t pid = static_cast<pid_t>(pidL);
				// apply filter
				if (!(filter && filter(pid))) {
					continue;
				}
				// append result
				if (includeThreads) {
					listThreads(pids, pid);
				} else {
					pids.emplace_back(pid);
				}
			}
		}

		/**
		 * List all threads in specified process.
		 * The result will be appended to `tids`
		 */
		static void listThreads(std::vector<pid_t>& tids, pid_t pid) {
			// use uninitialized array to avoid memory allocation
			static std::string prefix("/proc/");
			static std::string suffix("/task");
			StackBuffer<128> buf;
			// build task path "/proc/$pid/task"
			buf.appendStr(prefix.data(), prefix.size());
			buf.appendLongLong(pid);
			buf.appendStr(suffix.data(), suffix.size());
			buf.appendNullTerminator();
			// open task path
			::DIR* taskDir = ::opendir(buf.data());
			if (taskDir == nullptr) {
				// the process may have exited
				return;
			}
			std::unique_ptr<::DIR, int(*)(DIR*)> taskDirPtr(taskDir, ::closedir);
			// list directories under task path
			::dirent* entry = nullptr;
			while ((entry = ::readdir(taskDirPtr.get())) != nullptr) {
				if ((entry->d_type & DT_DIR) == 0) {
					continue;
				}
				// check the directory name is valid pid
				long long tidL = 0;
				if (!TypeConvertUtils::strToLongLong(entry->d_name, tidL)) {
					continue;
				}
				pid_t tid = static_cast<pid_t>(tidL);
				// append result
				tids.emplace_back(tid);
			}
		}

		/** Build a filter function that filters processes by name */
		static std::function<bool(pid_t)> getProcessFilterByName(const std::string& name) {
			// reuse some variables to avoid memory allocation
			static std::string prefix("/proc/");
			static std::string suffix("/exe");
			return [name, target=std::string(), buf=StackBuffer<128>()](pid_t pid) mutable {
				// build exe path "/proc/$pid/exe"
				buf.clear();
				buf.appendStr(prefix.data(), prefix.size());
				buf.appendLongLong(pid);
				buf.appendStr(suffix.data(), suffix.size());
				buf.appendNullTerminator();
				// read link target
				target.resize(PATH_MAX);
				auto len = ::readlink(buf.data(), &target.front(), target.size() - 1);
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

