#pragma once
#include <memory>
#include "LinuxPerfEntry.hpp"

namespace LiveProfiler {
	/** Static utility functions releated to linux perf_events */
	struct LinuxPerfUtils {
		bool monitorCpuSample(std::unique_ptr<LinuxPerfEntry>& entry) {
			return false;
		}
	};
}

