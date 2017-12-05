#include <iostream>
#include <cassert>
#include <LiveProfiler/Utils/Allocators/SingletonAllocator.hpp>
#include <LiveProfiler/Utils/Platform/Linux/LinuxExecutableSymbolResolver.hpp>
#include <LiveProfiler/Utils/Platform/Linux/LinuxProcessAddressMap.hpp>

namespace LiveProfilerTests {
	using namespace LiveProfiler;

	void testLinuxProcessAddressMap() {
		std::cout << __func__ << std::endl;
		auto pathAllocator = std::make_shared<SingletonAllocator<std::string, std::string>>();
		auto resolverAllocator = std::make_shared<
			SingletonAllocator<std::string, LinuxExecutableSymbolResolver>>();
		{
			std::string line("08048000-08056000 r-xp        00000000 03:0c  64593 /usr/sbin/gpm");
			LinuxProcessAddressMap map;
			assert(map.parseLine(line, pathAllocator, resolverAllocator));
			assert(map.getStartAddress() == 0x8048000);
			assert(map.getEndAddress() == 0x8056000);
			assert(map.getFileOffset() == 0);
			assert(*map.getPath() == "/usr/sbin/gpm");
			assert(map.getResolver() != nullptr);
		}
		{
			std::string line("08048000-08056000 r-xp        00000100 03:0c  64593");
			LinuxProcessAddressMap map;
			assert(map.parseLine(line, pathAllocator, resolverAllocator));
			assert(map.getStartAddress() == 0x8048000);
			assert(map.getEndAddress() == 0x8056000);
			assert(map.getFileOffset() == 0x100);
			assert(*map.getPath() == "");
			assert(map.getResolver() != nullptr);
		}
		{
			std::string line("08048000- r-xp        00000000 03:0c  64593 /usr/sbin/gpm");
			LinuxProcessAddressMap map;
			assert(!map.parseLine(line, pathAllocator, resolverAllocator));
		}
		{
			std::string line("08048000-08056000 r-xp        00000000 03:0c  ");
			LinuxProcessAddressMap map;
			assert(!map.parseLine(line, pathAllocator, resolverAllocator));
		}
		{
			std::string line("08048000-08056000 r-xp        * 03:0c  64593 /usr/sbin/gpm");
			LinuxProcessAddressMap map;
			assert(!map.parseLine(line, pathAllocator, resolverAllocator));
		}
	}
}

