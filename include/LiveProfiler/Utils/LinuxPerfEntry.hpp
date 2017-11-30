#pragma once
#include <unistd.h>
#include <linux/perf_event.h>
#include <sys/mman.h>
#include <atomic>
#include <vector>

namespace LiveProfiler {
	/** Class contains perf_events releated data */
	class LinuxPerfEntry {
	public:
		::perf_event_attr& getAttrRef() & {
			return attr_;
		}
		
		/** Close file descriptor and unmap memory */
		void freeResources() {
			if (fd_ != 0) {
				::close(fd_);
				::munmap(mmapBuffer_.data(), mmapBuffer_.size());
				fd_ = 0;
			}
		}

		/** Reset to initial state */
		void reset(std::size_t mmapBufferPages, std::size_t pageSize) {
			freeResources();
			attr_ = {};
			pid_ = 0;
			fd_ = 0;
			lastReadOffset_ = pageSize;
			pageSize_ = pageSize;
			mmapBuffer_.resize(mmapBufferPages * pageSize);
			std::fill(mmapBuffer_.begin(), mmapBuffer_.end(), 0);
		}

		/** Constructor */
		LinuxPerfEntry() :
			attr_(),
			pid_(0),
			fd_(0),
			lastReadOffset_(0),
			pageSize_(0),
			mmapBuffer_() { }

		/** Destructor */
		~LinuxPerfEntry() {
			freeResources();
		}

	protected:
		::perf_event_attr attr_;
		std::atomic_int pid_;
		std::atomic_int fd_;
		std::atomic_size_t lastReadOffset_;
		std::size_t pageSize_;
		std::vector<char> mmapBuffer_;
	};
}

