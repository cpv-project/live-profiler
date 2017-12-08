#pragma once
#include <unistd.h>
#include <linux/perf_event.h>
#include <sys/mman.h>
#include <vector>
#include <cassert>

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

		/**
		 * Set the address from mmap on fd.
		 * The address should be mapped with PROT_READ | PROT_WRITE.
		 * Previously I was use PROT_READ only but realize that's wrong,
		 * because the kernel can rewrite the data while it's be reading,
		 * if the data have more than one field then it will lost integrity.
		 */
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

		/** Get metadata struct from mapped memory */
		const ::perf_event_mmap_page* getMetaPage() const {
			assert(mmapStartAddress_ != nullptr);
			return reinterpret_cast<::perf_event_mmap_page*>(mmapStartAddress_);
		}

		/**
		 * Fetch records from mapped memory based on latest read offset.
		 * `maxRecords` should be less or equal to `attr.wakeup_events`.
		 * Please call `updateReadOffset` **AFTER** handle the records.
		 */
		const std::vector<::perf_event_header*>& getRecords(std::size_t maxRecords) & {
			assert(mmapDataAddress_ != nullptr);
			records_.clear();
			auto readOffset = mmapReadOffset_;
			for (std::size_t i = 0; i < maxRecords; ++i) {
				// please be careful about the calculation here
				// there may not be enough size between [readOffset, mmapDataSize_)
				if (readOffset + sizeof(::perf_event_header) > mmapDataSize_) {
					break; // not enough size for header
				}
				auto* header = reinterpret_cast<::perf_event_header*>(mmapDataAddress_ + readOffset);
				auto nextReadOffset = readOffset + sizeof(::perf_event_header) + header->size;
				if (nextReadOffset > mmapDataSize_) {
					break; // not enough size for this record
				}
				// add record
				records_.emplace_back(header);
				readOffset = nextReadOffset;
			}
			return records_;
		}

		/* Update read offset prepare for next round */
		void updateReadOffset() {
			// tell kernel data until data_head has been read
			// --------- simulation --------
			// initial state:
			// [ head | tail | lastHead, writable, writable, writable, ... ]
			// kernel wrote some data:
			// [ tail | lastHead, non-writable, non-writable, head, writable, ... ]
			// after getData and updateReadOffset:
			// [ writable, writable, tail | lastHead | head, writable, ... ]
			// kernal wrote some data:
			// [ writable, writable, tail | lastHead, non-writable, head, writable, ... ]
			// and so on...
			auto* metaPage = reinterpret_cast<::perf_event_mmap_page*>(mmapStartAddress_);
			auto lastHead = metaPage->data_head;
			metaPage->data_tail = lastHead;
			mmapReadOffset_ = lastHead % mmapDataSize_;
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
		std::uint64_t mmapReadOffset_;
		std::vector<::perf_event_header*> records_;
	};
}

