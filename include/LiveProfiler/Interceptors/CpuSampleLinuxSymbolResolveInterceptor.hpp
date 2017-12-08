#pragma once
#include <unistd.h>
#include <vector>
#include <memory>
#include <chrono>
#include <unordered_map>
#include "BaseInterceptor.hpp"
#include "../Models/CpuSampleModel.hpp"
#include "../Utils/Allocators/FreeListAllocator.hpp"
#include "../Utils/Allocators/SingletonAllocator.hpp"
#include "../Utils/Platform/Linux/LinuxExecutableSymbolResolver.hpp"
#include "../Utils/Platform/Linux/LinuxKernelSymbolResolver.hpp"
#include "../Utils/Platform/Linux/LinuxProcessAddressLocator.hpp"
#include "../Utils/Platform/Linux/LinuxProcessUtils.hpp"

namespace LiveProfiler {
	/**
	 * Interceptor used to setup symbol names in model data.
	 * How this interceptor resolve symbol name:
	 * - First, use LinuxProcessAddressLocator and LinuxExecutableSymbolResolver
	 * - Then, use LinuxKernelSymbolResolver
	 */
	class CpuSampleLinuxSymbolResolveInterceptor : public BaseInterceptor<CpuSampleModel> {
	public:
		/** Default parameters */
		static const std::size_t DefaultMaxFreeAddressLocator = 1024;
		static const std::size_t DefaultPidToAddressLocatorSweepInterval = 1000;

		/** Reset the state to it's initial state */
		void reset() override {
			for (auto& pair : pidToAddressLocator_) {
				addressLocatorAllocator_.deallocate(std::move(pair.second));
			}
			pidToAddressLocator_.clear();
			pidToAddressLocatorSwept_ = {};
			lastPid_ = 0;
			lastAddressLocatorIterator_ = {};
		}

		/** Setup symbol names in model data */
		void alter(std::vector<std::unique_ptr<CpuSampleModel>>& models) override {
			 // cleanup pidToAddressLocator_
			auto now = std::chrono::high_resolution_clock::now();
			if (now - pidToAddressLocatorSwept_ > pidToAddressLocatorSweepInterval_) {
				sweepPidToAddressLocator();
				pidToAddressLocatorSwept_ = now;
			}
			// setup symbol names in model data
			for (auto& model : models) {
				// find or create address locator by pid
				pid_t pid = model->getPid();
				decltype(lastAddressLocatorIterator_) addressLocatorIt = {};
				if (pid == lastPid_) {
					addressLocatorIt = lastAddressLocatorIterator_;
				} else {
					addressLocatorIt = pidToAddressLocator_.find(pid);
					if (addressLocatorIt == pidToAddressLocator_.end()) {
						auto pair = pidToAddressLocator_.emplace(pid,
							addressLocatorAllocator_.allocate(pid, pathAllocator_));
						addressLocatorIt = pair.first;
					}
					lastPid_ = pid;
					lastAddressLocatorIterator_ = addressLocatorIt;
				}
				// resolve symbol names
				// although ip is the next instruction of the executing instruction,
				// the executing instruction is rare to be ret,
				// moretimes, the next instruction would be the entry point of a dynamic function,
				// so here use ip, not ip-1.
				auto ip = model->getIp();
				auto pathAndOffset = addressLocatorIt->second->locate(ip, false);
				if (pathAndOffset.first != nullptr) {
					auto resolver = resolverAllocator_->allocate(std::move(pathAndOffset.first));
					model->setSymbolName(resolver->resolve(pathAndOffset.second));
				} else {
					model->setSymbolName(kernelResolver_.resolve(ip));
				}
				auto& callChainIps = model->getCallChainIps();
				auto& callChainSymbolNames = model->getCallChainSymbolNames();
				for (std::size_t i = 0; i < callChainIps.size(); ++i) {
					auto callChainIp = callChainIps[i];
					pathAndOffset = addressLocatorIt->second->locate(callChainIp, false);
					if (pathAndOffset.first != nullptr) {
						auto resolver = resolverAllocator_->allocate(std::move(pathAndOffset.first));
						callChainSymbolNames.at(i) = resolver->resolve(pathAndOffset.second);
					} else {
						callChainSymbolNames.at(i) = kernelResolver_.resolve(callChainIp);
					}
				}
			}
		}

		/** Constructor */
		CpuSampleLinuxSymbolResolveInterceptor() :
			pidToAddressLocator_(),
			addressLocatorAllocator_(DefaultMaxFreeAddressLocator),
			pathAllocator_(std::make_shared<decltype(pathAllocator_)::element_type>()),
			resolverAllocator_(std::make_shared<decltype(resolverAllocator_)::element_type>()),
			kernelResolver_(),
			pidToAddressLocatorSwept_(),
			pidToAddressLocatorSweepInterval_(
				std::chrono::milliseconds(DefaultPidToAddressLocatorSweepInterval)),
			lastPid_(0),
			lastAddressLocatorIterator_() { }

	protected:
		/** Find out which process no longer exist and cleanup pidToAddressLocator_ */
		void sweepPidToAddressLocator() {
			for (auto it = pidToAddressLocator_.begin(); it != pidToAddressLocator_.end();) {
				pid_t pid = it->first;
				if (LinuxProcessUtils::isProcessExists(pid)) {
					++it;
				} else {
					addressLocatorAllocator_.deallocate(std::move(it->second));
					it = pidToAddressLocator_.erase(it);
				}
			}
		}

	protected:
		std::unordered_map<pid_t, std::unique_ptr<LinuxProcessAddressLocator>> pidToAddressLocator_;
		FreeListAllocator<LinuxProcessAddressLocator> addressLocatorAllocator_;
		std::shared_ptr<SingletonAllocator<std::string, std::string>> pathAllocator_;
		std::shared_ptr<SingletonAllocator<
			std::shared_ptr<std::string>, LinuxExecutableSymbolResolver>> resolverAllocator_;
		LinuxKernelSymbolResolver kernelResolver_;

		std::chrono::high_resolution_clock::time_point pidToAddressLocatorSwept_;
		std::chrono::high_resolution_clock::duration pidToAddressLocatorSweepInterval_;

		pid_t lastPid_;
		decltype(pidToAddressLocator_)::iterator lastAddressLocatorIterator_;
	};
}

