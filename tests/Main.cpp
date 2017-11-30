#include "./Cases/Profiler/TestProfiler.hpp"
#include "./Cases/Collectors/TestCpuSampleLinuxCollector.hpp"
#include "./Cases/Utils/TestLinuxProcessUtils.hpp"
#include "./Cases/Utils/TestLinuxPerfEntryAllocator.hpp"
#include "./Cases/Utils/TestLinuxPerfUtils.hpp"

namespace LiveProfilerTests {
	void testAll() {
		testProfiler();
		testCpuSampleLinuxCollector();
		testLinuxProcessUtils();
		testLinuxPerfEntryAllocator();
		testLinuxPerfUtils();
	}
}

int main() {
	LiveProfilerTests::testAll();
	return 0;
}

