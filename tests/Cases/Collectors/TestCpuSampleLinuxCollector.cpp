#include <iostream>
#include <atomic>
#include <thread>
#include <LiveProfiler/Profiler/Profiler.hpp>
#include <LiveProfiler/Collectors/CpuSampleLinuxCollector.hpp>
#include <LiveProfiler/Interceptors/CpuSampleLinuxSymbolResolveInterceptor.hpp>

namespace LiveProfilerTests {
	using namespace LiveProfiler;

	namespace {
		class TestAnalyzer : public BaseAnalyzer<CpuSampleModel> {
		public:
			void reset() override { sampleCount_ = 0;};
			void feed(const std::vector<std::unique_ptr<CpuSampleModel>>& models) override {
				for (auto& model : models) {
					assert(model->getPid() == static_cast<std::uint64_t>(::getpid()));
					assert(model->getTid() != 0);
					assert(model->getIp() != 0);
					assert(model->getCallChainIps().size() == model->getCallChainSymbolNames().size());
				}
				sampleCount_ += models.size();
			}
			std::size_t getResult() const { return sampleCount_; }

		protected:
			std::size_t sampleCount_ = 0;
		};
	}

	void testCpuSampleLinuxCollectorWithSelfProcess() {
		Profiler<CpuSampleModel> profiler;
		auto collector = profiler.useCollector<CpuSampleLinuxCollector>();
		auto analyzer = profiler.addAnalyzer<TestAnalyzer>();
		auto interceptor = profiler.addInterceptor<CpuSampleLinuxSymbolResolveInterceptor>();
		collector->filterProcessByName("LiveProfilerTest");

		std::atomic_bool flag(true);
		std::atomic_int n(0);
		std::thread t([&flag, &n] {
			while (flag.load()) {
				++n;
				++n;
				++n;
			}
		});

		for (std::size_t i = 0; i < 3; ++i) {
			profiler.collectFor(std::chrono::milliseconds(100));
		}
		flag.store(false);
		t.join();
		assert(analyzer->getResult() > 0);
	}

	void testCpuSampleLinuxCollector() {
		std::cout << __func__ << std::endl;
		testCpuSampleLinuxCollectorWithSelfProcess();
	}
}

