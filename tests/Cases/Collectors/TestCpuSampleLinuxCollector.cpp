#include <iostream>
#include <LiveProfiler/Profiler/Profiler.hpp>
#include <LiveProfiler/Collectors/CpuSampleLinuxCollector.hpp>

namespace LiveProfilerTests {
	using namespace LiveProfiler;

	void testCpuSampleLinuxCollectorWithSelfProcess() {
		Profiler<CpuSampleModel> profiler;
		auto collector = profiler.useCollector<CpuSampleLinuxCollector>();
		collector->filterProcessByName("LiveProfilerTest");
		profiler.collectFor(std::chrono::seconds(1));
	}

	void testCpuSampleLinuxCollector() {
		std::cout << __func__ << std::endl;
		testCpuSampleLinuxCollectorWithSelfProcess();
	}
}

