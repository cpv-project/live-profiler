#include <cassert>
#include <iostream>
#include <algorithm>
#include <LiveProfiler/Utils/LinuxPerfUtils.hpp>

namespace LiveProfilerTests {
	using namespace LiveProfiler;

	void testLinuxPerfEntryAllocator() {
		LinuxPerfEntryAllocator allocator(3);
		for (int i = 0; i < 3; ++i) {
			auto entry = allocator.allocate();
			assert(entry != nullptr);
			assert(entry->attr.type == 0);
			assert(entry->attr.config == 0);
			assert(entry->pid == 0);
			assert(entry->fd == 0);
			assert(entry->mmapBuffer.size() == ::getpagesize() * 3);
			assert(entry->mmapBuffer.size() ==
				std::count(entry->mmapBuffer.cbegin(), entry->mmapBuffer.cend(), 0));
			allocator.deallocate(std::move(entry));
		}
	}

	void testLinuxPerfUtils() {
		std::cout << __func__ << std::endl;
		testLinuxPerfEntryAllocator();
	}
}

