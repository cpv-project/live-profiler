#pragma once
#include <unordered_map>
#include <algorithm>
#include "BaseAnalyzer.hpp"
#include "../Models/CpuSampleModel.hpp"

namespace LiveProfiler {
	/**
	 * Analyze which symbol names appear most frequently.
	 * There two rankings:
	 * Top Inclusive Symbol Names: The functions that uses the most cpu, include the functions it called
	 * Top Exclusive Symbol Names: The functions that uses the most cpu, not include the functions it called
	 */
	class CpuSampleFrequencyAnalyzer : public BaseAnalyzer<CpuSampleModel> {
	public:
		/** Default parameters */
		static const std::size_t DefaultInclusiveTraceLevel = 3;

		/** Reset the state to it's initial state */
		void reset() override {
			counts_.clear();
			topInclusiveSymbolNames_.clear();
			topExclusiveSymbolNames_.clear();
			totalInclusiveCount_ = 0;
			totalExclusiveCount_ = 0;
		}

		/** Receive performance data */
		void feed(const std::vector<std::unique_ptr<CpuSampleModel>>& models) override {
			for (const auto& model : models) {
				countSymbolName(model->getSymbolName(), false);
				std::size_t level = 0;
				for (const auto& callChainSymbolName : model->getCallChainSymbolNames()) {
					if (level++ >= inclusiveTraceLevel_) {
						break;
					}
					countSymbolName(callChainSymbolName, true);
				}
			}
		}

		/** Set how many levels should be considered for inclusive sampling */
		void setInclusiveTraceLevel(std::size_t inclusiveTraceLevel) {
			inclusiveTraceLevel_ = inclusiveTraceLevel;
		}

		/** Constructor */
		CpuSampleFrequencyAnalyzer() :
			counts_(),
			inclusiveTraceLevel_(DefaultInclusiveTraceLevel),
			topInclusiveSymbolNames_(),
			topExclusiveSymbolNames_(),
			totalInclusiveCount_(0),
			totalExclusiveCount_(0) { }

	public:
		using SymbolNameAndCountType = std::pair<std::shared_ptr<SymbolName>, std::size_t>;

		/** Result type of CpuSampleFrequencyAnalyzer */
		class ResultType {
		public:
			/** Getters */
			const auto& getTopInclusiveSymbolNames() const& { return topInclusiveSymbolNames_; }
			const auto& getTopExclusiveSymbolNames() const& { return topExclusiveSymbolNames_; }
			std::size_t getTotalInclusiveCount() const { return totalInclusiveCount_; }
			std::size_t getTotalExclusiveCount() const { return totalExclusiveCount_; }

			/** Constructor */
			ResultType(
				const std::vector<SymbolNameAndCountType>& topInclusiveSymbolNames,
				const std::vector<SymbolNameAndCountType>& topExclusiveSymbolNames,
				std::size_t totalInclusiveCount,
				std::size_t totalExclusiveCount) :
				topInclusiveSymbolNames_(topInclusiveSymbolNames),
				topExclusiveSymbolNames_(topExclusiveSymbolNames),
				totalInclusiveCount_(totalInclusiveCount),
				totalExclusiveCount_(totalExclusiveCount) { }

		protected:
			const std::vector<SymbolNameAndCountType>& topInclusiveSymbolNames_;
			const std::vector<SymbolNameAndCountType>& topExclusiveSymbolNames_;
			std::size_t totalInclusiveCount_;
			std::size_t totalExclusiveCount_;
		};

		/** Generate the result */
		ResultType getResult(std::size_t topInclusive, std::size_t topExclusive) & {
			topInclusiveSymbolNames_.clear();
			topExclusiveSymbolNames_.clear();
			for (const auto& pair : counts_) {
				if (topInclusive > 0 && pair.second.inclusiveCount > 0) {
					topInclusiveSymbolNames_.emplace_back(pair.first, pair.second.inclusiveCount);
				}
				if (topExclusive > 0 && pair.second.exclusiveCount > 0) {
					topExclusiveSymbolNames_.emplace_back(pair.first, pair.second.exclusiveCount);
				}
			}
			static const auto sortFunc = [](auto &a, auto& b) {
				return a.second > b.second;
			};
			if (topInclusive > 0 && topInclusive < topInclusiveSymbolNames_.size()) {
				std::partial_sort(
					topInclusiveSymbolNames_.begin(),
					topInclusiveSymbolNames_.begin() + topInclusive,
					topInclusiveSymbolNames_.end(),
					sortFunc);
				topInclusiveSymbolNames_.resize(topInclusive);
			}
			if (topExclusive > 0 && topExclusive < topExclusiveSymbolNames_.size()) {
				std::partial_sort(
					topExclusiveSymbolNames_.begin(),
					topExclusiveSymbolNames_.begin() + topExclusive,
					topExclusiveSymbolNames_.end(),
					sortFunc);
				topExclusiveSymbolNames_.resize(topExclusive);
			}
			std::sort(topInclusiveSymbolNames_.begin(), topInclusiveSymbolNames_.end(), sortFunc);
			std::sort(topExclusiveSymbolNames_.begin(), topExclusiveSymbolNames_.end(), sortFunc);
			return ResultType(
				topInclusiveSymbolNames_,
				topExclusiveSymbolNames_,
				totalInclusiveCount_,
				totalExclusiveCount_);
		}

	protected:
		/** Increase count for symbol name */
		void countSymbolName(
			const std::shared_ptr<SymbolName>& symbolName, bool inclusive) {
			++totalInclusiveCount_;
			if (!inclusive) {
				++totalExclusiveCount_;
			}
			if (symbolName != nullptr) {
				auto& count = counts_[symbolName];
				++count.inclusiveCount;
				if (!inclusive) {
					++count.exclusiveCount;
				}
			}
		}

	protected:
		struct CountType {
			std::size_t inclusiveCount = 0;
			std::size_t exclusiveCount = 0;
		};

		std::unordered_map<std::shared_ptr<SymbolName>, CountType> counts_;
		std::size_t inclusiveTraceLevel_;
		std::vector<SymbolNameAndCountType> topInclusiveSymbolNames_;
		std::vector<SymbolNameAndCountType> topExclusiveSymbolNames_;
		std::size_t totalInclusiveCount_;
		std::size_t totalExclusiveCount_;
	};
}

