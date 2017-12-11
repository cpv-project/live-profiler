#include <iostream>
#include <iomanip>
#include <LiveProfiler/Analyzers/CpuSampleFrequencyAnalyzer.hpp>
#include <LiveProfiler/Profiler/Profiler.hpp>
#include <LiveProfiler/Collectors/CpuSampleLinuxCollector.hpp>
#include <LiveProfiler/Interceptors/CpuSampleLinuxSymbolResolveInterceptor.hpp>

namespace {
	using namespace LiveProfiler;

	void printTopSymbolNames(
		const std::vector<CpuSampleFrequencyAnalyzer::SymbolNameAndCountType>& symbolNameAndCounts,
		std::size_t totalCount) {
		std::cout << "No. Overhead Samples Symbol Name" << std::endl;
		for (std::size_t i = 0; i < symbolNameAndCounts.size(); ++i) {
			auto& symbolNameAndCount = symbolNameAndCounts[i];
			std::cout << std::setw(3) << i+1 << " " <<
				std::setw(8) << std::fixed << std::setprecision(2) <<
				static_cast<double>(symbolNameAndCount.second) / totalCount << " " <<
				std::setw(7) << symbolNameAndCount.second << " " <<
				symbolNameAndCount.first->getName() << std::endl;
		}
	}
}

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
	auto analyzer = profiler.addAnalyzer<CpuSampleFrequencyAnalyzer>();
	auto interceptor = profiler.addInterceptor<CpuSampleLinuxSymbolResolveInterceptor>();
	collector->filterProcessByName(processName);
	std::cout << "collect for " << processName << " in " << collectTime << " ms" << std::endl;
	profiler.collectFor(std::chrono::milliseconds(collectTime));
	
	static std::size_t topInclusive = 100;
	static std::size_t topExclusive = 100;
	auto result = analyzer->getResult(topInclusive, topExclusive);
	auto& topInclusiveSymbolNames = result.getTopInclusiveSymbolNames();
	auto& topExclusiveSymbolNames = result.getTopExclusiveSymbolNames();
	std::cout << "top " << topInclusiveSymbolNames.size() << " inclusive symbol names:" << std::endl;
	printTopSymbolNames(topInclusiveSymbolNames, result.getTotalInclusiveCount());
	std::cout << std::endl;
	std::cout << "top " << topExclusiveSymbolNames.size() << " exclusive symbol names:" << std::endl;
	printTopSymbolNames(topExclusiveSymbolNames, result.getTotalExclusiveCount());
	return 0;
}

