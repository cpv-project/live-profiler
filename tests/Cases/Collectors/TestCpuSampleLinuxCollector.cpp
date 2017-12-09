#include <iostream>
#include <atomic>
#include <thread>
#include <LiveProfiler/Analyzers/CpuSampleDebugAnalyzer.hpp>
#include <LiveProfiler/Profiler/Profiler.hpp>
#include <LiveProfiler/Collectors/CpuSampleLinuxCollector.hpp>
#include <LiveProfiler/Interceptors/CpuSampleLinuxSymbolResolveInterceptor.hpp>

namespace LiveProfilerTests {
	using namespace LiveProfiler;

	namespace {
		class TestAnalyzer : public BaseAnalyzer<CpuSampleModel> {
			void reset() override {};
			void feed(const std::vector<std::unique_ptr<CpuSampleModel>>& models) override {
			}
		};

		__attribute__((noinline))
		static void increaseA(std::atomic_int& n) {
			n = n + ::getpid() + ::getuid();
		}

		__attribute__((noinline))
		static void increaseB(std::atomic_int& n) {
			increaseA(n);
		}

		__attribute__((noinline))
		static void increaseC(std::atomic_int& n) {
			increaseB(n);
		}
	}

	void testCpuSampleLinuxCollectorWithSelfProcess() {
		Profiler<CpuSampleModel> profiler;
		auto collector = profiler.useCollector<CpuSampleLinuxCollector>();
		auto analyzer = profiler.addAnalyzer<CpuSampleDebugAnalyzer>();
		auto interceptor = profiler.addInterceptor<CpuSampleLinuxSymbolResolveInterceptor>();
		collector->setExcludeKernel(false);
		collector->setIncludeCallChain(false);
		collector->filterProcessByName("LiveProfilerTest");
		// collector->filterProcessByName("a.out");

		std::atomic_bool flag(true);
		std::atomic_int n(0);
		std::thread t([&flag, &n] {
			while (flag) {
				increaseC(n);
				increaseC(n);
				increaseC(n);
			}
		});

		for (std::size_t i = 0; i < 3; ++i) {
			profiler.collectFor(std::chrono::milliseconds(3000));
			std::cout << "---" << std::endl;
		}
		flag = false;
		t.join();

		// TODO
		// std::this_thread::sleep_for(std::chrono::seconds(1000000));
	}

	void testCpuSampleLinuxCollector() {
		std::cout << __func__ << std::endl;
		testCpuSampleLinuxCollectorWithSelfProcess();
	}
}

