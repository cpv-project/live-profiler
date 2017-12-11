#include <unistd.h>
#include <iostream>
#include <cassert>
#include <LiveProfiler/Utils/Platform/Linux/LinuxProcessCustomSymbolResolver.hpp>

namespace LiveProfilerTests {
	using namespace LiveProfiler;

	void testLinuxProcessCustomSymbolResolver() {
		std::cout << __func__ << std::endl;
		std::string path = std::string("/tmp/perf-") + std::to_string(::getpid()) + ".map";
		{
			std::ofstream file(path);
			file << "30 1a customSymbolA" << std::endl;
			file << "50 1c customSymbolB" << std::endl;
			file << "70 1e customSymbolC (checkspace)" << std::endl;
		}
		{
			LinuxProcessCustomSymbolResolver resolver;
			resolver.reset(::getpid());
			auto symbolA = resolver.resolve(0x30, false);
			auto symbolAA = resolver.resolve(0x49, false);
			auto symbolB = resolver.resolve(0x51, false);
			auto symbolC = resolver.resolve(0x8c, false);
			auto symbolNotExist = resolver.resolve(0x8e, false);
			assert(symbolA != nullptr);
			assert(symbolA == symbolAA);
			assert(symbolB != nullptr);
			assert(symbolC != nullptr);
			assert(symbolNotExist == nullptr);
			assert(symbolA->getName() == "customSymbolA");
			assert(symbolB->getName() == "customSymbolB");
			assert(symbolC->getName() == "customSymbolC (checkspace)");
		}
		::unlink(path.c_str());
	}
}

