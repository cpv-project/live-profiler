#pragma once
#include <sys/types.h>
#include <cassert>
#include <vector>
#include <memory>
#include <chrono>
#include <utility>
#include <fstream>
#include <algorithm>
#include "../../Allocators/SingletonAllocator.hpp"
#include "../../Containers/StackBuffer.hpp"
#include "LinuxProcessAddressMap.hpp"

namespace LiveProfiler {
	/**
	 * Class used to find out address is mapped to which file for specificed linux process
	 * The maps is read from /proc/$pid/maps,
	 * because maps may change continuously it needs reload under certain conditions.
	 * And because I want to avoid frequent reload so locate a newly mapped address may fail,
	 * unless `forceReload` option is used.
	 */
	class LinuxProcessAddressLocator {
	public:
		/** Default parameters */
		static const std::size_t DefaultMapsUpdateMinInterval = 1;

		/**
		 * Locate file path and offset for the specified address.
		 * Return (nullptr, 0) if locate failed.
		 * When `forceReload` option is true, maps will be force to reload after first locate is failed,
		 * it can ensure no newly mapped address is missed but may reduce performance.
		 */
		std::pair<std::shared_ptr<std::string>, std::ptrdiff_t> locate(
			std::uintptr_t address, bool forceReload) {
			for (std::size_t i = 0; i < 2; ++i) {
				auto it = locateMap(address);
				if (it != maps_.cend()) {
					auto offset = address - it->getStartAddress() + it->getFileOffset();
					return { it->getPath(), offset };
				}
				if (i == 0) {
					// first locate failed
					auto now = std::chrono::high_resolution_clock::now();
					if (forceReload || now - mapsUpdated_ > mapsUpdateMinInterval_) {
						reload();
						mapsUpdated_ = now;
					}
				}
			}
			return { nullptr, 0 };
		}

		/** Reload maps from /proc/$pid/maps */
		void reload() {
			// build path
			static const std::string prefix("/proc/");
			static const std::string suffix("/maps");
			if (mapsPathBuffer_.empty()) {
				mapsPathBuffer_.appendStr(prefix.data(), prefix.size());
				mapsPathBuffer_.appendLongLong(pid_);
				mapsPathBuffer_.appendStr(suffix.data(), suffix.size());
				mapsPathBuffer_.appendNullTerminator();
			}
			// parse lines
			std::ifstream file(mapsPathBuffer_.data());
			LinuxProcessAddressMap map;
			maps_.clear();
			while (std::getline(file, line_)) {
				if (map.parseLine(line_, pathAllocator_)) {
					maps_.emplace_back(std::move(map));
				}
			}
			// sort maps by start address
			std::sort(maps_.begin(), maps_.end(),
				[](const auto& a, const auto& b) {
					return a.getStartAddress() < b.getStartAddress();
				});
		}

		/** For FreeListAllocator */
		void freeResources() {
			pathAllocator_ = nullptr;
			maps_.clear();
		}

		/** For FreeListAllocator */
		void reset(
			pid_t pid,
			const std::shared_ptr<SingletonAllocator<std::string, std::string>>& pathAllocator) {
			assert(pathAllocator != nullptr);
			pid_ = pid;
			pathAllocator_ = pathAllocator;
			maps_.clear();
			mapsUpdated_ = {};
			mapsPathBuffer_.clear();
			line_.clear();
		}

		/** Constructor */
		LinuxProcessAddressLocator() :
			pid_(0),
			pathAllocator_(nullptr),
			maps_(),
			mapsUpdated_(),
			mapsUpdateMinInterval_(std::chrono::milliseconds(DefaultMapsUpdateMinInterval)),
			mapsPathBuffer_(),
			line_() { }

	protected:
		/** Find map iterator for specified address, return maps_.cend() if not found */
		std::vector<LinuxProcessAddressMap>::const_iterator locateMap(std::uintptr_t address) const {
			// find first map that startAddress > address
			auto it = std::upper_bound(
				maps_.cbegin(), maps_.cend(), address,
				[](const auto& a, const auto& b) {
					return a < b.getStartAddress();
				});
			// get the previous map
			if (it == maps_.cbegin()) {
				return maps_.cend();
			}
			--it;
			// check is address >= startAddress and address < endAddress
			return address < it->getEndAddress() ? it : maps_.cend();
		}

	protected:
		pid_t pid_;
		std::shared_ptr<SingletonAllocator<std::string, std::string>> pathAllocator_;
		std::vector<LinuxProcessAddressMap> maps_;
		std::chrono::high_resolution_clock::time_point mapsUpdated_;
		std::chrono::high_resolution_clock::duration mapsUpdateMinInterval_;
		StackBuffer<128> mapsPathBuffer_;
		std::string line_;
	};
}

