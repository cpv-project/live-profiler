#pragma once
#include <unistd.h>
#include <linux/perf_event.h>
#include <atomic>
#include <vector>
#include <memory>

namespace LiveProfiler {
	/** Class contains perf_events releated data */
	struct LinuxPerfEntry {
		::perf_event_attr attr;
		std::atomic_int pid;
		std::atomic_int fd;
		std::vector<char> mmapBuffer;
	};

	/** Class used to allocate and initialize LinuxPerfEntry */
	class LinuxPerfEntryAllocator {
	public:
		/** Allocate LinuxPerfEntry instance */
		std::unique_ptr<LinuxPerfEntry> allocate() {
			std::unique_ptr<LinuxPerfEntry> entry;
			if (freeEntries_.empty()) {
				entry = std::make_unique<LinuxPerfEntry>();
			} else {
				entry = std::move(freeEntries_.back());
				freeEntries_.pop_back();
			}
			entry->attr = {};
			entry->pid = 0;
			entry->fd = 0;
			entry->mmapBuffer.resize(mmapBufferPages_ * pageSize_);
			std::fill(entry->mmapBuffer.begin(), entry->mmapBuffer.end(), 0);
			return entry;
		}

		/** Deallocate LinuxPerfEntry instance */
		void deallocate(std::unique_ptr<LinuxPerfEntry>&& entry) {
			freeEntries_.emplace_back(std::move(entry)); // no limits for now
		}

		/** Constructor */
		LinuxPerfEntryAllocator(std::size_t mmapBufferPages) :
			freeEntries_(),
			pageSize_(::getpagesize()),
			mmapBufferPages_(mmapBufferPages) { }

	protected:
		std::vector<std::unique_ptr<LinuxPerfEntry>> freeEntries_;
		std::size_t pageSize_;
		std::size_t mmapBufferPages_;
	};

	/** Static utility functions releated to linux perf_events */
	struct LinuxPerfUtils {

	};
}

