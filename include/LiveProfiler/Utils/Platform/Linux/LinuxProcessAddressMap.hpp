#pragma once
#include <cassert>
#include <string>
#include <memory>
#include "../../Allocators/SingletonAllocator.hpp"
#include "../../StringUtils.hpp"
#include "../../TypeConvertUtils.hpp"
#include "LinuxExecutableSymbolResolver.hpp"

namespace LiveProfiler {
	/** Represent a single line in /proc/$pid/maps */
	class LinuxProcessAddressMap {
	public:
		/** Getters */
		std::uintptr_t getStartAddress() const { return startAddress_; }
		std::uintptr_t getEndAddress() const { return endAddress_; }
		std::uintptr_t getFileOffset() const { return fileOffset_; }
		const std::shared_ptr<std::string>& getPath() const& { return path_; }

		/**
		 * Line format:
		 * address           permissions offset   device inode pathname(optional)
		 * 08048000-08056000 r-xp        00000000 03:0c  64593 /usr/sbin/gpm
		 * Return whether the parse is successful.
		 */
		bool parseLine(
			const std::string& line,
			const std::shared_ptr<SingletonAllocator<std::string, std::string>>& pathAllocator) {
			assert(pathAllocator != nullptr);
			// reset members
			startAddress_ = 0;
			endAddress_ = 0;
			fileOffset_ = 0;
			path_.reset();
			// split line with blank characters
			std::size_t successParts = 0;
			StringUtils::split(line,
				[this, &line, &successParts, &pathAllocator]
				(auto startIndex, auto endIndex, auto count) {
				if (count == 0) {
					// address
					auto middleIndex = line.find_first_of('-', startIndex);
					unsigned long long startAddressL = 0;
					unsigned long long endAddressL = 0;
					if (middleIndex != line.npos && middleIndex + 1 < endIndex &&
						TypeConvertUtils::strToUnsignedLongLong(
							line.c_str() + startIndex, startAddressL, 16) &&
						TypeConvertUtils::strToUnsignedLongLong(
							line.c_str() + middleIndex + 1, endAddressL, 16)) {
						startAddress_ = static_cast<std::uintptr_t>(startAddressL);
						endAddress_ = static_cast<std::uintptr_t>(endAddressL);
						++successParts;
					}
				} else if (count == 2) {
					// offset
					unsigned long long offsetL = 0;
					if (TypeConvertUtils::strToUnsignedLongLong(
						line.c_str() + startIndex, offsetL, 16)) {
						fileOffset_ = static_cast<std::uintptr_t>(offsetL);
						++successParts;
					}
				} else if (count == 5) {
					// pathname
					const char* pathPtr = line.c_str() + startIndex;
					std::size_t pathSize = endIndex - startIndex;
					path_ = pathAllocator->allocate(pathPtr, pathSize);
					++successParts;
				}
			});
			// handle empty pathname
			if (path_ == nullptr && successParts == 2) {
				path_ = pathAllocator->allocate("", 0);
				++successParts;
			}
			// is all address, offset, pathname parse successful?
			return successParts == 3;
		}

		/** Constructor */
		LinuxProcessAddressMap() :
			startAddress_(0),
			endAddress_(0),
			fileOffset_(0),
			path_() { }

	protected:
		std::uintptr_t startAddress_;
		std::uintptr_t endAddress_;
		std::uintptr_t fileOffset_;
		std::shared_ptr<std::string> path_;
	};
}

