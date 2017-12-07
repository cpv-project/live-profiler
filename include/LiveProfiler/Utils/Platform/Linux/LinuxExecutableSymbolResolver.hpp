#pragma once
#include <bfd.h>
#include <cassert>
#include <cstring>
#include <vector>
#include <memory>
#include <algorithm>
#include "../../../Models/Shared/SymbolName.hpp"

namespace LiveProfiler {
	/**
	 * Class used to resolve symbol name from single linux executable file
	 * For how to get the symbols from executable file please see nm.c in binutils.
	 * This class require binutils-dev to be installed.
	 */
	class LinuxExecutableSymbolResolver {
	public:
		/** Getters */
		const std::shared_ptr<std::string>& getPath() const& { return path_; }

		/** Constructor */
		LinuxExecutableSymbolResolver(const std::shared_ptr<std::string>& path) :
			path_(path),
			symbolNames_() {
			loadSymbolNames();
		}

		/** Resolve symbol name from offset, return nullptr if not found */
		std::shared_ptr<SymbolName> resolve(std::size_t offset) {
			// find first symbol that fileOffset > offset
			auto it = std::upper_bound(
				symbolNames_.cbegin(), symbolNames_.cend(), offset,
				[](const auto& a, const auto& b) {
					return a < b->getFileOffset();
				});
			// find out which previous symbol contains this offset
			// only check the symbols that have same address with the first
			// gives [ ..., s', s', s, s, s, it, ... ], check [s, s, s]
			std::size_t fileOffset = 0;
			for (bool isFirst = true; it != symbolNames_.cbegin(); isFirst = false) {
				--it;
				if (offset < (*it)->getFileOffset() + (*it)->getSymbolSize()) {
					return *it;
				}
				if (isFirst) {
					fileOffset = (*it)->getFileOffset();
				} else if (fileOffset != (*it)->getFileOffset()) {
					break;
				}
			}
			return nullptr;
		}

	protected:
		/**
		 * Load symbol names from executable file
		 * see: https://gist.github.com/303248153/f792ed53a4245b46900d4ce3926888e6
		 * and please compare the result with `nm -S $path`
		 */
		void loadSymbolNames() {
			assert(path_ != nullptr);
			// if path is empty, don't load
			if (path_->empty()) {
				return;
			}
			// if cannot open file, don't load
			::bfd* file = ::bfd_openr(path_->c_str(), nullptr);
			if (file == nullptr) {
				return;
			}
			std::unique_ptr<::bfd, int(*)(::bfd*)> filePtr(file, ::bfd_close);
			// if file isn't object file, don't load
			char** matching = nullptr;
			if (!::bfd_check_format_matches(file, ::bfd_object, &matching)) {
				return;
			}
			// load symbols
			void* miniSymbols = nullptr;
			unsigned int miniSymbolSize = 0;
			std::size_t miniSymbolCount = bfd_read_minisymbols(
				file, false, &miniSymbols, &miniSymbolSize);
			if (miniSymbolCount == 0 || miniSymbols == nullptr) {
				return;
			}
			std::unique_ptr<void, void(*)(void*)> miniSymbolsPtr(miniSymbols, ::free);
			// load symbol sizes and append to symbolNames_
			bfd_byte* miniSymbolsFrom = reinterpret_cast<bfd_byte*>(miniSymbols);
			bfd_byte* miniSymbolsTo = miniSymbolsFrom + miniSymbolSize * miniSymbolCount;
			asymbol* symbolStore = bfd_make_empty_symbol(file);
			std::vector<asymbol*> symbols;
			for (; miniSymbolsFrom < miniSymbolsTo; miniSymbolsFrom += miniSymbolSize) {
				asymbol* symbol = bfd_minisymbol_to_symbol(file, false, miniSymbolsFrom, symbolStore);
				if (symbol != nullptr) {
					symbols.emplace_back(symbol);
				}
			}
			std::sort(symbols.begin(), symbols.end(), [](const auto& a, const auto& b) {
				// sort by symbol value, then by section vma
				// see `size_forward1` in nm.c in binutils
				auto valueA = bfd_asymbol_value(a);
				auto valueB = bfd_asymbol_value(b);
				if (valueA != valueB) {
					return valueA < valueB;
				}
				auto baseA = bfd_asymbol_base(a); // section->vma
				auto baseB = bfd_asymbol_base(b);
				return baseA < baseB;
			});
			static const std::string elf32("elf32");
			static const std::string elf64("elf64");
			const char* target = bfd_get_target(file);
			bool isElf32 = std::strncmp(target, elf32.c_str(), elf32.size()) == 0;
			bool isElf64 = std::strncmp(target, elf64.c_str(), elf64.size()) == 0;
			for (auto it = symbols.begin(); it < symbols.end(); ++it) {
				asymbol* symbol = *it;
				asymbol* next = (it + 1 < symbols.end()) ? *(it + 1) : nullptr;
				// get size from elf format
				std::size_t size = 0;
				if ((symbol->flags & (BSF_SECTION_SYM | BSF_SYNTHETIC)) != 0) {
					// don't have a full type set of data available, see nm.c for full explanation
				} else if (isElf32) {
					size = *reinterpret_cast<std::uint32_t*>(
						reinterpret_cast<char*>(symbol)+sizeof(asymbol)+8);
				} else if (isElf64) {
					size = *reinterpret_cast<std::uint64_t*>(
						reinterpret_cast<char*>(symbol)+sizeof(asymbol)+8);
				}
				// guess size by next symbol in same section
				std::size_t guessSize = 0;
				if (next != nullptr && bfd_get_section(symbol) == bfd_get_section(next)) {
					guessSize = bfd_asymbol_value(next) - bfd_asymbol_value(symbol);
				} else {
					guessSize = (bfd_asymbol_base(symbol) +
						bfd_section_size(file, bfd_get_section(symbol)) - bfd_asymbol_value(symbol));
				}
				if (size == 0) {
					// guestSize may be 0 if two symbol have same address
					// guestSize may less than size if file only contains the first part
					size = guessSize;
				}
				// demangle name, these flags are defined in demangle.h in binutils
				static const int DMGL_ANSI = (1 << 1);
				static const int DMGL_PARAMS = (1 << 0);
				const char* originalName = bfd_asymbol_name(symbol);
				char* demangleName = ::bfd_demangle(file, originalName, DMGL_ANSI | DMGL_PARAMS);
				std::unique_ptr<char, void(*)(void*)> demangleNamePtr(demangleName, ::free);
				// append to symbolNames_
				auto symbolName = std::make_shared<SymbolName>();
				symbolName->setOriginalName(originalName);
				symbolName->setDemangleName((demangleName != nullptr) ? demangleName : "");
				symbolName->setPath(path_);
				symbolName->setFileOffset(bfd_asymbol_value(symbol));
				symbolName->setSymbolSize(size);
				symbolNames_.emplace_back(std::move(symbolName));
			}
			// symbolNames_ should be already sorted by file offset
			// the order of symbol names that have same file offset is undefined for now
		}

	protected:
		std::shared_ptr<std::string> path_;
		std::vector<std::shared_ptr<SymbolName>> symbolNames_;
	};
}

