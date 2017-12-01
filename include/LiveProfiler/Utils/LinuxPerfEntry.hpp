#pragma once
#include <unistd.h>
#include <linux/perf_event.h>
#include <sys/mman.h>
#include <vector>

namespace LiveProfiler {
	/**
	 * Class contains perf_events releated data.
	 * MMAP layout:
	 * - first page, type is perf_event_mmap_page*
	 * - ring buffer, element size is indeterminate
	 * mmapDataAddress = mmapStartAddress + pageSize
	 * mmapDataSize = mmapTotalSize - pageSize
	 */
	class LinuxPerfEntry {
	public:
		/** Getters and setters */
		::perf_event_attr& getAttrRef() & { return attr_; }
		pid_t getPid() const { return pid_; }
		void setPid(pid_t pid) { pid_ = pid; }
		int getFd() const { return fd_; }
		void setFd(int fd) { fd_ = fd; }

		/** Unmap mmap address and close file descriptor */
		void freeResources() {
			if (mmapStartAddress_ != nullptr) {
				::munmap(mmapStartAddress_, mmapTotalSize_);
				mmapStartAddress_ = nullptr;
				mmapDataAddress_ = nullptr;
			}
			if (fd_ != 0) {
				::close(fd_);
				fd_ = 0;
			}
		}

		/** Reset to initial state */
		void reset() {
			freeResources();
			attr_ = {};
			pid_ = 0;
			fd_ = 0;
			mmapStartAddress_ = nullptr;
			mmapDataAddress_ = nullptr;
			mmapTotalSize_ = 0;
			mmapDataSize_ = 0;
			mmapReadOffset_ = 0;
		}

		/** Set the address from mmap on fd */
		void setMmapAddress(
			char* mmapStartAddress,
			std::size_t mmapTotalSize,
			std::size_t pageSize) {
			mmapStartAddress_ = mmapStartAddress;
			mmapDataAddress_ = mmapStartAddress + pageSize;
			mmapTotalSize_ = mmapTotalSize;
			mmapDataSize_ = mmapTotalSize - pageSize;
			mmapReadOffset_ = 0;
		}

		/** Constructor */
		LinuxPerfEntry() :
			attr_(),
			pid_(0),
			fd_(0),
			mmapStartAddress_(nullptr),
			mmapDataAddress_(nullptr),
			mmapTotalSize_(0),
			mmapDataSize_(0),
			mmapReadOffset_(0) { }

		/** Destructor */
		~LinuxPerfEntry() {
			freeResources();
		}

	protected:
		::perf_event_attr attr_;
		pid_t pid_;
		int fd_;
		char* mmapStartAddress_;
		char* mmapDataAddress_;
		std::size_t mmapTotalSize_;
		std::size_t mmapDataSize_;
		std::size_t mmapReadOffset_;
	};
}

