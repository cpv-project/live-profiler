#include "./Cases/Collectors/TestCpuSampleLinuxCollector.hpp"
#include "./Cases/Interceptors/TestCpuSampleLinuxSymbolResolveInterceptor.hpp"
#include "./Cases/Profiler/TestProfiler.hpp"
#include "./Cases/Utils/Allocators/TestFreeListAllocator.hpp"
#include "./Cases/Utils/Allocators/TestSingletonAllocator.hpp"
#include "./Cases/Utils/Containers/TestStackBuffer.hpp"
#include "./Cases/Utils/Platform/Linux/TestLinuxEpollDescriptor.hpp"
#include "./Cases/Utils/Platform/Linux/TestLinuxExecutableSymbolResolver.hpp"
#include "./Cases/Utils/Platform/Linux/TestLinuxPerfUtils.hpp"
#include "./Cases/Utils/Platform/Linux/TestLinuxProcessAddressLocator.hpp"
#include "./Cases/Utils/Platform/Linux/TestLinuxProcessAddressMap.hpp"
#include "./Cases/Utils/Platform/Linux/TestLinuxProcessUtils.hpp"
#include "./Cases/Utils/TestStringUtils.hpp"
#include "./Cases/Utils/TestTypeConvertUtils.hpp"

namespace LiveProfilerTests {
	void testAll() {
		testCpuSampleLinuxCollector();
		testCpuSampleLinuxSymbolResolveInterceptor();
		testProfiler();
		testFreeListAllocator();
		testSingletonAllocator();
		testStackBuffer();
		testLinuxEpollDescriptor();
		testLinuxExecutableSymbolResolver();
		testLinuxPerfUtils();
		testLinuxProcessAddressLocator();
		testLinuxProcessAddressMap();
		testLinuxProcessUtils();
		testStringUtils();
		testTypeConvertUtils();
	}
}

int main() {
	LiveProfilerTests::testAll();
	return 0;
}

