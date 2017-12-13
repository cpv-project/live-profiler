#pragma once
#include "./Shared/SymbolName.hpp"

namespace LiveProfiler {
	/**
	 * Represent a point of execution.
	 * Result from `getCallChainIps` and `getCallChainSymbolNames` should have same size.
	 * It's valid that `getSymbolName` returns nullptr,
	 * and `getCallChainSymbolNames` returns a vector which contains some nullptr.
	 */
	class CpuSampleModel {
	public:
		/** Getters and setters */
		std::uint64_t getIp() const { return ip_; }
		std::uint64_t getPid() const { return pid_; }
		std::uint64_t getTid() const { return tid_; }
		const auto& getSymbolName() const& { return symbolName_; }
		const auto& getCallChainIps() const& { return callChainIps_; }
		auto& getCallChainIps() & { return callChainIps_; }
		const auto& getCallChainSymbolNames() const& { return callChainSymbolNames_; }
		auto& getCallChainSymbolNames() & { return callChainSymbolNames_; }
		void setIp(std::uint64_t ip) { ip_ = ip; }
		void setPid(std::uint64_t pid) { pid_ = pid; }
		void setTid(std::uint64_t tid) { tid_ = tid; }
		void setSymbolName(const std::shared_ptr<SymbolName>& name) { symbolName_ = name; }

		/** For FreeListAllocator */
		// cppcheck-suppress functionStatic
		void freeResources() { }

		/** For FreeListAllocator */
		void reset() {
			ip_ = 0;
			pid_ = 0;
			tid_ = 0;
			symbolName_ = nullptr;
			callChainIps_.clear();
			callChainSymbolNames_.clear();
		}

		/** Constructor */
		CpuSampleModel() :
			ip_(),
			pid_(),
			tid_(),
			symbolName_(),
			callChainIps_(),
			callChainSymbolNames_() { }

	protected:
		std::uint64_t ip_;
		std::uint64_t pid_;
		std::uint64_t tid_;
		std::shared_ptr<SymbolName> symbolName_;
		std::vector<std::uint64_t> callChainIps_;
		std::vector<std::shared_ptr<SymbolName>> callChainSymbolNames_;
	};
}

