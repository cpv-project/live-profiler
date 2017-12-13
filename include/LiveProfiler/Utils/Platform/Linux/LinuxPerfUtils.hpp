#pragma once
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <memory>
#include "LinuxPerfEntry.hpp"
#include "../../../Exceptions/ProfilerException.hpp"

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
			std::uint32_t type, // eg: PERF_TYPE_SOFTWARE
			std::uint64_t config, // eg: PERF_COUNT_SW_CPU_CLOCK
			std::uint64_t samplePeriod, // eg: 100000
			std::uint64_t sampleType, // eg: PERF_SAMPLE_IP | PERF_SAMPLE_TID
			std::size_t mmapPageCount, // eg: 16, should be power of 2
			std::uint32_t wakeupEvents, // eg: 8, atleast 1
			bool excludeUser, // exclude samples in user space
			bool excludeKernel, // exclude samples in kernel space
			bool excludeHv) { // exclude samples in hypervisor
			// caller should set a valid pid
			auto pid = entry->getPid();
			if (pid <= 0) {
				return false;
			}
			// setup attributes
			auto& attr = entry->getAttrRef();
			attr.type = type;
			attr.size = sizeof(attr);
			attr.config = config;
			attr.sample_period = samplePeriod;
			attr.sample_type = sampleType;
			attr.disabled = 1;
			attr.inherit = 0;
			attr.wakeup_events = wakeupEvents;
			attr.exclude_user = excludeUser;
			attr.exclude_kernel = excludeKernel;
			attr.exclude_hv = excludeHv;
			// open file descriptor
			auto fd = perfEventOpen(&attr, pid, -1, -1, 0);
			if (fd < 0) {
				auto err = errno;
				if (err == ESRCH) {
					// process may have exited
					return false;
				}
				throw ProfilerException(err, "[monitorSample] perf_event_open");
			}
			entry->setFd(fd);
			// setup memory mapping
			std::size_t pageSize = ::getpagesize();
			std::size_t pageCount = mmapPageCount + 1;
			std::size_t totalSize = pageSize * pageCount;
			auto* address = ::mmap(0, totalSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
			if (address == nullptr || reinterpret_cast<intptr_t>(address) == -1) {
				// cppcheck-suppress memleak
				throw ProfilerException(errno, "[monitorSample] mmap");
			}
			entry->setMmapAddress(
				reinterpret_cast<char*>(address), totalSize, pageSize);
			return true;
		}
	};
}

