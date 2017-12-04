#include <iostream>
#include <atomic>
#include <thread>
#include <LiveProfiler/Profiler/Profiler.hpp>
#include <LiveProfiler/Collectors/CpuSampleLinuxCollector.hpp>

namespace LiveProfilerTests {
	using namespace LiveProfiler;

	void testCpuSampleLinuxCollectorWithSelfProcess() {
		Profiler<CpuSampleModel> profiler;
		auto collector = profiler.useCollector<CpuSampleLinuxCollector>();
		collector->filterProcessByName("LiveProfilerTest");

		std::atomic_bool flag(true);
		std::thread t([&flag] {
			while (flag) { }
		});

		for (std::size_t i = 0; i < 3; ++i) {
			profiler.collectFor(std::chrono::milliseconds(30000));
			std::cout << "---" << std::endl;
		}
		flag = false;
		t.join();
	}

	void testCpuSampleLinuxCollector() {
		std::cout << __func__ << std::endl;
		testCpuSampleLinuxCollectorWithSelfProcess();
	}
}

