#include <iostream>
#include <cassert>
#include <thread>
#include <LiveProfiler/Profiler/Profiler.hpp>

namespace LiveProfilerTests {
	using namespace LiveProfiler;

	namespace {
		struct MinimalModel { std::size_t count; };

		struct MinimalCollector : BaseCollector<MinimalModel> {
			std::size_t count = 0;
			bool enabled = false;
			std::vector<std::unique_ptr<MinimalModel>> result;

			void reset() override { count = 0; }
			void enable() override { enabled = true; }
			void disable() override { enabled = false; }
			std::vector<std::unique_ptr<MinimalModel>>& collect(
				std::chrono::high_resolution_clock::duration timeout) & override {
				result.clear();
				auto start = std::chrono::high_resolution_clock::now();
				while (result.size() < 5 &&
					std::chrono::high_resolution_clock::now() - start < timeout) {
					assert(enabled);
					result.emplace_back(
						std::make_unique<MinimalModel>(MinimalModel({ ++count })));
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
				return result;
			}
		};

		struct MinimalAnalyzer : BaseAnalyzer<MinimalModel> {
			std::size_t lastReceived = 0;
			std::size_t adjust = 0;
			
			void reset() override { lastReceived = 0; adjust = 0; }
			void feed(const std::vector<std::unique_ptr<MinimalModel>>& models) override {
				for (const auto& model : models) {
					assert(lastReceived + 1 + adjust == model->count);
					lastReceived = model->count - adjust;
				}
			}
			std::size_t getResult() { return lastReceived; }
		};

		struct MinimalInterceptor : BaseInterceptor<MinimalModel> {
			std::size_t adjust = 0;
			
			void reset() override { adjust = 0; }
			void alter(std::vector<std::unique_ptr<MinimalModel>>& models) override {
				for (auto& model : models) {
					model->count += adjust;
				}
			}
		};
	}

	void testProfilerThrowsWhenCollectorNotSet() {
		Profiler<MinimalModel> profiler;
		bool catched = false;
		try {
			profiler.collectFor(std::chrono::seconds(1));
		} catch (const ProfilerException&) {
			catched = true;
		}
		assert(catched);
	}

	void testProfilerWithMinimalModel() {
		Profiler<MinimalModel> profiler;
		auto collector = profiler.useCollector<MinimalCollector>();
		auto analyzerA = profiler.addAnalyzer<MinimalAnalyzer>();
		auto analyzerB = profiler.addAnalyzer<MinimalAnalyzer>();

		assert(collector != nullptr);
		assert(analyzerA != nullptr);
		assert(analyzerB != nullptr);
		assert(analyzerA != analyzerB);
		assert(!collector->enabled);
		assert(analyzerA->getResult() == 0);
		assert(analyzerB->getResult() == 0);

		profiler.collectFor(std::chrono::milliseconds(20));
		assert(!collector->enabled);
		assert(analyzerA->getResult() > 0); // the value depends on the kernel scheduler
		assert(analyzerA->getResult() == analyzerB->getResult());

		profiler.reset();
		assert(!collector->enabled);
		assert(collector->count == 0);
		assert(analyzerA->getResult() == 0);
		assert(analyzerB->getResult() == 0);

		auto removedFirst = profiler.removeAnalyzer(analyzerB);
		auto removedSecond = profiler.removeAnalyzer(analyzerB);
		assert(removedFirst);
		assert(!removedSecond);

		profiler.collectFor(std::chrono::milliseconds(20));
		assert(!collector->enabled);
		assert(analyzerA->getResult() > 0);
		assert(analyzerB->getResult() == 0);
	}

	void testProfilerWithInterceptor() {
		Profiler<MinimalModel> profiler;
		auto collector = profiler.useCollector<MinimalCollector>();
		auto analyzer = profiler.addAnalyzer<MinimalAnalyzer>();
		auto interceptor = profiler.addInterceptor<MinimalInterceptor>();
		analyzer->adjust = 3;
		interceptor->adjust = analyzer->adjust;

		profiler.collectFor(std::chrono::milliseconds(20));
		assert(!collector->enabled);
		assert(analyzer->getResult() > analyzer->adjust);
		
		profiler.reset();
		assert(analyzer->adjust == 0);
		assert(interceptor->adjust == 0);
	}

	void testProfiler() {
		std::cout << __func__ << std::endl;
		testProfilerThrowsWhenCollectorNotSet();
		testProfilerWithMinimalModel();
		testProfilerWithInterceptor();
	}
}

