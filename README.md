# Header only library for real time performance analysis [![Build Status](https://travis-ci.org/cpv-project/live-profiler.svg?branch=master)](https://travis-ci.org/cpv-project/live-profiler)

There are already many profiling tools on the market,
but not much integrations well with other services.<br/>
So I decided to write a profiler library that can help peoples build their own profiler or APM agent.<br/>
This library is intend to be high performance and easy to understand.<br/>

The short term goal is to support cpu sampling on linux for native applications.<br/>
Other platform and feature support may come later.<br/>

# Design

There are some concepts in the design:

- Model: The model contains the data that needs to be analyzed
- Collector: The collector collect the model data in real time
- Analyzer: The analyzers take the model data and generate the report in real time
- Interceptor: The interceptors alter the model data in real time
- Profiler: The entry point class coordinate collector, analyzers and interceptors

``` text
+-------------------------------------------------------------------------------------+
| Profiler                                                                            |
|                                                                       +----------+  |
|                                                             +---------> Analyzer |  |
|                                                             |         +----------+  |
|                                                             |                       |
|  +-----------+   +------------+   +-------------+   +-------+-----+   +----------+  |
|  | Collector +---> Model Data +---> Interceptor +---> Model Data' +---> Analyzer |  |
|  +-----------+   +------------+   +-------------+   +-------+-----+   +----------+  |
|                                                             |                       |
|                                                             |         +----------+  |
|                                                             +---------> Analyzer |  |
|                                                                       +----------+  |
|                                                                                     |
+-------------------------------------------------------------------------------------+
```

Different to many profiler, model data is handed over to analyzer in real time,<br/>
the analyzer has the right to choose whether to incremental update the report or analysis at once,<br/>
incremental updates can reduce memory footprint and allocation, which improves performance.

The profiler should have exactly one collector and may have one or more analyzers or interceptors,<br/>
this is because some collector may use io multiplexing mechanism to wait for events,<br/>
and other may periodically polls the data, mixing them will create a lot of problems.<br/>
For now if you want to use multiple collectors you should create multiple profilers and run them in different threads.<br/>
Unlike collector, analyzers and interceptors should be non blocking, so more than one is allowed.<br/>

Each analyzer may return different types of report,<br/>
you can dump them to console, generate a graph, or send to an APM service,<br/>
anyway you should write your own code to handle the report.<br/>

# Requirement

C++ compiler support at least c++14

# How To Use

There are many combinations of collectors and analyzers,<br/>
here I chose the example "CpuSampleFrequencyAnalyzer" to explain,<br/>
this example program can analyze which functions have the higest CPU usage. 

First, install the required packages:

- Ubuntu: `sudo apt-get install g++ binutils-dev`
- Fedora: `su -c "dnf install gcc-c++ binutils-devel"`

Then, compile and run the example:

``` bash
cd live-profiler/examples/CpuSampleFrequencyAnalyzer
sh run.sh a.out 20000
```

It collects the running status of all programs named "a.out" in real time, and output the report after 20 seconds.<br/>
The content of the report is like:

```
top 13 inclusive symbol names:
No. Overhead Samples Symbol Name
  1     0.42   86582 make(int, NodePool&)
  2     0.13   26340 apr_palloc
  3     0.12   25406 main._omp_fn.0
  4     0.09   17897 GOMP_parallel
  5     0.07   13619 vmxarea
  6     0.05   10930 Node::check() const
  7     0.00     496 apr_pool_clear
  8     0.00     335 main
  9     0.00     335 __libc_start_main
 10     0.00      54 apr_allocator_destroy
 11     0.00      29 apr_pool_destroy
 12     0.00      11 __munmap
 13     0.00       3 mmap

top 9 exclusive symbol names:
No. Overhead Samples Symbol Name
  1     0.51   26340 apr_palloc
  2     0.23   11869 make(int, NodePool&)
  3     0.19    9627 Node::check() const
  4     0.03    1565 main._omp_fn.0
  5     0.01     496 apr_pool_clear
  6     0.00      54 apr_allocator_destroy
  7     0.00      29 apr_pool_destroy
  8     0.00      11 __munmap
  9     0.00       3 mmap
```

