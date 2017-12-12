The source code of this class is located at [CpuSampleLinuxSymbolResolveInterceptor.hpp](../../include/LiveProfiler/Interceptors/CpuSampleLinuxSymbolResolveInterceptor.hpp).

CpuSampleLinuxSymbolResolveInterceptor is an interceptor used to setup symbol names in model data.

It can resolve three types of symbol name:

- Symbol name in ELF binary (normal, or dynamic)
- Symbol name in linux kernel (load from /proc/kallsyms)
- Custom symbol name (load from /map/perf-$pid.map)

# Support for native programs

Native programs written in c, c++, go, etc are supported by default, usually you don't need to change any code to get it work.
However, there some minor problems:

### Some symbol names are missing

See this link: [StackOverflow](https://stackoverflow.com/questions/6934659/how-to-make-backtrace-backtrace-symbols-print-the-function-names)

For gcc and clang, use `-rdynamic` option may solve this problem.

### The call chain is incomplete

See this link: [StackOverflow](https://stackoverflow.com/questions/14666665/trying-to-understand-gcc-option-fomit-frame-pointer)

For gcc and clang, use `-fomit-frame-pointer` option may solve this problem.

# Support for vm based programs

VM based programs written in java, .net, etc needs to do some extra works.

### Java

Follow the instruction here:

[https://github.com/jvm-profiling-tools/perf-map-agent](https://github.com/jvm-profiling-tools/perf-map-agent)

The command you need should be "create-java-perf-map.sh <pid>".

### .Net

Support for .net(coreclr only) is very simple, just export the following environment variable:

``` bash
export COMPlus_PerfMapEnabled=1
```

And execute your .net program.

It may cause many junk files leaved in "/tmp" if you don't clean it manually,<br/>
the suggested solution would be mount "/tmp" to a memory based temporary file system.

# Example

Just call addInterceptor is enough.

``` c++
Profiler<CpuSampleModel> profiler;
auto collector = profiler.useCollector<CpuSampleLinuxCollector>();
auto interceptor = profiler.addInterceptor<CpuSampleLinuxSymbolResolveInterceptor>();
```

