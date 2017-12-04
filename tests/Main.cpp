#include "./Cases/Profiler/TestProfiler.hpp"
#include "./Cases/Collectors/TestCpuSampleLinuxCollector.hpp"
#include "./Cases/Utils/Allocators/TestFreeListAllocator.hpp"
#include "./Cases/Utils/Containers/TestStackBuffer.hpp"
#include "./Cases/Utils/TestLinuxEpollDescriptor.hpp"
#include "./Cases/Utils/TestLinuxPerfUtils.hpp"
#include "./Cases/Utils/TestTypeConvertUtils.hpp"
#include "./Cases/Utils/TestLinuxProcessUtils.hpp"

namespace LiveProfilerTests {
	void testAll() {
		testProfiler();
		testCpuSampleLinuxCollector();
		testFreeListAllocator();
		testStackBuffer();
		testLinuxEpollDescriptor();
		testLinuxPerfUtils();
		testLinuxProcessUtils();
		testTypeConvertUtils();
	}
}

int main() {
	LiveProfilerTests::testAll();
	return 0;
}

