#include <iostream>
#include <cassert>
#include <vector>
#include <LiveProfiler/Utils/StringUtils.hpp>

namespace LiveProfilerTests {
	using namespace LiveProfiler;

	void testStringUtilsSplit() {
		std::vector<std::string> results;
		std::size_t countRecord = 0;
		std::string str("aaa \tb  c\nd eee");
		StringUtils::split(str,
			[&results, &countRecord, &str](auto startIndex, auto endIndex, auto count) {
				assert(countRecord == count);
				++countRecord;
				results.emplace_back(str, startIndex, endIndex - startIndex);
			});
		assert(results.size() == 5);
		assert(results.at(0) == "aaa");
		assert(results.at(1) == "b");
		assert(results.at(2) == "c");
		assert(results.at(3) == "d");
		assert(results.at(4) == "eee");
	}

	void testStringUtils() {
		std::cout << __func__ << std::endl;
		testStringUtilsSplit();
	}
}

