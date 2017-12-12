The source code of this class is located at [CpuSampleLinuxCollector.hpp](../../include/LiveProfiler/Collectors/CpuSampleLinuxCollector.hpp).

CpuSampleLinuxCollector is a collector for collecting cpu samples on linux, based on perf_events.

CpuSampleLinuxCollector only support linux.

You may want to read the document of [CpuSampleLinuxSymbolResolveInterceptor](../Interceptors/CpuSampleLinuxSymbolResolveInterceptor.md) as well.<br/>
In most cases CpuSampleLinuxSymbolResolveInterceptor should be used with CpuSampleLinuxCollector.

# Functions in CpuSampleLinuxCollector

### setIncludeCallChain

Set whether to include callchain in samples.
Exclude callchain in samples will improve performance.
Default value is true.

Exclude callchain would break some features like inclusive samples analysis.

Example:

``` c++
Profiler<CpuSampleModel> profiler;
auto collector = profiler.useCollector<CpuSampleLinuxCollector>();
collector->setIncludeCallChain(false);
```

### setProcessesUpdateInterval

Set how often to update the list of processes.
Default value is 100ms.

Example:

``` c++
Profiler<CpuSampleModel> profiler;
auto collector = profiler.useCollector<CpuSampleLinuxCollector>();
collector->setProcessesUpdateInterval(std::chrono::milliseconds(50));
```

### filterProcessBy

Use the specified function to decide which processes to monitor.

Example:

``` c++
Profiler<CpuSampleModel> profiler;
auto collector = profiler.useCollector<CpuSampleLinuxCollector>();
collector->filterProcessBy([](pid_t pid) { return pid == 123; });
```

### filterProcessByName

Use the specified process name to decide which processes to monitor.

Example:

``` c++
Profiler<CpuSampleModel> profiler;
auto collector = profiler.useCollector<CpuSampleLinuxCollector>();
collector->filterProcessByName("a.out");
```

### setSamplePeriod

Set how often to take a sample, the unit is cpu clock.
Default value is 100000.

Example:

``` c++
Profiler<CpuSampleModel> profiler;
auto collector = profiler.useCollector<CpuSampleLinuxCollector>();
collector->setSamplePeriod(300000);
```

### setMmapPageCount

Set how many pages for the mmap ring buffer,
this count is not contains metadata page, and should be power of 2.
Default value is 8.

Example:

``` c++
Profiler<CpuSampleModel> profiler;
auto collector = profiler.useCollector<CpuSampleLinuxCollector>();
collector->setMmapPageCount(16);
```

### setWakeupEvents

Set the number of records required to raise an event.
A larger value may improve performance but delay the collection.
Default value is 8.

Example:

``` c++
Profiler<CpuSampleModel> profiler;
auto collector = profiler.useCollector<CpuSampleLinuxCollector>();
collector->setWakeupEvents(16);
```

### setExcludeUser

Set whether to exclude samples in user space.
Default value is false.

Example:

``` c++
Profiler<CpuSampleModel> profiler;
auto collector = profiler.useCollector<CpuSampleLinuxCollector>();
collector->setExcludeUser(true);
```

### setExcludeKernel

Set whether to exclude samples in kernel space.
Default value is true.

Example:

``` c++
Profiler<CpuSampleModel> profiler;
auto collector = profiler.useCollector<CpuSampleLinuxCollector>();
collector->setExcludeKernel(false);
```

### setExcludeHypervisor

Set whether to exclude samples in hypervisor.
Default value is true.

Example:

``` c++
Profiler<CpuSampleModel> profiler;
auto collector = profiler.useCollector<CpuSampleLinuxCollector>();
collector->setExcludeHypervisor(false);
```

