#pragma once
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <array>

namespace LiveProfiler {
	struct TypeConvertUtils {
		/** Convert c string to long long, return whether the conversion is successful */
		static bool strToLongLong(const char* str, long long& result, int base = 10) {
			char* endptr = nullptr;
			result = std::strtoll(str, &endptr, base);
			if (endptr == nullptr || endptr == str) {
				return false;
			}
			return true;
		}

		/** Convert c string to unsigned long long, return whether the conversion is successful */
		static bool strToUnsignedLongLong(const char* str, unsigned long long& result, int base = 10) {
			char* endptr = nullptr;
			result = std::strtoull(str, &endptr, base);
			if (endptr == nullptr || endptr == str) {
				return false;
			}
			return true;
		}

		/** Convert long long to c string, return how many characters appended (exclude null) */
		static std::size_t longLongToStr(char* str, std::size_t size, long long value) {
			static constexpr std::size_t cacheSize = 32768;
			static std::array<std::string, cacheSize> cache = ([] {
				// `cache` should link to same variable
				std::array<std::string, cacheSize> result;
				for (std::size_t i = 0; i < cacheSize; ++i) {
					auto& str = result.at(i);
					str.resize(100);
					auto ret = snprintf(&str.front(), str.size(), "%lld", static_cast<long long>(i));
					str.resize(ret);
				}
				// cppcheck-suppress CastAddressToIntegerAtReturn
				return result;
			})();
			// cppcheck-suppress unsignedLessThanZero
			if (size <= 0) {
				return 0;
			} else if (value >= 0 && value < static_cast<long long>(cache.size())) {
				// use string in cache
				const auto& cacheStr = cache[value];
				if (size < cacheStr.size() + 1) {
					return 0; // size not enough
				}
				std::memcpy(str, cacheStr.data(), cacheStr.size());
				str[cacheStr.size()] = '\0';
				return cacheStr.size();
			} else {
				// use snprintf
				auto ret = snprintf(str, size, "%lld", value);
				if (ret <= 0 || size < static_cast<std::size_t>(ret) + 1) {
					return 0; // size not enough
				}
				return ret;
			}
		}
	};
}

