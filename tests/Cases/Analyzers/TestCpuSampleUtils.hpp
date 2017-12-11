#include <iostream>
#include <cassert>
#include <LiveProfiler/Models/CpuSampleModel.hpp>

namespace LiveProfilerTests {
	using namespace LiveProfiler;

	namespace {
		std::shared_ptr<SymbolName> makeSymbol(
			const std::shared_ptr<std::string>& path,
			const std::string& name) {
			auto symbolName = std::make_shared<SymbolName>();
			symbolName->setOriginalName(name);
			symbolName->setPath(path);
			return symbolName;
		}

		std::unique_ptr<CpuSampleModel> makeModel(
			std::shared_ptr<SymbolName> symbolName,
			std::initializer_list<std::shared_ptr<SymbolName>> callChainSymbolNames) {
			auto model = std::make_unique<CpuSampleModel>();
			model->setSymbolName(symbolName);
			for (auto& callChainSymbolName : callChainSymbolNames) {
				model->getCallChainIps().emplace_back(0);
				model->getCallChainSymbolNames().emplace_back(callChainSymbolName);
			}
			return model;
		}
	}
}

