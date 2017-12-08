#pragma once
#include "BaseAnalyzer.hpp"
#include "../Models/CpuSampleModel.hpp"

namespace LiveProfiler {
	/** Class used to print details of cpu sample to screen for debugging */
	class CpuSampleDebugAnalyzer : public BaseAnalyzer<CpuSampleModel> {
	public:
		/** Reset the state to it's initial state */
		void reset() override { }

		/** Receive performance data */
		void feed(const std::vector<std::unique_ptr<CpuSampleModel>>& models) override {
			std::cout << "[CpuSample] receive " << models.size() << " models" << std::endl;
			for (const auto& model : models) {
				auto& symbolName = model->getSymbolName();
				std::cout << "[CpuSample] - " << std::hex << model->getIp() << std::dec << std::endl;
				if (symbolName != nullptr) {
					std::cout << "[CpuSample]   > " << symbolName->getDemangleName() << std::endl;
					std::cout << "[CpuSample]   > " << *symbolName->getPath() << std::endl;
				}
				auto& callChainIps = model->getCallChainIps();
				auto& callChainSymbolNames = model->getCallChainSymbolNames();
				for (std::size_t i = 0; i < callChainIps.size(); ++i) {
					auto ip = callChainIps.at(i);
					auto& callChainSymbolName = callChainSymbolNames.at(i);
					std::cout << "[CpuSample]   - " << std::hex << ip << std::dec << std::endl;
					if (callChainSymbolName != nullptr) {
						std::cout << "[CpuSample]     > " <<
							callChainSymbolName->getDemangleName() << std::endl;
						std::cout << "[CpuSample]     > " << *callChainSymbolName->getPath() << std::endl;
					}
				}
			}
		}
	};
}

