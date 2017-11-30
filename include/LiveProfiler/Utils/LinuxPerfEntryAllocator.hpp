#pragma once
#include <memory>
#include "LinuxPerfEntry.hpp"

namespace LiveProfiler {
	/** Class used to allocate and reuse LinuxPerfEntry instances */
	class LinuxPerfEntryAllocator {
	public:
		/** Getters and setters */
		std::size_t getMmapPageCount() const { return mmapPageCount_; }
		void setMmapPageCount(std::size_t count) { mmapPageCount_ = count; }

		/** Allocate LinuxPerfEntry instance */
		std::unique_ptr<LinuxPerfEntry> allocate() {
			std::unique_ptr<LinuxPerfEntry> entry;
			if (freeEntries_.empty()) {
				entry = std::make_unique<LinuxPerfEntry>();
			} else {
				entry = std::move(freeEntries_.back());
				freeEntries_.pop_back();
			}
			entry->reset(mmapPageCount_);
			return entry;
		}

		/** Deallocate LinuxPerfEntry instance */
		void deallocate(std::unique_ptr<LinuxPerfEntry>&& entry) {
			entry->freeResources();
			freeEntries_.emplace_back(std::move(entry)); // no limits for now
		}

		/** Constructor */
		LinuxPerfEntryAllocator(std::size_t mmapPageCount) :
			freeEntries_(),
			mmapPageCount_(mmapPageCount) { }

	protected:
		std::vector<std::unique_ptr<LinuxPerfEntry>> freeEntries_;
		std::size_t mmapPageCount_;
	};
}

