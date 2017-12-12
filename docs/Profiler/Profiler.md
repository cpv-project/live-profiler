The source code of this class is located at [Profiler.hpp](../../include/LiveProfiler/Profiler/Profiler.hpp).

Profiler is the entry point class coordinate collector, analyzers and interceptors,<br/>
to create a profiler instance you need to specific a model type, for example:

``` c++
Profiler<CpuSampleModel> profiler;
```

# The terms of profiler

- The profiler should have a model type
- The profiler should have exactly one collector
- The profiler may have one or more analyzers
- The profiler should not be copied, or used in multiple threads
- The collector should not know there is a class called Profiler
- The analyzers should not know there is a class called Profiler
- The interceptors should not know there is a class called Profiler

# Functions in profiler

### useCollector

Use specified collector, replaces the old collector if this function is called twice.

Example:

``` c++
Profiler<CpuSampleModel> profiler;
auto collector = profiler.useCollector<CpuSampleLinuxCollector>();
```

### addAnalyzer

Add analyzer to analyzer list.

Example:

``` c++
Profiler<CpuSampleModel> profiler;
auto analyzer = profiler.addAnalyzer<CpuSampleFrequencyAnalyzer>();
```

### removeAnalyzer

Remove analyzer from analyzer list, return whether the analyzer is in the list.

Example:

``` c++
Profiler<CpuSampleModel> profiler;
auto analyzer = profiler.addAnalyzer<CpuSampleFrequencyAnalyzer>();
profiler.removeAnalyzer(analyzer);
```

### addInterceptor

Add interceptor to interceptor list.

Example:

``` c++
Profiler<CpuSampleModel> profiler;
auto interceptor = profiler.addInterceptor<CpuSampleLinuxSymbolResolveInterceptor>();
```

### removeInterceptor

Remove interceptor from interceptor list, return whether the interceptor is in the list.

Example:

``` c++
Profiler<CpuSampleModel> profiler;
auto interceptor = profiler.addInterceptor<CpuSampleLinuxSymbolResolveInterceptor>();
profiler.removeInterceptor(interceptor);
```

### reset

Reset state of collectors and analyzers.

Example:

``` c++
Profiler<CpuSampleModel> profiler;
// add collector, analyzer and interceptor...
while (true) {
	profiler.collectFor(std::chrono::milliseconds(1000));
	// output analysis results...
	profiler.reset();
}
```

### collectFor

Collect and feed the data to the analyzers for the specified time.

Example:

``` c++
Profiler<CpuSampleModel> profiler;
// add collector, analyzer and interceptor...
profiler.collectFor(std::chrono::milliseconds(1000));
```

