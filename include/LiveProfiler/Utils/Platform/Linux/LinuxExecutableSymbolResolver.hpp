#pragma once
#include <bfd.h>
#include <elf.h>
#include <cassert>
#include <cstring>
#include <vector>
#include <memory>
#include <fstream>
#include <algorithm>
#include <array>
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

		/** Resolve symbol name from file offset, return nullptr if not found */
		std::shared_ptr<SymbolName> resolve(std::size_t offset) {
			// find first symbol that fileOffsetEnd > offset
			auto it = std::upper_bound(
				symbolNames_.cbegin(), symbolNames_.cend(), offset,
				[](const auto& a, const auto& b) {
					return a < b->getFileOffsetEnd();
				});
			if (it == symbolNames_.cend()) {
				return nullptr;
			}
			// check is offset >= fileOffsetStart and offset < fileOffsetEnd
			// since the smallest fileOffsetStart will come first when fileOffsetEnd is equal
			// only check the first element
			const auto& symbolName = *it;
			if (offset >= symbolName->getFileOffsetStart()) {
				return symbolName;
			}
			return nullptr;
		}

		/** Constructor */
		explicit LinuxExecutableSymbolResolver(const std::shared_ptr<std::string>& path) :
			path_(path),
			loadEntries_(),
			symbolNames_() {
			loadLoadEntries();
			loadSymbolNames();
		}

	protected:
		/** Load LOAD entries from elf program headers by elf class type */
		template <class EHdr, class Phdr>
		void loadLoadEntriesByElfClass(std::ifstream& file) {
			EHdr header;
			file.seekg(0);
			if (!file.read(reinterpret_cast<char*>(&header), sizeof(header))) {
				return; // read elf header failed
			}
			std::vector<Phdr> programHeaders;
			programHeaders.resize(header.e_phnum);
			file.seekg(header.e_phoff);
			if (!file.read(
				reinterpret_cast<char*>(programHeaders.data()),
				sizeof(Phdr) * programHeaders.size())) {
				return; // read program headers failed
			}
			for (auto& programHeader : programHeaders) {
				if (programHeader.p_type == PT_LOAD) {
					// found LOAD entry
					loadEntries_.emplace_back(LoadEntry({
						static_cast<std::size_t>(programHeader.p_offset),
						static_cast<std::size_t>(programHeader.p_vaddr),
						static_cast<std::size_t>(programHeader.p_vaddr + programHeader.p_memsz)
					}));
				}
			}
		}

		/** Load LOAD entries from elf program headers */
		void loadLoadEntries() {
			assert(path_ != nullptr);
			std::ifstream file(*path_);
			std::array<char, EI_NIDENT> ident;
			if (!file.read(ident.data(), ident.size())) {
				return; // read ident failed
			}
			if (std::memcmp(ident.data(), ELFMAG, SELFMAG) != 0) {
				return; // magic not matched
			}
			auto elfClass = ident.at(EI_CLASS);
			if (elfClass == ELFCLASS32) {
				loadLoadEntriesByElfClass<Elf32_Ehdr, Elf32_Phdr>(file);
			} else if (elfClass == ELFCLASS64) {
				loadLoadEntriesByElfClass<Elf64_Ehdr, Elf64_Phdr>(file);
			} else {
				// other elf classes are unsupported, but there no others for now
			}
		}

		/**
		 * Load symbol names from executable file
		 * 
		 * First use bfd api from binutils to find symbol names,
		 * Then use LOAD entry from elf program headers to calcualte the file offset.
		 *
		 * Test Code: https://gist.github.com/303248153/f792ed53a4245b46900d4ce3926888e6
		 * Please compare the result with `nm -S $path`.
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
			static auto load = [](::bfd* file, std::vector<asymbol*>& symbols, bool dynamic) {
				void* miniSymbols = nullptr;
				unsigned int miniSymbolSize = 0;
				std::size_t miniSymbolCount = bfd_read_minisymbols(
					file, dynamic, &miniSymbols, &miniSymbolSize);
				std::unique_ptr<void, void(*)(void*)> miniSymbolsPtr(miniSymbols, ::free);
				if (miniSymbolCount == 0 ||  miniSymbols == nullptr) {
					return miniSymbolsPtr;
				}
				bfd_byte* miniSymbolsFrom = reinterpret_cast<bfd_byte*>(miniSymbols);
				bfd_byte* miniSymbolsTo = miniSymbolsFrom + miniSymbolSize * miniSymbolCount;
				asymbol* symbolStore = bfd_make_empty_symbol(file);
				for (; miniSymbolsFrom < miniSymbolsTo; miniSymbolsFrom += miniSymbolSize) {
					asymbol* symbol = bfd_minisymbol_to_symbol(
						file, dynamic, miniSymbolsFrom, symbolStore);
					// see filter_symbols in nm.c in binutils
					if (symbol == nullptr ||
						(symbol->flags & (BSF_SECTION_SYM | BSF_DEBUGGING)) != 0) {
						continue;
					}
					symbols.emplace_back(symbol);
				}
				return miniSymbolsPtr;
			};
			std::vector<asymbol*> symbols;
			auto normalMiniSymbolsPtr = load(file, symbols, false);
			auto dynamicMiniSymbolsPtr = load(file, symbols, true);
			// load symbol sizes and append to symbolNames_
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
				bool sizeIsSure = false;
				if ((symbol->flags & (BSF_SECTION_SYM | BSF_SYNTHETIC)) != 0) {
					// don't have a full type set of data available, see nm.c for full explanation
				} else if (isElf32) {
					size = *reinterpret_cast<std::uint32_t*>(
						reinterpret_cast<char*>(symbol)+sizeof(asymbol)+8);
					sizeIsSure = true;
				} else if (isElf64) {
					size = *reinterpret_cast<std::uint64_t*>(
						reinterpret_cast<char*>(symbol)+sizeof(asymbol)+8);
					sizeIsSure = true;
				}
				// guess size by next symbol in same section
				std::size_t guessSize = 0;
				if (next != nullptr && bfd_get_section(symbol) == bfd_get_section(next)) {
					guessSize = bfd_asymbol_value(next) - bfd_asymbol_value(symbol);
				} else {
					guessSize = (bfd_asymbol_base(symbol) +
						bfd_section_size(file, bfd_get_section(symbol)) - bfd_asymbol_value(symbol));
				}
				if (!sizeIsSure) {
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
				// convert virtual address to file offset
				// usually there very few LOAD entries so it's not necessary to do binary search
				auto fileOffset = bfd_asymbol_value(symbol);
				for (const auto& loadEntry : loadEntries_) {
					if (fileOffset >= loadEntry.virtualAddressStart &&
						fileOffset < loadEntry.virtualAddressEnd) {
						fileOffset = fileOffset - loadEntry.virtualAddressStart + loadEntry.fileOffset;
						break;
					}
				}
				// append to symbolNames_
				auto symbolName = std::make_shared<SymbolName>();
				symbolName->setOriginalName(originalName);
				if (demangleName != nullptr && symbolName->getOriginalName() != demangleName) {
					symbolName->setDemangleName(demangleName);
				} else {
					symbolName->setDemangleName("");
				}
				symbolName->setPath(path_);
				symbolName->setFileOffsetStart(fileOffset);
				symbolName->setFileOffsetEnd(fileOffset + size);
				symbolNames_.emplace_back(std::move(symbolName));
			}
			// sort symbolNames_ by file offset
			// it's already sorted by virtual address
			std::sort(symbolNames_.begin(), symbolNames_.end(), [](auto& a, auto& b) {
				if (a->getFileOffsetEnd() != b->getFileOffsetEnd()) {
					return a->getFileOffsetEnd() < b->getFileOffsetEnd();
				}
				return a->getFileOffsetStart() < b->getFileOffsetStart();
			});
		}

		/** Represent LOAD entry in elf program headers */
		struct LoadEntry {
			std::size_t fileOffset;
			std::size_t virtualAddressStart;
			std::size_t virtualAddressEnd;
		};

	protected:
		std::shared_ptr<std::string> path_;
		std::vector<LoadEntry> loadEntries_;
		std::vector<std::shared_ptr<SymbolName>> symbolNames_;
	};
}

