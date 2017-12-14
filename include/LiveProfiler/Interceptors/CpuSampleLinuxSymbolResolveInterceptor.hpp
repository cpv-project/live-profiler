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
#include "../Utils/Platform/Linux/LinuxProcessCustomSymbolResolver.hpp"
#include "../Utils/Platform/Linux/LinuxProcessUtils.hpp"

namespace LiveProfiler {
	/**
	 * Interceptor used to setup symbol names in model data.
	 * How this interceptor resolve symbol name:
	 * - First, use LinuxProcessAddressLocator and LinuxExecutableSymbolResolver
	 * - Then, use LinuxKernelSymbolResolver
	 * - Finally, use LinuxProcessCustomSymbolResolver
	 */
	class CpuSampleLinuxSymbolResolveInterceptor : public BaseInterceptor<CpuSampleModel> {
	public:
		/** Default parameters */
		static const std::size_t DefaultMaxFreeAddressLocator = 1024;
		static const std::size_t DefaultMaxFreeCustomResolver = 1024;
		static const std::size_t DefaultSurvivalProcessMinCheckInterval = 1000;

		/** Reset the state to it's initial state */
		void reset() override {
			for (auto& pair : pidToAddressLocator_) {
				addressLocatorAllocator_.deallocate(std::move(pair.second));
			}
			lastAddressLocatorPid_ = 0;
			lastAddressLocatorIterator_ = {};
			pidToAddressLocator_.clear();
			for (auto& pair : pidToCustomResolver_) {
				customResolverAllocator_.deallocate(std::move(pair.second));
			}
			lastCustomResolverPid_ = 0;
			lastCustomResolverIterator_ = {};
			pidToCustomResolver_.clear();
			survivalProcessChecked_ = {};
		}

		/** Setup symbol names in model data */
		void alter(std::vector<std::unique_ptr<CpuSampleModel>>& models) override {
			 // cleanup pidToAddressLocator_
			auto now = std::chrono::high_resolution_clock::now();
			if (now - survivalProcessChecked_ > survivalProcessMinCheckInterval_) {
				checkSurvivalProcess();
				survivalProcessChecked_ = now;
			}
			// setup symbol names in model data
			for (auto& model : models) {
				// resolve symbol names
				auto ip = model->getIp();
				pid_t pid = model->getPid();
				model->setSymbolName(resolve(pid, ip));
				auto& callChainIps = model->getCallChainIps();
				auto& callChainSymbolNames = model->getCallChainSymbolNames();
				for (std::size_t i = 0; i < callChainIps.size(); ++i) {
					auto callChainIp = callChainIps[i];
					callChainSymbolNames.at(i) = resolve(pid, callChainIp);
				}
			}
		}

		/** Constructor */
		CpuSampleLinuxSymbolResolveInterceptor() :
			pidToAddressLocator_(),
			addressLocatorAllocator_(DefaultMaxFreeAddressLocator),
			lastAddressLocatorPid_(0),
			lastAddressLocatorIterator_(),
			pathAllocator_(std::make_shared<decltype(pathAllocator_)::element_type>()),
			resolverAllocator_(std::make_shared<decltype(resolverAllocator_)::element_type>()),
			kernelResolver_(),
			pidToCustomResolver_(),
			customResolverAllocator_(DefaultMaxFreeCustomResolver),
			customSymbolNamePath_(std::make_shared<std::string>("perfmap")),
			customSymbolNameAllocator_(std::make_shared<SingletonAllocator<std::string, SymbolName>>()),
			lastCustomResolverPid_(0),
			lastCustomResolverIterator_(),
			survivalProcessChecked_(),
			survivalProcessMinCheckInterval_(
				std::chrono::milliseconds(+DefaultSurvivalProcessMinCheckInterval)) { }

	protected:
		/** Find out which process no longer exist and cleanup */
		void checkSurvivalProcess() {
			// cleanup pidToAddressLocator_
			lastAddressLocatorPid_ = 0;
			lastAddressLocatorIterator_ = {};
			for (auto it = pidToAddressLocator_.begin(); it != pidToAddressLocator_.end();) {
				pid_t pid = it->first;
				if (LinuxProcessUtils::isProcessExists(pid)) {
					++it;
				} else {
					addressLocatorAllocator_.deallocate(std::move(it->second));
					it = pidToAddressLocator_.erase(it);
				}
			}
			// cleanup pidToCustomResolver_
			lastCustomResolverPid_ = 0;
			lastCustomResolverIterator_ = {};
			for (auto it = pidToCustomResolver_.begin(); it != pidToCustomResolver_.end();) {
				pid_t pid = it->first;
				if (pidToAddressLocator_.count(pid) > 0) {
					++it;
				} else {
					customResolverAllocator_.deallocate(std::move(it->second));
					it = pidToCustomResolver_.erase(it);
				}
			}
		}

