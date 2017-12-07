#include <unistd.h>
#include <iostream>
#include <cassert>
#include <LiveProfiler/Utils/Platform/Linux/LinuxProcessAddressLocator.hpp>

namespace LiveProfilerTests {
	using namespace LiveProfiler;

	void testLinuxProcessAddressLocatorLocateSelf() {
		auto pathAllocator = std::make_shared<SingletonAllocator<std::string, std::string>>();
		LinuxProcessAddressLocator locator;
		locator.reset(::getpid(), pathAllocator);
		auto address = reinterpret_cast<uintptr_t>(
			&testLinuxProcessAddressLocatorLocateSelf) + 1;
		auto pathAndOffset = locator.locate(address, false);
		assert(pathAndOffset.first != nullptr);
		assert(pathAndOffset.second > 0);
	}

	void testLinuxProcessAddressLocator() {
		std::cout << __func__ << std::endl;
		testLinuxProcessAddressLocatorLocateSelf();
	}
}

