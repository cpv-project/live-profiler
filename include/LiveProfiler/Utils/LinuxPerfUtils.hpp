#pragma once
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <memory>
#include "LinuxPerfEntry.hpp"
#include "../Exceptions/ProfilerException.hpp"

namespace LiveProfiler {
	/** Static utility functions releated to linux perf_events */
	struct LinuxPerfUtils {
		/**
		 * perf_event_open syscall wrapper
		 * see: http://www.man7.org/linux/man-pages/man2/perf_event_open.2.html
		 */
		static int perfEventOpen(
			::perf_event_attr* hw_event,
			pid_t pid,
			int cpu,
			int group_fd,
			unsigned long flags) {
			return ::syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
		}

		/** Enable perf events, also reset if `reset` parameter is true */
		static bool perfEventEnable(int fd, bool reset) {
			auto retReset = reset ? ::ioctl(fd, PERF_EVENT_IOC_RESET, 0) : 0;
			auto retEnable = ::ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);
			return retReset >= 0 && retEnable >= 0;
		}

		/** Disable perf events */
		static bool perfEventDisable(int fd) {
			auto ret = ::ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
			return ret >= 0;
		}

		/** Setup perf sample monitor for specified process */
		static bool monitorSample(
			std::unique_ptr<LinuxPerfEntry>& entry,
			std::size_t samplePeriod,
			std::size_t mmapPageCount,
			std::uint64_t sample_type) {
			// caller should set a valid pid
			auto pid = entry->getPid();
			if (pid <= 0) {
				return false;
			}
			// setup attributes
			auto& attr = entry->getAttrRef();
			attr.size = sizeof(attr);
			attr.type = PERF_TYPE_SOFTWARE;
			attr.config = PERF_COUNT_SW_CPU_CLOCK;
			attr.sample_period = samplePeriod;
			attr.sample_type = sample_type;
			attr.disabled = 1;
			attr.inherit = 0;
			attr.wakeup_events = 1;
			attr.exclude_kernel = 1;
			attr.exclude_hv = 1;
			// open file descriptor
			auto fd = perfEventOpen(&attr, pid, -1, -1, 0);
			if (fd < 0) {
				auto err = errno;
				if (err == ESRCH) {
					return false; // process may have exited
				}
				throw ProfilerException(err, "[monitorSample] perf_event_open");
			}
			entry->setFd(fd);
			// setup memory mapping
			std::size_t pageSize = ::getpagesize();
			std::size_t pageCount = mmapPageCount + 1;
			std::size_t totalSize = pageSize * pageCount;
			auto* address = ::mmap(0, totalSize, PROT_READ, MAP_SHARED, fd, 0);
			if (address == nullptr || reinterpret_cast<intptr_t>(address) == -1) {
				throw ProfilerException(errno, "[monitorSample] mmap");
			}
			entry->setMmapAddress(reinterpret_cast<char*>(address), totalSize, pageSize);
			return true;
		}
	};
}

