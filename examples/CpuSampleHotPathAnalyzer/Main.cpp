#include <iostream>
#include <iomanip>
#include <LiveProfiler/Analyzers/CpuSampleHotPathAnalyzer.hpp>
#include <LiveProfiler/Profiler/Profiler.hpp>
#include <LiveProfiler/Collectors/CpuSampleLinuxCollector.hpp>
#include <LiveProfiler/Interceptors/CpuSampleLinuxSymbolResolveInterceptor.hpp>

namespace {
	using namespace LiveProfiler;

	void printNode(
		const std::unique_ptr<CpuSampleHotPathAnalyzer::NodeType>& node,
		std::size_t totalSampleCount,
		std::size_t level) {
		// sort childs by count descending
		auto& childs = node->getChilds();
		std::vector<std::reference_wrapper<
			const std::decay_t<decltype(childs)>::value_type>>
			sortedChilds(childs.cbegin(), childs.cend());
		std::sort(sortedChilds.begin(), sortedChilds.end(), [](auto& a, auto& b) {
			return a.get().second->getCount() > b.get().second->getCount();
		});
		// print childs
		for (const auto& pair : sortedChilds) {
			auto& symbolName = pair.get().first;
			auto& child = pair.get().second;
			for (std::size_t i = 0; i < level; ++i) {
				std::cout << "  ";
			}
			std::cout << "- " << symbolName->getName() <<
				" " << child->getCount() <<
				" (" << std::fixed << std::setprecision(2) <<
				static_cast<double>(child->getCount()) / totalSampleCount << ")" << std::endl;
			printNode(child, totalSampleCount, level+1);
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
	auto analyzer = profiler.addAnalyzer<CpuSampleHotPathAnalyzer>();
	auto interceptor = profiler.addInterceptor<CpuSampleLinuxSymbolResolveInterceptor>();
	collector->filterProcessByName(processName);
	std::cout << "collect for " << processName << " in " << collectTime << " ms" << std::endl;
	profiler.collectFor(std::chrono::milliseconds(collectTime));

	auto result = analyzer->getResult();
	auto& root = result.getRoot();
	std::cout << "- SymbolName Samples (Overhead)" << std::endl;
	std::cout << "- root " << root->getCount()  << " (1.00)" << std::endl;
	printNode(result.getRoot(), result.getTotalSampleCount(), 1);
	return 0;
}

