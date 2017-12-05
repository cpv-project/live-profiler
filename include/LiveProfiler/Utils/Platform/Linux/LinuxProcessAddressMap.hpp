#pragma once
#include <string>
#include <memory>
#include "../../Allocators/SingletonAllocator.hpp"
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
		const std::shared_ptr<LinuxExecutableSymbolResolver>& getResolver() const& { return resolver_; }

		/**
		 * Line format:
		 * address           permissions offset   device inode pathname
		 * 08048000-08056000 r-xp        00000000 03:0c  64593 /usr/sbin/gpm
		 * Return whether the parse is successful.
		 */
		bool parseLine(
			const std::string& line,
			const std::shared_ptr<SingletonAllocator<std::string, std::string>>& pathAllocator,
			const std::shared_ptr<SingletonAllocator<
				std::string, LinuxExecutableSymbolResolver>>& resolverAllocator) {
			std::size_t startIndex = 0;
			std::size_t endIndex = 0;
			std::size_t partIndex = 0;
			// reset members
			startAddress_ = 0;
			endAddress_ = 0;
			fileOffset_ = 0;
			path_.reset();
			resolver_.reset();
			// split line with blank characters
			static const char blankChars[] = " \t\n";
			std::size_t successParts = 0;
			while (startIndex < line.size()) {
				auto index = line.find_first_of(blankChars, startIndex);
				endIndex = (index == line.npos) ? line.size() : index;
				if (partIndex == 0) {
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
				} else if (partIndex == 2) {
					// offset
					unsigned long long offsetL = 0;
					if (TypeConvertUtils::strToUnsignedLongLong(
						line.c_str() + startIndex, offsetL, 16)) {
						fileOffset_ = static_cast<std::uintptr_t>(offsetL);
						++successParts;
					}
				} else if (partIndex == 5) {
					// pathname
					const char* pathPtr = line.c_str() + startIndex;
					std::size_t pathSize = endIndex - startIndex;
					path_ = pathAllocator->allocate(pathPtr, pathSize);
					resolver_ = resolverAllocator->allocate(*path_);
					++successParts;
				}
				index = line.find_first_not_of(blankChars, endIndex);
				startIndex = (index == line.npos) ? line.size() : index;
				++partIndex;
			}
			// handle empty pathname
			// TODO

			// is all address, offset, pathname parse successful?
			return successParts == 3;
		}

		/** Constructor */
		LinuxProcessAddressMap() :
			startAddress_(0),
			endAddress_(0),
			fileOffset_(0),
			path_(),
			resolver_() { }

	protected:
		std::uintptr_t startAddress_;
		std::uintptr_t endAddress_;
		std::uintptr_t fileOffset_;
		std::shared_ptr<std::string> path_;
		std::shared_ptr<LinuxExecutableSymbolResolver> resolver_;
	};
}

