#include <cassert>
#include <iostream>
#include <algorithm>
#include <LiveProfiler/Utils/LinuxPerfEntryAllocator.hpp>

namespace LiveProfilerTests {
	using namespace LiveProfiler;

	void testLinuxPerfEntryAllocator() {
		std::cout << __func__ << std::endl;
		std::size_t defaultCount = 3;
		LinuxPerfEntryAllocator allocator(defaultCount);
		for (int i = 0; i < 3; ++i) {
			assert(allocator.getMmapPageCount() == defaultCount+i);
			allocator.setMmapPageCount(defaultCount+i+1);
			auto entry = allocator.allocate();
			assert(entry != nullptr);
			assert(entry->getAttrRef().type == 0);
			assert(entry->getAttrRef().config == 0);
			assert(entry->getPid() == 0);
			assert(entry->getFd() == 0);
			assert(entry->getMmapPageCount() == defaultCount+i+1);
			assert(entry->getMmapPageSize() == static_cast<std::size_t>(::getpagesize()));
			allocator.deallocate(std::move(entry));
		}
	}
}
