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
			// find first symbol that fileOffsetEnd > address
			auto it = std::upper_bound(
				symbolNames_.cbegin(), symbolNames_.cend(), address,
				[](const auto& a, const auto& b) {
					return a < b->getFileOffsetEnd();
				});
			if (it == symbolNames_.cend()) {
				return nullptr;
			}
			// check is address >= fileOffsetStart and address < fileOffsetEnd
			const auto& symbolName = *it;
			if (address >= symbolName->getFileOffsetStart()) {
				return symbolName;
			}
			return nullptr;
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
		/**
		 * Line format:
		 * address          type name           module(optional)
		 * ffffffffc012340d t    cleanup_module [pata_acpi]
		 */
		void loadSymbolNames() {
			// parse file
			std::ifstream file("/proc/kallsyms");
			std::string line;
			while (std::getline(file, line)) {
				std::uintptr_t startAddress = 0;
				std::string functionName;
				StringUtils::split(line,
					[&line, &startAddress, &functionName]
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
					symbolName->setOriginalName(std::move(functionName));
					symbolName->setDemangleName("");
					symbolName->setPath(path_);
					static_assert(
						std::numeric_limits<std::uintptr_t>::max() ==
						static_cast<std::size_t>(std::numeric_limits<std::uintptr_t>::max()),
						"ensure case std::uintptr_t to std::size_t will not cause overflow");
					symbolName->setFileOffsetStart(static_cast<std::size_t>(startAddress));
					symbolNames_.emplace_back(std::move(symbolName));
				}
			}
			// sort symbol names and guess size
			std::sort(symbolNames_.begin(), symbolNames_.end(), [](auto& a, auto& b) {
				return a->getFileOffsetStart() < b->getFileOffsetStart();
			});
			for (auto it = symbolNames_.begin(); it < symbolNames_.end(); ++it) {
				auto next = it + 1;
				if (next < symbolNames_.end()) {
					(*it)->setFileOffsetEnd((*next)->getFileOffsetStart());
				} else {
					(*it)->setFileOffsetEnd((*it)->getFileOffsetStart() + 1);
				}
			}
			// set min address and max address
			minAddress_ = symbolNames_.empty() ? 0 : symbolNames_.front()->getFileOffsetStart();
			maxAddress_ = symbolNames_.empty() ? 0 : symbolNames_.back()->getFileOffsetEnd();
		}

	protected:
		std::shared_ptr<std::string> path_;
		std::vector<std::shared_ptr<SymbolName>> symbolNames_;
		std::uintptr_t minAddress_;
		std::uintptr_t maxAddress_;
	};
}

