#pragma once
#include <unordered_map>
#include "BaseAnalyzer.hpp"
#include "../Models/CpuSampleModel.hpp"

namespace LiveProfiler {
	/**
	 * Analyze which call path takes the most cpu usage.
	 * The structure of the result is like:
	 * - root 100 (1.00)
	 *   - A 50 (0.50)
	 *   - B 35 (0.35)
	 *     - C 25 (0.25)
	 *     - D 5 (0.05)
	 * The missing number means there are some samples have none symbol name.
	 */
	class CpuSampleHotPathAnalyzer : public BaseAnalyzer<CpuSampleModel> {
	public:
		/** Reset the state to it's initial state */
		void reset() override {
			root_ = std::make_unique<NodeType>();
			totalSampleCount_ = 0;
		}

		/** Receive performance data */
		void feed(const std::vector<std::unique_ptr<CpuSampleModel>>& models) override {
			for (const auto& model : models) {
				++totalSampleCount_;
				countModel(root_, model, model->getCallChainSymbolNames().size());
			}
		}

		/** Constructor */
		CpuSampleHotPathAnalyzer() :
			root_(std::make_unique<NodeType>()),
			totalSampleCount_(0) { }
	
	public:
		/** Tree type represent the call path */
		class NodeType {
		public:
			/** Getters */
			std::size_t getCount() const { return count_; }
			const auto& getChilds() const& { return childs_; }

			/** Increase count in this node */
			void increaseCount() { ++count_; }

			/** Get child node by symbol name, create if not exists */
			std::unique_ptr<NodeType>& getChild(const std::shared_ptr<SymbolName>& symbolName) & {
				auto it = childs_.find(symbolName);
				if (it == childs_.end()) {
					auto pair = childs_.emplace(symbolName, std::make_unique<NodeType>());
					it = pair.first;
				}
				return it->second;
			}

			/** Constructor */
			NodeType() :
				count_(0),
				childs_() { }

		protected:
			std::size_t count_;
			std::unordered_map<std::shared_ptr<SymbolName>, std::unique_ptr<NodeType>> childs_;
		};

		/** Result type of CpuSampleHotPathAnalyzer */
		class ResultType {
		public:
			/** Getters */
			const auto& getRoot() const& { return root_; }
			std::size_t getTotalSampleCount() const { return totalSampleCount_; }

			/** Constructor */
			ResultType(
				const std::unique_ptr<NodeType>& root,
				std::size_t totalSampleCount) :
				root_(root),
				totalSampleCount_(totalSampleCount) { }

		protected:
			const std::unique_ptr<NodeType>& root_;
			std::size_t totalSampleCount_;
		};

		/** Generate the result */
		ResultType getResult() {
			return ResultType(root_, totalSampleCount_);
		}

	protected:
		/** Recursively increase the count in the tree for single model data */
		void countModel(
			std::unique_ptr<NodeType>& node,
			const std::unique_ptr<CpuSampleModel>& model,
			std::size_t index) {
			if (index > 0) {
				// symbol name in callchain
				auto& symbolName = model->getCallChainSymbolNames()[index-1];
				if (symbolName == nullptr) {
					// continue to use this node, that mean a -> ? -> b will reduce to a -> b
					countModel(node, model, index-1);
				} else {
					node->increaseCount();
					countModel(node->getChild(symbolName), model, index-1);
				}
			} else {
				// last symbol name
				node->increaseCount();
				auto& symbolName = model->getSymbolName();
				if (symbolName != nullptr) {
					node->getChild(symbolName)->increaseCount();
				}
			}
		}

	protected:
		std::unique_ptr<NodeType> root_;
		std::size_t totalSampleCount_;
	};
}

