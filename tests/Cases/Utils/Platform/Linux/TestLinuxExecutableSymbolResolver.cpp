#include <unistd.h>
#include <iostream>
#include <cassert>
#include <LiveProfiler/Utils/Allocators/SingletonAllocator.hpp>
#include <LiveProfiler/Utils/Platform/Linux/LinuxProcessAddressLocator.hpp>
#include <LiveProfiler/Utils/Platform/Linux/LinuxExecutableSymbolResolver.hpp>

namespace LiveProfilerTests {
	using namespace LiveProfiler;

	void testLinuxExecutableSymbolResolverResolveSelf() {
		auto pathAllocator = std::make_shared<SingletonAllocator<std::string, std::string>>();
		LinuxProcessAddressLocator locator;
		locator.reset(::getpid(), pathAllocator);
		auto address = reinterpret_cast<uintptr_t>(
			&testLinuxExecutableSymbolResolverResolveSelf) + 1;
		auto pathAndOffset = locator.locate(address, false);
		assert(pathAndOffset.first != nullptr);
		assert(pathAndOffset.second > 0);
		LinuxExecutableSymbolResolver resolver(pathAndOffset.first);
		auto symbolName = resolver.resolve(pathAndOffset.second);
		assert(symbolName != nullptr);
		assert(symbolName->getOriginalName().find(__func__) != std::string::npos);
		assert(symbolName->getName().find(__func__) != std::string::npos);
		assert(symbolName->getPath() == pathAndOffset.first);
		assert(symbolName->getFileOffsetStart() > 0);
		assert(symbolName->getFileOffsetEnd() > symbolName->getFileOffsetStart());
	}

	void testLinuxExecutableSymbolResolver() {
		std::cout << __func__ << std::endl;
		testLinuxExecutableSymbolResolverResolveSelf();
	}
}

