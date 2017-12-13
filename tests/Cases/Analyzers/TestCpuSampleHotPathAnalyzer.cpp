#include <iostream>
#include <cassert>
#include <LiveProfiler/Analyzers/CpuSampleHotPathAnalyzer.hpp>
#include "TestCpuSampleUtils.hpp"

namespace LiveProfilerTests {
	using namespace LiveProfiler;

	void testCpuSampleHotPathAnalyzer() {
		std::cout << __func__ << std::endl;
		auto path = std::make_shared<std::string>("test");
		auto symbolNameA = makeSymbol(path, "symbolNameA");
		auto symbolNameB = makeSymbol(path, "symbolNameB");
		auto symbolNameC = makeSymbol(path, "symbolNameC");
		auto analyzer = std::make_shared<CpuSampleHotPathAnalyzer>();
	}
}

