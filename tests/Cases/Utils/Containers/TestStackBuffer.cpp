#include <iostream>
#include <cassert>
#include <LiveProfiler/Utils/Containers/StackBuffer.hpp>

namespace LiveProfilerTests {
	using namespace LiveProfiler;

	void testStackBuffer() {
		std::cout << __func__ << std::endl;
		StackBuffer<128> buffer;
		assert(buffer.appendStr("abc", 3));
		assert(buffer.appendLongLong(123));
		assert(buffer.appendStr("qwert", 5));
		assert(buffer.appendNullTerminator());
		assert(std::string(buffer.data()) == "abc123qwert");
		assert(buffer.offset() == 12);
		assert(!buffer.empty());

		buffer.clear();
		assert(buffer.empty());
		assert(buffer.appendNullTerminator());
		assert(std::string(buffer.data()) == "");
		assert(buffer.offset() == 1);
		assert(!buffer.empty());

		StackBuffer<1> smallBuffer;
		assert(!smallBuffer.appendStr("abc", 3));
		assert(!smallBuffer.appendLongLong(123));
		assert(smallBuffer.appendNullTerminator());
		assert(!smallBuffer.appendNullTerminator());
		assert(smallBuffer.offset() == 1);
		assert(!buffer.empty());
	}
}

