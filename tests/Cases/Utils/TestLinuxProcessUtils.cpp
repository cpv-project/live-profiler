#include <syscall.h>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <thread>
#include <LiveProfiler/Utils/LinuxProcessUtils.hpp>

namespace LiveProfilerTests {
	using namespace LiveProfiler;

	void testLinuxProcessUtilsListProcesses() {
		pid_t selfpid = ::getpid();
		std::vector<pid_t> pids;
		LinuxProcessUtils::listProcesses(pids, [](pid_t) { return true; }, false);
		assert(std::count(pids.cbegin(), pids.cend(), selfpid) > 0);
	}

	void testLinuxProcessUtilsListThreads() {
		std::thread t([] {
			pid_t selfpid = ::getpid();
			pid_t selftid = ::syscall(__NR_gettid);
			std::vector<pid_t> tids;
			LinuxProcessUtils::listProcesses(
				tids, [selfpid](pid_t pid) { return pid == selfpid; }, true);
			assert(std::count(tids.cbegin(), tids.cend(), selfpid) > 0);
			assert(std::count(tids.cbegin(), tids.cend(), selftid) > 0);
			tids.clear();
			LinuxProcessUtils::listThreads(tids, selfpid);
			assert(std::count(tids.cbegin(), tids.cend(), selfpid) > 0);
			assert(std::count(tids.cbegin(), tids.cend(), selftid) > 0);
		});
		t.join();
	}

	void testLinuxProcessUtilsGetProcessFilterByName() {
		auto selfpid = ::getpid();
		auto filter = LinuxProcessUtils::getProcessFilterByName("LiveProfilerTest");
		assert(filter(selfpid));
		assert(!filter(0));
		assert(!filter(1));
	}

	void testLinuxProcessUtils() {
		std::cout << __func__ << std::endl;
		testLinuxProcessUtilsListProcesses();
		testLinuxProcessUtilsGetProcessFilterByName();
	}
}