		/**
		 * Resolve symbol name by pid and ip.
		 * Return nullptr if no symbol name is found.
		 */
		std::shared_ptr<SymbolName> resolve(pid_t pid, std::uint64_t ip) {
			// find or create address locator by pid
			// cache last result to improve performance
			decltype(lastAddressLocatorIterator_) addressLocatorIt = {};
			if (pid == lastAddressLocatorPid_) {
				addressLocatorIt = lastAddressLocatorIterator_;
			} else {
				addressLocatorIt = pidToAddressLocator_.find(pid);
				if (addressLocatorIt == pidToAddressLocator_.end()) {
					auto pair = pidToAddressLocator_.emplace(pid,
						addressLocatorAllocator_.allocate(pid, pathAllocator_));
					addressLocatorIt = pair.first;
				}
				lastAddressLocatorPid_ = pid;
				lastAddressLocatorIterator_ = addressLocatorIt;
			}
			// although ip is the next instruction of the executing instruction,
			// the executing instruction is rare to be ret,
			// moretimes, the next instruction would be the entry point of a dynamic function,
			// so here use ip, not ip-1.
			std::shared_ptr<SymbolName> symbolName;
			auto pathAndOffset = addressLocatorIt->second->locate(ip, false);
			if (pathAndOffset.first != nullptr) {
				auto resolver = resolverAllocator_->allocate(std::move(pathAndOffset.first));
				symbolName = resolver->resolve(pathAndOffset.second);
			} else {
				symbolName = kernelResolver_.resolve(ip);
			}
			if (symbolName == nullptr) {
				// find or create custom resolver by pid
				// cache last result to improve performance
				decltype(lastCustomResolverIterator_) customResolverIt = {};
				if (pid == lastCustomResolverPid_) {
					customResolverIt = lastCustomResolverIterator_;
				} else {
					customResolverIt = pidToCustomResolver_.find(pid);
					if (customResolverIt == pidToCustomResolver_.end()) {
						auto pair = pidToCustomResolver_.emplace(pid,
							customResolverAllocator_.allocate(
								pid,
								customSymbolNamePath_,
								customSymbolNameAllocator_));
						customResolverIt = pair.first;
					}
					lastCustomResolverPid_ = pid;
					lastCustomResolverIterator_ = customResolverIt;
				}
				symbolName = customResolverIt->second->resolve(ip, false);
			}
			return symbolName;
		}

	protected:
		// address -> (file, offset)
		std::unordered_map<pid_t, std::unique_ptr<LinuxProcessAddressLocator>> pidToAddressLocator_;
		FreeListAllocator<LinuxProcessAddressLocator> addressLocatorAllocator_;
		pid_t lastAddressLocatorPid_;
		decltype(pidToAddressLocator_)::iterator lastAddressLocatorIterator_;
		// (file, offset) -> symbol
		std::shared_ptr<SingletonAllocator<std::string, std::string>> pathAllocator_;
		std::shared_ptr<SingletonAllocator<
			std::shared_ptr<std::string>, LinuxExecutableSymbolResolver>> resolverAllocator_;
		// address -> kernel symbol
		LinuxKernelSymbolResolver kernelResolver_;
		// address -> custom symbol
		std::unordered_map<pid_t, std::unique_ptr<LinuxProcessCustomSymbolResolver>> pidToCustomResolver_;
		FreeListAllocator<LinuxProcessCustomSymbolResolver> customResolverAllocator_;
		std::shared_ptr<std::string> customSymbolNamePath_;
		std::shared_ptr<SingletonAllocator<std::string, SymbolName>> customSymbolNameAllocator_;
		pid_t lastCustomResolverPid_;
		decltype(pidToCustomResolver_)::iterator lastCustomResolverIterator_;

		std::chrono::high_resolution_clock::time_point survivalProcessChecked_;
		std::chrono::high_resolution_clock::duration survivalProcessMinCheckInterval_;
	};
}

