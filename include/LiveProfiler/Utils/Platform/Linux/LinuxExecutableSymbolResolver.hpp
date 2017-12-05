#pragma once
#include <vector>

namespace LiveProfiler {
	/**
	 * Class used to resolve symbol name from single linux executable file
	 * For how to get the symbols from executable file please see nm.c in binutils.
	 * This class require binutils-dev to be installed.
	 */
	class LinuxExecutableSymbolResolver {
	public:
		LinuxExecutableSymbolResolver(const std::string& path) {
		}
	protected:
	};
}

