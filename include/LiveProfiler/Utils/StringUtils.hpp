#pragma once
#include <string>

namespace LiveProfiler {
	struct StringUtils {
		/**
		 * Split string with specified characters.
		 * Call func(startIndex, endIndex, count) while split.
		 * Default split characters are empty characters.
		 */
		template <class Func>
		static void split(
			const std::string& str, const Func& func, const char* chars = " \t\r\n") {
			std::size_t startIndex = 0;
			std::size_t count = 0;
			while (startIndex < str.size()) {
				auto index = str.find_first_of(chars, startIndex);
				auto endIndex = (index == str.npos) ? str.size() : index;
				func(startIndex, endIndex, count);
				index = str.find_first_not_of(chars, endIndex);
				startIndex = (index == str.npos) ? str.size() : index;
				++count;
			}
		}
	};
}

