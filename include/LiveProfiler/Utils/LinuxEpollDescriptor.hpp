#pragma once
#include <unistd.h>
#include <sys/epoll.h>
#include "../Exceptions/ProfilerException.hpp"

namespace LiveProfiler {
	/** Class used to operate epoll instance */
	class LinuxEpollDescriptor {
	public:
		/** Getter */
		int getEpollFd() const { return epollFd_; }

		/** Constructor */
		LinuxEpollDescriptor() : LinuxEpollDescriptor(::epoll_create(1)) { }

		/** Constructor */
		LinuxEpollDescriptor(int epollFd) : epollFd_(epollFd) { }

		/** Destructor */
		~LinuxEpollDescriptor() { ::close(epollFd_); }

		/** Register the target file descriptor on the epoll instance */
		void add(int fd, uint32_t events, std::uint64_t data) {
			::epoll_event ev;
			ev.events = events;
			ev.data.u64 = data;
			auto ret = ::epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &ev);
			if (ret != 0) {
				throw ProfilerException(errno, "[LinuxEpollDescriptor::add] epoll_ctl");
			}
		}

		/** Change the event associated with the target file descriptor */
		void mod(int fd, uint32_t events, std::uint64_t data) {
			::epoll_event ev;
			ev.events = events;
			ev.data.u64 = data;
			auto ret = ::epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &ev);
			if (ret != 0) {
				throw ProfilerException(errno, "[LinuxEpollDescriptor::mod] epoll_ctl");
			}
		}

		/** Deregister the target file descriptor from the epoll instance */
		void del(int fd) {
			::epoll_event ev ({ }); // see BUGS in man page
			auto ret = ::epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, &ev);
			if (ret != 0) {
				int err = errno;
				if (err == ENOENT) {
					// don't throw exception for duplicated delete
					return;
				}
				throw ProfilerException(err, "[LinuxEpollDescriptor::del] epoll_ctl");
			}
		}

	protected:
		/** Disable copy */
		LinuxEpollDescriptor(const LinuxEpollDescriptor&) = delete;
		LinuxEpollDescriptor& operator=(const LinuxEpollDescriptor&) = delete;

	protected:
		int epollFd_;
	};
}

