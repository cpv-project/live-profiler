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

		struct Bar {
			std::size_t freeParameter;
			std::size_t resetParameter;
			void freeResources(std::size_t n) { freeParameter = n; }
			void reset(std::size_t n) { resetParameter = n; }
		};
	}

	void testFreeListAllocatorSimple() {
		FreeListAllocator<Foo> allocator(3);
		for (std::size_t i = 0; i < 3; ++i) {
			auto foo = allocator.allocate();
			assert(foo->freeCount == i);
			assert(foo->resetCount == i+1);
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

	void testFreeListAllocatorWithForwardedParameters() {
		FreeListAllocator<Bar> allocator(3);
		for (std::size_t i = 0; i < 3; ++i) {
			auto foo = allocator.allocate(i);
			assert(foo->freeParameter == i);
			assert(foo->resetParameter == i);
			allocator.deallocate(std::move(foo), i+1);
		}
		for (std::size_t i = 0; i < 3; ++i) {
			auto a = allocator.allocate(i);
			auto b = allocator.allocate(i+1);
			auto c = allocator.allocate(i+2);
			assert(a->resetParameter == i);
			assert(b->resetParameter == i+1);
			assert(c->resetParameter == i+2);
			allocator.deallocate(std::move(a), 0);
			allocator.deallocate(std::move(b), 0);
			allocator.deallocate(std::move(c), 0);
		}
	}

	void testFreeListAllocator() {
		std::cout << __func__ << std::endl;
		testFreeListAllocatorSimple();
		testFreeListAllocatorWithForwardedParameters();
	}
}

