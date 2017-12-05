#include <iostream>
#include <cassert>
#include <LiveProfiler/Utils/Allocators/SingletonAllocator.hpp>

namespace LiveProfilerTests {
	using namespace LiveProfiler;

	void testSingletonAllocator() {
		std::cout << __func__ << std::endl;
		SingletonAllocator<std::string, std::string> allocator;
		auto a = allocator.allocate("abc");
		auto b = allocator.allocate(std::string("abc"));
		auto c = allocator.allocate("asd", 3);
		assert(*a == "abc");
		assert(*b == "abc");
		assert(a == b);
		assert(*c == "asd");
		assert(a != c);
	}
}

