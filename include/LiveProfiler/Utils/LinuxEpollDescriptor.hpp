#pragma once
#include <unistd.h>
#include <sys/epoll.h>
#include <chrono>
#include <vector>
#include "../Exceptions/ProfilerException.hpp"

namespace LiveProfiler {
	/** Class used to operate epoll instance */
	class LinuxEpollDescriptor {
	public:
		/** Default parameters */
		static const std::size_t DefaultMaxEpollEvents = 128;

		/** Getter */
		int getEpollFd() const { return epollFd_; }

		/** Constructor */
		LinuxEpollDescriptor() :
			LinuxEpollDescriptor(::epoll_create(1), DefaultMaxEpollEvents) { }

		/** Constructor */
		LinuxEpollDescriptor(int epollFd, std::size_t maxEpollEvents) :
			epollFd_(epollFd),
			maxEpollEvents_(maxEpollEvents),
			events_() { }

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

		/** Wait for an I/O event on an epoll file descriptor */
		template <class Rep, class Period>
		const std::vector<::epoll_event>& wait(
			std::chrono::duration<Rep, Period> timeout) & {
			int timeoutVal = std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count();
			if (timeout.count() > 0 && timeoutVal == 0) {
				timeoutVal = 1; // timeout < 1ms, fix to 1ms
			}
			events_.resize(maxEpollEvents_);
			auto ret = ::epoll_wait(epollFd_, events_.data(), events_.size(), timeoutVal);
			if (ret < 0) {
				int err = errno;
				if (err == EINTR) {
					// interrupted by a signal handle is not a error
					ret = 0;
				} else {
					throw ProfilerException(err, "[LinuxEpollDescriptor::wait] epoll_wait");
				}
			}
			events_.resize(ret);
			return events_;
		}

	protected:
		/** Disable copy */
		LinuxEpollDescriptor(const LinuxEpollDescriptor&) = delete;
		LinuxEpollDescriptor& operator=(const LinuxEpollDescriptor&) = delete;

	protected:
		int epollFd_;
		std::size_t maxEpollEvents_;
		std::vector<::epoll_event> events_;
	};
}

