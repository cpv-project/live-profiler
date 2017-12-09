#include <iostream>
#include <LiveProfiler/Analyzers/CpuSampleDebugAnalyzer.hpp>
#include <LiveProfiler/Profiler/Profiler.hpp>
#include <LiveProfiler/Collectors/CpuSampleLinuxCollector.hpp>
#include <LiveProfiler/Interceptors/CpuSampleLinuxSymbolResolveInterceptor.hpp>

int main(int argc, char** argv) {
	using namespace LiveProfiler;
	if (argc < 3) {
		std::cerr << "Usage: ./a.out ProcessName CollectTimeInMilliseconds" << std::endl;
		return -1;
	}
	auto processName = argv[1];
	auto collectTime = std::stoi(argv[2]);
	
	Profiler<CpuSampleModel> profiler;
	auto collector = profiler.useCollector<CpuSampleLinuxCollector>();
	auto analyzer = profiler.addAnalyzer<CpuSampleDebugAnalyzer>();
	auto interceptor = profiler.addInterceptor<CpuSampleLinuxSymbolResolveInterceptor>();
	collector->filterProcessByName(processName);
	std::cout << "collect for " << processName << " in " << collectTime << " ms" << std::endl;
	profiler.collectFor(std::chrono::milliseconds(collectTime));
	return 0;
}

