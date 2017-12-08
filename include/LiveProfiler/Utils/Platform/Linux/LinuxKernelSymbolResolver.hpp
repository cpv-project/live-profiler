#pragma once
#include <fstream>
#include <algorithm>
#include <limits>
#include "../../../Models/Shared/SymbolName.hpp"
#include "../../StringUtils.hpp"
#include "../../TypeConvertUtils.hpp"

namespace LiveProfiler {
	/**
	 * Class used to resolve symbol name from kernel.
	 * Kernel symbol names are read from /proc/kallsyms.
	 */
	class LinuxKernelSymbolResolver {
	public:
		/** Getters */
		std::uintptr_t getMinAddress() const { return minAddress_; }
		std::uintptr_t getMaxAddress() const { return maxAddress_; }

		/** Resolve kernel symbol name from address, return nullptr if not found */
		std::shared_ptr<SymbolName> resolve(std::size_t address) {
			// fast check
			if (address < minAddress_ || address >= maxAddress_) {
				return nullptr;
			}
			// find first symbol that fileOffset > address
			auto it = std::upper_bound(
				symbolNames_.cbegin(), symbolNames_.cend(), address,
				[](const auto& a, const auto& b) {
					return a < b->getFileOffset();
				});
			// get the previous symbol
			if (it == symbolNames_.cbegin()) {
				return nullptr;
			}
			--it;
			// check is address >= fileOffset and address < fileOffset + symbolSize
			const auto& symbolName = *it;
			if (address >= symbolName->getFileOffset() + symbolName->getSymbolSize()) {
				return nullptr;
			}
			return symbolName;
		}

		/** Constructor */
		LinuxKernelSymbolResolver() :
			path_(std::make_shared<std::string>("[kallsyms]")),
			symbolNames_(),
			minAddress_(0),
			maxAddress_(0) {
			loadSymbolNames();
		}

	protected:
		/** Load symbol names from /proc/kallsyms */
		void loadSymbolNames() {
			// parse file
			std::ifstream file("/proc/kallsyms");
			std::string line;
			while (std::getline(file, line)) {
				std::uintptr_t startAddress = 0;
				std::string functionName;
				StringUtils::split(line,
					[this, &line, &startAddress, &functionName]
					(auto startIndex, auto endIndex, auto count) {
					if (count == 0) {
						unsigned long long startAddressL = 0;
						if (TypeConvertUtils::strToUnsignedLongLong(
							line.c_str() + startIndex, startAddressL, 16)) {
							startAddress = static_cast<std::uintptr_t>(startAddressL);
						}
					} else if (count == 2) {
						functionName.assign(line, startIndex, endIndex - startIndex);
					}
				});
				if (startAddress != 0 && !functionName.empty()) {
					auto symbolName = std::make_shared<SymbolName>();
					symbolName->setOriginalName(functionName);
					symbolName->setDemangleName(std::move(functionName));
					symbolName->setPath(path_);
					static_assert(
						std::numeric_limits<std::uintptr_t>::max() ==
						static_cast<std::size_t>(std::numeric_limits<std::uintptr_t>::max()),
						"ensure case std::uintptr_t to std::size_t will not cause overflow");
					symbolName->setFileOffset(static_cast<std::size_t>(startAddress));
					symbolNames_.emplace_back(std::move(symbolName));
				}
			}
			// sort symbol names and guess size
			std::sort(symbolNames_.begin(), symbolNames_.end(), [](auto& a, auto& b) {
				return a->getFileOffset() < b->getFileOffset();
			});
			for (auto it = symbolNames_.begin(); it < symbolNames_.end(); ++it) {
				auto next = it + 1;
				if (next < symbolNames_.end()) {
					(*it)->setSymbolSize((*next)->getFileOffset() - (*it)->getFileOffset());
				} else {
					(*it)->setSymbolSize(1);
				}
			}
			// set min address and max address
			minAddress_ = symbolNames_.empty() ? 0 : symbolNames_.front()->getFileOffset();
			maxAddress_ = symbolNames_.empty() ? 0 : (
				symbolNames_.back()->getFileOffset() + symbolNames_.back()->getSymbolSize());
		}

	protected:
		std::shared_ptr<std::string> path_;
		std::vector<std::shared_ptr<SymbolName>> symbolNames_;
		std::uintptr_t minAddress_;
		std::uintptr_t maxAddress_;
	};
}

