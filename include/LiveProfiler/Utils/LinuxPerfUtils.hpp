#pragma once
#include <fcntl.h>
#include <sys/syscall.h>
#include <memory>
#include "LinuxPerfEntry.hpp"

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

		/** Setup cpu sample monitor for specified process */
		static bool monitorCpuSample(std::unique_ptr<LinuxPerfEntry>& entry, std::size_t samplePeriod) {
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
			attr.sample_type = PERF_SAMPLE_IP | PERF_SAMPLE_CALLCHAIN;
			attr.disabled = 1;
			// open file descriptor
			auto fd = perfEventOpen(&attr, pid, -1, -1, 0);
			if (fd < 0) {
				return false;
			}
			entry->setFd(fd);
			// setup memory mapping
			// TODO
			return true;
		}
	};
}