Because this project is a library, you may be more interested in how this example program is written,<br/>
let's see the code:

``` c++
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
```

Function "printTopSymbolNames" is only used to output the report, it doesn't matter.<br/>
The second part of the main function is important, let's break it down one step at a time:

First decide which model to use, in this case it's "CpuSampleModel", which represent a point of execution:

``` c++
Profiler<CpuSampleModel> profiler;
```

Next decide who provided these model data, in this case it's "CpuSampleLinuxCollector":

``` c++
auto collector = profiler.useCollector<CpuSampleLinuxCollector>();
```

Then decide who analyzes these model data, in this case it's "CpuSampleFrequencyAnalyzer":

``` c++
auto analyzer = profiler.addAnalyzer<CpuSampleFrequencyAnalyzer>();
```

Because "CpuSampleFrequencyAnalyzer" requires function symbol names,<br/>
and "CpuSampleLinuxCollector" only provides memory address,<br/>
a third party is need to convert memory address to function symbol name:

``` c++
auto interceptor = profiler.addInterceptor<CpuSampleLinuxSymbolResolveInterceptor>();
```

Before start the collecting, we need to tell "CpuSampleLinuxCollector" which processes is interested,<br/>
processName can be "a.out", "python3", "java" or whatever, here it takes from command line:

``` c++
collector->filterProcessByName(processName);
```

Now everything is ready, start collecting the data for the specified time.<br/>
Function "collectFor" can be called multiple times, and the data will be accumulated.

``` c++
profiler.collectFor(std::chrono::milliseconds(collectTime));
```

Finally, enough data has been collected, we can start the analysis,<br/>
different analyzers give different types of results,<br/>
"CpuSampleFrequencyAnalyzer" will give the top inclusive and exclusive symbol names:

``` c++
auto result = analyzer->getResult(topInclusive, topExclusive);
```

To compile this code, use the following command (also see it in run.sh):

``` bash
g++ -Wall -Wextra --std=c++14 -O3 -g -I../../include Main.cpp -lbfd
```

Now you should be able to write a minimal profiler,<br/>
you can find more detailed information from the following documents.

# Documents

### Profiler

- Profiler ([Document](./docs/Profiler/Profiler.md))

### Models

- CpuSampleModel ([Document](./docs/Models/CpuSampleModel.md))

### Collectors

- BaseCollector ([Document](./docs/Collectors/BaseCollector.md))
- CpuSampleLinuxCollector ([Document](./docs/Collectors/CpuSampleLinuxCollector.md))

### Analyzers

- BaseAnalyzer ([Document](./docs/Analyzers/BaseAnalyzer.md))
- CpuSampleDebugAnalyzer ([Document](./docs/Analyzers/CpuSampleDebugAnalyzer.md))
- CpuSampleFrequencyAnalyzer ([Document](./docs/Analyzers/CpuSampleFrequencyAnalyzer.md))
- CpuSampleHotPathAnalyzer ([Document](./docs/Analyzers/CpuSampleHotPathAnalyzer.md))

### Interceptors

- BaseInterceptor ([Document](./docs/Interceptors/BaseInterceptor.md))
- CpuSampleLinuxSymbolResolveInterceptor ([Document](./docs/Interceptors/CpuSampleLinuxSymbolResolveInterceptor.md))

# Coding Standards

You should follow the rules below if you want to contribute.

- Use tabs instead of spaces
- For class names, use camel case and start with a upper case (e.g. SomeClass)
- For function names, use camel case and start with a lower case (e.g. someFunction)
- For local variable names, use camel case and start with a lower case (e.g. someInt)
- For global variable names, use camel case and start with a upper case (e.g. SomeGlobalValue)
- For class member names, use camel case and start with a lower case and ends with `_` (e.g. someMember_)
- Write comments for every public class and function, make code simple
- Exceptions thrown should be based on ProfilerException, and the message should contains function name
- Avoid memory allocation as much as possible, use FreeListAllocator to reuse instances

# License

LICENSE: MIT LICENSE<br/>
Copyright © 2017 303248153@github<br/>
If you have any license issue please contact 303248153@qq.com.

