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

For java you need a jvm agent from [here](https://github.com/jvm-profiling-tools/perf-map-agent).<br/>
Since upstream didn't support "agentpath" option I will use a fork here until this [PR](https://github.com/jvm-profiling-tools/perf-map-agent/pull/63) is merged.

Build perf-map-agent:

``` bash
git clone https://github.com/trustin/perf-map-agent
cd perf-map-agent
export JAVA_HOME=/usr/lib/jvm/java-9-openjdk-amd64
cmake .
make
sudo cp -f out/libperfmap.so /usr/lib
```

Then execute your java program:

``` bash
java -agentpath:/usr/lib/libperfmap.so programName
```

### .Net

Support for .net(coreclr only) is very simple.<br/>
Just export the following environment variable and execute your .net program.

``` bash
export COMPlus_PerfMapEnabled=1
dotnet programName.dll
```

It may cause many junk files leaved in "/tmp" if you don't clean it manually,<br/>
the suggested solution would be mount "/tmp" to a memory based temporary file system.

# Example

Just call addInterceptor is enough.

``` c++
Profiler<CpuSampleModel> profiler;
auto collector = profiler.useCollector<CpuSampleLinuxCollector>();
auto interceptor = profiler.addInterceptor<CpuSampleLinuxSymbolResolveInterceptor>();
```

