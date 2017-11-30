#include <cassert>
#include <iostream>
#include <algorithm>
#include <LiveProfiler/Utils/LinuxProcessUtils.hpp>

namespace LiveProfilerTests {
	using namespace LiveProfiler;

	void testLinuxProcessUtilsListProcesses() {
		auto selfpid = ::getpid();
		std::vector<pid_t> pids;
		LinuxProcessUtils::listProcesses(pids, [](pid_t pid) { return true; });
		assert(std::count(pids.cbegin(), pids.cend(), selfpid) > 0);
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

