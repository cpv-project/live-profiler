#pragma once
#include "./Shared/SymbolName.hpp"

namespace LiveProfiler {
	/** Represent a point of execution */
	class CpuSampleModel {
	public:
		/** Getters and setters */
		std::uint64_t getIp() const { return ip_; }
		std::uint64_t getPid() const { return pid_; }
		std::uint64_t getTid() const { return tid_; }
		const std::shared_ptr<SymbolName>& getSymbolName() const& { return symbolName_; }
		const std::vector<std::uint64_t>& getCallChainIps() const& { return callChainIps_; }
		std::vector<std::uint64_t>& getCallChainIps() & { return callChainIps_; }
		const std::vector<std::shared_ptr<SymbolName>> getCallChainSymbolNames() const& {
			return callChainSymbolNames_;
		}
		std::vector<std::shared_ptr<SymbolName>> getCallChainSymbolNames() & {
			return callChainSymbolNames_;
		}
		void setIp(std::uint64_t ip) { ip_ = ip; }
		void setPid(std::uint64_t pid) { pid_ = pid; }
		void setTid(std::uint64_t tid) { tid_ = tid; }
		void setSymbolName(const std::shared_ptr<SymbolName>& name) { symbolName_ = name; }

		/** For FreeListAllocator */
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

