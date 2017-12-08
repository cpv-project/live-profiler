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
			void reset() override {};
			void feed(const std::vector<std::unique_ptr<CpuSampleModel>>& models) override {
				std::cout << "receive " << models.size() << " models" << std::endl;
				for (const auto& model : models) {
					auto& symbolName = model->getSymbolName();
					if (symbolName != nullptr) {
						std::cout << "symbol name: " << symbolName->getDemangleName() <<
							" (" << std::hex << model->getIp() << std::dec << ")" << std::endl;
					} else {
						std::cout << "no symbol name: " << std::hex << model->getIp() << std::dec << std::endl;
					}
					auto& callChainIps = model->getCallChainIps();
					auto& callChainSymbolNames = model->getCallChainSymbolNames();
					for (std::size_t i = 0; i < callChainIps.size(); ++i) {
						auto ip = callChainIps.at(i);
						auto& callChainSymbolName = callChainSymbolNames.at(i);
						if (callChainSymbolName != nullptr) {
							std::cout << "\tsymbol name: " << callChainSymbolName->getDemangleName() <<
								" (" << std::hex << model->getIp() << std::dec << ")" << std::endl;
						} else {
							std::cout << "\tno symbol name: " << std::hex << ip << std::dec << std::endl;
						}
					}
				}
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
		auto analyzer = profiler.addAnalyzer<TestAnalyzer>();
		auto interceptor = profiler.addInterceptor<CpuSampleLinuxSymbolResolveInterceptor>();
		collector->filterProcessByName("LiveProfilerTest");

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
			profiler.collectFor(std::chrono::milliseconds(30000));
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

