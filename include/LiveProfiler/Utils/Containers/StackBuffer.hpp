#pragma once
#include <cstring>
#include "../TypeConvertUtils.hpp"

namespace LiveProfiler {
	/** Default initialized array allocated on stack, with offset recorded */
	template <std::size_t Size>
	class StackBuffer {
	public:
		/** Getters */
		char* data() { return data_; }
		const char* data() const { return data_; }
		std::size_t offset() const { return offset_; }

		/** Constructor */
		StackBuffer() : offset_(0) { }

		/** Reset offset to 0 */
		void clear() {
			offset_ = 0;
		}

		/** Append c string to buffer, return whether size is enough */
		bool appendStr(const char* src, std::size_t size) {
			if (offset_ + size > Size) {
				return false;
			}
			std::memcpy(data_+offset_, src, size);
			offset_ += size;
			return true;
		}

		/** Append long long to buffer, return whether size is enough */
		bool appendLongLong(long long value) {
			auto ret = TypeConvertUtils::longLongToStr(data_+offset_, Size-offset_, value);
			if (ret == 0) {
				return false;
			}
			offset_ += ret;
			return true;
		}

		/** Append null terminator to buffer, return whether size is enough */
		bool appendNullTerminator() {
			if (offset_ >= Size) {
				return false;
			}
			data_[offset_++] = '\0';
			return true;
		}

	protected:
		char data_[Size]; // default initialized
		std::size_t offset_;
	};
}

