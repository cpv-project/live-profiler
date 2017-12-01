#include "./Cases/Profiler/TestProfiler.hpp"
#include "./Cases/Collectors/TestCpuSampleLinuxCollector.hpp"
#include "./Cases/Utils/TestLinuxProcessUtils.hpp"
#include "./Cases/Utils/TestLinuxPerfEntryAllocator.hpp"
#include "./Cases/Utils/TestLinuxPerfUtils.hpp"
#include "./Cases/Utils/TestStackBuffer.hpp"
#include "./Cases/Utils/TestTypeConvertUtils.hpp"

namespace LiveProfilerTests {
	void testAll() {
		testProfiler();
		testCpuSampleLinuxCollector();
		testLinuxProcessUtils();
		testLinuxPerfEntryAllocator();
		testLinuxPerfUtils();
		testStackBuffer();
		testTypeConvertUtils();
	}
}

int main() {
	LiveProfilerTests::testAll();
	return 0;
}

