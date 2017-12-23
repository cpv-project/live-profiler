#if defined(__linux__)
#include <unistd.h>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <LiveProfiler/Utils/Platform/Linux/LinuxEpollDescriptor.hpp>
#include <LiveProfiler/Utils/Platform/Linux/LinuxPerfEntry.hpp>
#include <LiveProfiler/Utils/Platform/Linux/LinuxPerfUtils.hpp>

namespace LiveProfilerTests {
	using namespace LiveProfiler;

	namespace {
		class BusyThread {
		public:
			pid_t start() {
				flag_ = true;
				tid_ = 0;
				thread_ = std::thread([this] {
					tid_ = ::syscall(__NR_gettid);
					cv_.notify_one();
					while (flag_) { }
				});
				std::unique_lock<std::mutex> lock(mutex_);
				cv_.wait(lock, [this] { return tid_ != 0; });
				return tid_;
			}

			BusyThread() :
				mutex_(), cv_(), flag_(false), tid_(0), thread_() { }

			~BusyThread() {
				flag_ = false;
				thread_.join();
			}

		protected:
			std::mutex mutex_;
			std::condition_variable cv_;
			std::atomic_bool flag_;
			std::atomic<pid_t> tid_;
			std::thread thread_;
		};

		struct CpuSampleRawData {
			::perf_event_header header;
			std::uint64_t ip;
			std::uint32_t pid;
			std::uint32_t tid;
		};
	}

	void testLinuxPerfUtilsPerfEventOpen() {
		perf_event_attr attr = {};
		attr.type = PERF_TYPE_SOFTWARE;
		attr.size = sizeof(perf_event_attr);
		attr.config = PERF_COUNT_SW_CPU_CLOCK;
		attr.sample_period = 100000;
		attr.sample_type = PERF_SAMPLE_TID | PERF_SAMPLE_IP;
		attr.disabled = 1;
		attr.inherit = 0;
		attr.wakeup_events = 1;
		attr.exclude_user = 0;
		attr.exclude_kernel = 1;
		attr.exclude_hv = 1;
		auto fd = LinuxPerfUtils::perfEventOpen(&attr, ::getpid(), -1, -1, 0);
		assert(fd > 0);
		::close(fd);
	}

	void testLinuxPerfUtilsPerfEventEnable() {
		BusyThread busyThread;
		LinuxEpollDescriptor epoll;
		auto entry = std::make_unique<LinuxPerfEntry>();
		auto tid = busyThread.start();
		entry->setPid(tid);
		auto ret = LinuxPerfUtils::monitorSample(
			entry,
			PERF_TYPE_SOFTWARE,
			PERF_COUNT_SW_CPU_CLOCK,
			100000,
			PERF_SAMPLE_IP | PERF_SAMPLE_TID,
			16,
			1,
			false,
			true,
			true);
		assert(ret);
		LinuxPerfUtils::perfEventEnable(entry->getFd(), true);
		epoll.add(entry->getFd(), EPOLLIN, tid);
		for (std::size_t i = 0; i < 5; ++i) {
			auto& events = epoll.wait(std::chrono::milliseconds(1000));
			auto inCount = std::count_if(events.cbegin(), events.cend(),
				[](auto& e) { return (e.events & EPOLLIN) == EPOLLIN; });
			assert(inCount == 1);
		}
	}

	void testLinuxPerfUtilsPerfEventDisable() {
		BusyThread busyThread;
		LinuxEpollDescriptor epoll;
		auto entry = std::make_unique<LinuxPerfEntry>();
		auto tid = busyThread.start();
		entry->setPid(tid);
		auto ret = LinuxPerfUtils::monitorSample(
			entry,
			PERF_TYPE_SOFTWARE,
			PERF_COUNT_SW_CPU_CLOCK,
			100000,
			PERF_SAMPLE_IP | PERF_SAMPLE_TID,
			16,
			1,
			false,
			true,
			true);
		assert(ret);
		LinuxPerfUtils::perfEventEnable(entry->getFd(), true);
		LinuxPerfUtils::perfEventDisable(entry->getFd());
		epoll.add(entry->getFd(), EPOLLIN, tid);
		for (std::size_t i = 0; i < 5; ++i) {
			auto& events = epoll.wait(std::chrono::milliseconds(5));
			assert(i == 0 || events.size() == 0); // first round may have events
		}
	}

	void testLinuxPerfUtilsMonitorSample() {
		BusyThread busyThread;
		LinuxEpollDescriptor epoll;
		auto entry = std::make_unique<LinuxPerfEntry>();
		auto tid = busyThread.start();
		entry->setPid(tid);
		auto ret = LinuxPerfUtils::monitorSample(
			entry,
			PERF_TYPE_SOFTWARE,
			PERF_COUNT_SW_CPU_CLOCK,
			100000,
			PERF_SAMPLE_IP | PERF_SAMPLE_TID,
			16,
			2,
			false,
			true,
			true);
		assert(ret);
		LinuxPerfUtils::perfEventEnable(entry->getFd(), true);
		epoll.add(entry->getFd(), EPOLLIN, tid);
		for (std::size_t i = 0; i < 5; ++i) {
			auto& events = epoll.wait(std::chrono::milliseconds(1000));
			auto inCount = std::count_if(events.cbegin(), events.cend(),
				[](auto& e) { return (e.events & EPOLLIN) == EPOLLIN; });
			assert(inCount == 1);
			auto& records = entry->getRecords();
			for (auto* record : records) {
				if (record->type == PERF_RECORD_SAMPLE) {
					auto* data = reinterpret_cast<const CpuSampleRawData*>(record);
					assert(data->ip != 0);
					assert(data->pid == static_cast<std::uint32_t>(::getpid()));
					assert(data->tid == static_cast<std::uint32_t>(tid));
				}
			}
			entry->updateReadOffset();
		}
	}

	void testLinuxPerfUtils() {
		std::cout << __func__ << std::endl;
		testLinuxPerfUtilsPerfEventOpen();
		testLinuxPerfUtilsPerfEventEnable();
		testLinuxPerfUtilsPerfEventDisable();
		testLinuxPerfUtilsMonitorSample();
	}
}
#else // defined(__linux__)
namespace LiveProfilerTests {
	void testLinuxPerfUtils() {
		// unsupported on other platform
	}
}
#endif // defined(__linux__)
