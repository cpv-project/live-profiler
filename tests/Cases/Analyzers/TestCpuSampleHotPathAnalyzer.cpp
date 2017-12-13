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
			models.emplace_back(makeModel(symbolNameB, { nullptr, symbolNameC }));
			models.emplace_back(makeModel(symbolNameC, { }));
			analyzer->feed(models);
		}
		{
			auto result = analyzer->getResult();
			assert(result.getTotalSampleCount() == 6);
			auto& root = result.getRoot();
			auto& rootChilds = root->getChilds();
			assert(root->getCount() == 6);
			assert(rootChilds.size() == 1);
			assert(rootChilds.count(symbolNameC) == 1);
			auto& c = rootChilds.at(symbolNameC);
			auto& cChilds = c->getChilds();
			assert(c->getCount() == 6);
			assert(cChilds.size() == 1);
			assert(cChilds.count(symbolNameB) == 1);
			auto& b = cChilds.at(symbolNameB);
			auto& bChilds = b->getChilds();
			assert(b->getCount() == 5);
			assert(bChilds.size() == 1);
			assert(bChilds.count(symbolNameA) == 1);
			auto& a = bChilds.at(symbolNameA);
			auto& aChilds = a->getChilds();
			assert(a->getCount() == 3);
			assert(aChilds.empty());
		}
		{
			analyzer->reset();
			auto result = analyzer->getResult();
			assert(result.getTotalSampleCount() == 0);
			auto& root = result.getRoot();
			assert(root->getCount() == 0);
			assert(root->getChilds().empty());
		}
	}
}

