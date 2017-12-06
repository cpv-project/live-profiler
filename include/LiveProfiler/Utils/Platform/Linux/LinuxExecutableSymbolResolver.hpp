#pragma once
#include <cassert>
#include <vector>
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
		LinuxExecutableSymbolResolver(const std::string&) :
			path_(),
			symbolNames_() { }

		/** Resolve symbol name from offset, return nullptr if not found */
		std::shared_ptr<SymbolName> resolve(std::size_t offset) {
			return nullptr;
		}

	protected:
		std::shared_ptr<std::string> path_;
		std::vector<std::shared_ptr<SymbolName>> symbolNames_;
	};
}

