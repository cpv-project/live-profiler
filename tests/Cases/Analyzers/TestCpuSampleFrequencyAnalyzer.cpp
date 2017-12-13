#include <iostream>
#include <cassert>
#include <LiveProfiler/Analyzers/CpuSampleFrequencyAnalyzer.hpp>
#include "TestCpuSampleUtils.hpp"

namespace LiveProfilerTests {
	using namespace LiveProfiler;

	void testCpuSampleFrequencyAnalyzer() {
		std::cout << __func__ << std::endl;
		auto path = std::make_shared<std::string>("test");
		auto symbolNameA = makeSymbol(path, "symbolNameA");
		auto symbolNameB = makeSymbol(path, "symbolNameB");
		auto symbolNameC = makeSymbol(path, "symbolNameC");
		auto analyzer = std::make_shared<CpuSampleFrequencyAnalyzer>();
		{
			std::vector<std::unique_ptr<CpuSampleModel>> models;
			models.emplace_back(makeModel(symbolNameA, { symbolNameB, symbolNameC }));
			models.emplace_back(makeModel(symbolNameA, { symbolNameB, symbolNameC }));
			models.emplace_back(makeModel(symbolNameA, { symbolNameB, symbolNameC }));
			analyzer->feed(models);
		}
		{
			std::vector<std::unique_ptr<CpuSampleModel>> models;
			models.emplace_back(makeModel(symbolNameB, { symbolNameC }));
			models.emplace_back(makeModel(symbolNameB, { symbolNameC }));
			models.emplace_back(makeModel(symbolNameC, { }));
			analyzer->feed(models);
		}
		{
			auto result = analyzer->getResult(2, 2);
			auto& topInclusiveSymbolNames = result.getTopInclusiveSymbolNames();
			auto& topExclusiveSymbolNames = result.getTopExclusiveSymbolNames();
			assert(topInclusiveSymbolNames.size() == 2);
			assert(topInclusiveSymbolNames.at(0).first == symbolNameC);
			assert(topInclusiveSymbolNames.at(0).second == 6);
			assert(topInclusiveSymbolNames.at(1).first == symbolNameB);
			assert(topInclusiveSymbolNames.at(1).second == 5);
			assert(topExclusiveSymbolNames.size() == 2);
			assert(topExclusiveSymbolNames.at(0).first == symbolNameA);
			assert(topExclusiveSymbolNames.at(0).second == 3);
			assert(topExclusiveSymbolNames.at(1).first == symbolNameB);
			assert(topExclusiveSymbolNames.at(1).second == 2);
			assert(result.getTotalSampleCount() == 6);
		}
		{
			auto result = analyzer->getResult(0, 1000);
			assert(result.getTopInclusiveSymbolNames().empty());
			auto& topExclusiveSymbolNames = result.getTopExclusiveSymbolNames();
			assert(topExclusiveSymbolNames.size() == 3);
			assert(topExclusiveSymbolNames.at(0).first == symbolNameA);
			assert(topExclusiveSymbolNames.at(0).second == 3);
			assert(topExclusiveSymbolNames.at(1).first == symbolNameB);
			assert(topExclusiveSymbolNames.at(1).second == 2);
			assert(topExclusiveSymbolNames.at(2).first == symbolNameC);
			assert(topExclusiveSymbolNames.at(2).second == 1);
			assert(result.getTotalSampleCount() == 6);
		}
		{
			analyzer->reset();
			auto result = analyzer->getResult(1000, 1000);
			assert(result.getTopInclusiveSymbolNames().empty());
			assert(result.getTopExclusiveSymbolNames().empty());
			assert(result.getTotalSampleCount() == 0);
		}
	}
}

