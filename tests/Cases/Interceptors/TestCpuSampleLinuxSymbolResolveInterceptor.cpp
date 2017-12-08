#include <unistd.h>
#include <iostream>
#include <cassert>
#include <LiveProfiler/Interceptors/CpuSampleLinuxSymbolResolveInterceptor.hpp>

namespace LiveProfilerTests {
	using namespace LiveProfiler;

	void testCpuSampleLinuxSymbolResolveInterceptorReset() {
		auto interceptor = std::make_shared<CpuSampleLinuxSymbolResolveInterceptor>();
		interceptor->reset();
	}

	void testCpuSampleLinuxSymbolResolveInterceptorAlter() {
		auto interceptor = std::make_shared<CpuSampleLinuxSymbolResolveInterceptor>();
		for (std::size_t i = 0; i < 3; ++i) {
			interceptor->reset();

			std::vector<std::unique_ptr<CpuSampleModel>> models;
			auto modelA = std::make_unique<CpuSampleModel>();
			modelA->setPid(::getpid());
			modelA->setTid(::getpid());
			modelA->setIp(reinterpret_cast<std::uint64_t>(
				&testCpuSampleLinuxSymbolResolveInterceptorReset));
			modelA->getCallChainIps().emplace_back(modelA->getIp());
			modelA->getCallChainSymbolNames().emplace_back(nullptr);
			models.emplace_back(std::move(modelA));
			auto modelB = std::make_unique<CpuSampleModel>();
			modelB->setPid(::getpid());
			modelB->setTid(::getpid());
			modelB->setIp(reinterpret_cast<std::uint64_t>(
				&testCpuSampleLinuxSymbolResolveInterceptorAlter));
			models.emplace_back(std::move(modelB));

			for (std::size_t j = 0; j < 3; ++j) {
				interceptor->alter(models);
				auto symbolNameA = models.at(0)->getSymbolName();
				auto symbolNameB = models.at(1)->getSymbolName();
				auto callChainSymbolNameA = models.at(0)->getCallChainSymbolNames().at(0);
				assert(symbolNameA != nullptr);
				assert(symbolNameB != nullptr);
				assert(callChainSymbolNameA != nullptr);
				assert(symbolNameA->getName().find(
					"testCpuSampleLinuxSymbolResolveInterceptorReset") != std::string::npos);
				assert(symbolNameB->getName().find(
					"testCpuSampleLinuxSymbolResolveInterceptorAlter") != std::string::npos);
				assert(callChainSymbolNameA->getName().find(
					"testCpuSampleLinuxSymbolResolveInterceptorReset") != std::string::npos);
			}
		}
	}

	void testCpuSampleLinuxSymbolResolveInterceptor() {
		std::cout << __func__ << std::endl;
		testCpuSampleLinuxSymbolResolveInterceptorReset();
		testCpuSampleLinuxSymbolResolveInterceptorAlter();
	}
}

