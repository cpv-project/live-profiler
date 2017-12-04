#include "./Cases/Profiler/TestProfiler.hpp"
#include "./Cases/Collectors/TestCpuSampleLinuxCollector.hpp"
#include "./Cases/Utils/Allocators/TestFreeListAllocator.hpp"
#include "./Cases/Utils/Containers/TestStackBuffer.hpp"
#include "./Cases/Utils/Platform/Linux/TestLinuxEpollDescriptor.hpp"
#include "./Cases/Utils/Platform/Linux/TestLinuxExecutableSymbolResolver.hpp"
#include "./Cases/Utils/Platform/Linux/TestLinuxPerfUtils.hpp"
#include "./Cases/Utils/Platform/Linux/TestLinuxProcessAddressLocator.hpp"
#include "./Cases/Utils/Platform/Linux/TestLinuxProcessUtils.hpp"
#include "./Cases/Utils/TestTypeConvertUtils.hpp"

namespace LiveProfilerTests {
	void testAll() {
		testProfiler();
		testCpuSampleLinuxCollector();
		testFreeListAllocator();
		testStackBuffer();
		testLinuxEpollDescriptor();
		testLinuxExecutableSymbolResolver();
		testLinuxPerfUtils();
		testLinuxProcessAddressLocator();
		testLinuxProcessUtils();
		testTypeConvertUtils();
	}
}

int main() {
	LiveProfilerTests::testAll();
	return 0;
}

