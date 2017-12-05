#include <iostream>
#include <cassert>
#include <limits>
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
		assert(TypeConvertUtils::strToLongLong("-abc", result, 16));
		assert(result == -0xabc);
		if (std::numeric_limits<long long>::max() >= std::numeric_limits<std::int64_t>::max()) {
			assert(TypeConvertUtils::strToLongLong("-12345678abc", result, 16));
			assert(result == -0x12345678abc);
		}
	}

	void testTypeConvertUtilsStrToUnsignedLongLong() {
		unsigned long long result = 0;
		assert(TypeConvertUtils::strToUnsignedLongLong("111", result));
		assert(result == 111);
		assert(TypeConvertUtils::strToUnsignedLongLong("1ab", result, 16));
		assert(result == 0x1ab);
		if (std::numeric_limits<unsigned long long>::max() >= std::numeric_limits<std::uint64_t>::max()) {
			assert(TypeConvertUtils::strToUnsignedLongLong("ffff0000ffff1234", result, 16));
			assert(result == 0xffff0000ffff1234);
		}
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
		testTypeConvertUtilsStrToUnsignedLongLong();
		testTypeConvertUtilsLongLongToStr();
	}
}

