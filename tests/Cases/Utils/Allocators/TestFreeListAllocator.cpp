#include <cassert>
#include <iostream>
#include <algorithm>
#include <LiveProfiler/Utils/Allocators/FreeListAllocator.hpp>

namespace LiveProfilerTests {
	using namespace LiveProfiler;

	namespace {
		struct Foo {
			std::size_t freeCount;
			std::size_t resetCount;
			void freeResources() { ++freeCount; }
			void reset() { ++resetCount; }
		};
	}

	void testFreeListAllocator() {
		std::cout << __func__ << std::endl;
		FreeListAllocator<Foo> allocator(3);
		for (std::size_t i = 0; i < 3; ++i) {
			auto foo = allocator.allocate();
			assert(foo->freeCount == i);
			assert(foo->resetCount == i);
			allocator.deallocate(std::move(foo));
		}
		for (std::size_t i = 0; i < 3; ++i) {
			auto a = allocator.allocate();
			auto b = allocator.allocate();
			auto c = allocator.allocate();
			assert(a != b && b != c && a != c);
			allocator.deallocate(std::move(a));
			allocator.deallocate(std::move(b));
			allocator.deallocate(std::move(c));
		}
	}
}

