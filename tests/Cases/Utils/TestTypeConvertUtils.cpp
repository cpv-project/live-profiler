#include <iostream>
#include <cassert>
#include <LiveProfiler/Utils/TypeConvertUtils.hpp>

namespace LiveProfilerTests {
	using namespace LiveProfiler;

	void testTypeConvertUtilsStrToLongLong() {
		long long result = 0;
		assert(TypeConvertUtils::strToLongLong("123", result));
		assert(result == 123);
		assert(TypeConvertUtils::strToLongLong("0", result));
		assert(result == 0);
		assert(TypeConvertUtils::strToLongLong("-10000", result));
		assert(result == -10000);
		assert(TypeConvertUtils::strToLongLong("12345678abc", result));
		assert(result == 12345678);
	}

	void testTypeConvertUtilsLongLongToStr() {
		std::array<char, 100> buf;
		assert(TypeConvertUtils::longLongToStr(buf.data(), buf.size(), 123) == 3);
		assert(std::string(buf.data()) == "123");
		assert(TypeConvertUtils::longLongToStr(buf.data(), buf.size(), 0) == 1);
		assert(std::string(buf.data()) == "0");
		assert(TypeConvertUtils::longLongToStr(buf.data(), buf.size(), -10000) == 6);
		assert(std::string(buf.data()) == "-10000");
		assert(TypeConvertUtils::longLongToStr(buf.data(), buf.size(), 12345678) == 8);
		assert(std::string(buf.data()) == "12345678");
		assert(TypeConvertUtils::longLongToStr(buf.data(), 3, 123) == 0);
		assert(TypeConvertUtils::longLongToStr(buf.data(), 1, 123) == 0);
	}

	void testTypeConvertUtils() {
		std::cout << __func__ << std::endl;
		testTypeConvertUtilsStrToLongLong();
		testTypeConvertUtilsLongLongToStr();
	}
}

