#include <iostream>
#include <cassert>
#include <memory>
#include <LiveProfiler/Utils/Platform/Linux/LinuxEpollDescriptor.hpp>

namespace LiveProfilerTests {
	using namespace LiveProfiler;

	void testLinuxEpollDescriptor() {
		std::cout << __func__ << std::endl;
		using FilePtrType = std::unique_ptr<::FILE, int(*)(::FILE*)>;

		std::vector<FilePtrType> files;
		for (std::size_t i = 0; i < 3; ++i) {
			files.emplace_back(FilePtrType(::fopen("/proc/version", "rb"), ::fclose));
		}

		LinuxEpollDescriptor epoll;
		for (std::size_t i = 0; i < files.size(); ++i) {
			epoll.add(::fileno(files.at(i).get()), EPOLLIN, i);
		}

		{
			auto& events = epoll.wait(std::chrono::milliseconds(1));
			assert(events.size() == 3);
			for (auto& event : events) {
				auto index = event.data.u32;
				assert((event.events & EPOLLIN) == EPOLLIN);
				epoll.mod(::fileno(files.at(index).get()), 0, index);
			}
		}

		{
			auto& events = epoll.wait(std::chrono::milliseconds(1));
			assert(events.size() == 0);
			for (std::size_t i = 0; i < files.size(); ++i) {
				auto fd = ::fileno(files.at(i).get());
				epoll.mod(fd, EPOLLIN, i);
				epoll.del(fd);
			}
		}

		{
			auto& events = epoll.wait(std::chrono::milliseconds(1));
			assert(events.size() == 0);
		}
	}
}

