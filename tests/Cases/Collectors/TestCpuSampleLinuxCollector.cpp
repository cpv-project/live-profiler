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
		std::atomic_int n(0);
		std::thread t([&flag, &n] {
			while (flag) {
				++n;
				++n;
				++n;
			}
		});

		for (std::size_t i = 0; i < 3; ++i) {
			profiler.collectFor(std::chrono::milliseconds(300));
			std::cout << "---" << std::endl;
		}
		flag = false;
		t.join();
		// TODO
	}

	void testCpuSampleLinuxCollector() {
		std::cout << __func__ << std::endl;
		testCpuSampleLinuxCollectorWithSelfProcess();
	}
}

