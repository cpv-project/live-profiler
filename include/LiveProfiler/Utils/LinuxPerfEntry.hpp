#pragma once
#include <unistd.h>
#include <linux/perf_event.h>
#include <sys/mman.h>
#include <vector>

namespace LiveProfiler {
	/** Class contains perf_events releated data */
	class LinuxPerfEntry {
	public:
		/** Getters and setters */
		::perf_event_attr& getAttrRef() & { return attr_; }
		pid_t getPid() const { return pid_; }
		void setPid(pid_t pid) { pid_ = pid; }
		int getFd() const { return fd_; }
		void setFd(int fd) { fd_ = fd; }
		std::size_t getMmapPageSize() const { return mmapPageSize_; }
		std::size_t getMmapPageCount() const { return mmapPageCount_; }

		/** Unmap mmap address and close file descriptor */
		void freeResources() {
			if (mmapAddress_ != nullptr) {
				::munmap(mmapAddress_, mmapPageSize_ * mmapPageCount_);
				mmapAddress_ = nullptr;
			}
			if (fd_ != 0) {
				::close(fd_);
				fd_ = 0;
			}
		}

		/** Reset to initial state */
		void reset(std::size_t mmapPageCount) {
			freeResources();
			attr_ = {};
			pid_ = 0;
			fd_ = 0;
			mmapPageCount_ = mmapPageCount;
			mmapReadIndex_ = 0;
			mmapAddress_ = nullptr;
		}

		/** Constructor */
		LinuxPerfEntry() :
			attr_(),
			pid_(0),
			fd_(0),
			mmapPageSize_(::getpagesize()),
			mmapPageCount_(0),
			mmapReadIndex_(0),
			mmapAddress_(nullptr) { }

		/** Destructor */
		~LinuxPerfEntry() {
			freeResources();
		}

	protected:
		::perf_event_attr attr_;
		pid_t pid_;
		int fd_;
		std::size_t mmapPageSize_;
		std::size_t mmapPageCount_;
		std::size_t mmapReadIndex_;
		char* mmapAddress_;
	};
}

