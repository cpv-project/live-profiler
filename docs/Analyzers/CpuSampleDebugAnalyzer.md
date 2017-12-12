The source code of this class is located at [CpuSampleDebugAnalyzer.hpp](../../include/LiveProfiler/Analyzers/CpuSampleDebugAnalyzer.hpp).

CpuSampleDebugAnalyzer is an analyzer used to print details of cpu sample to screen for debugging.

# Example

Example code: [Main.cpp](../../examples/CpuSampleDebugAnalyzer/Main.cpp)

Run example:

``` bash
sh run.sh a.out 10000
```

The output is like:

``` text
[CpuSample] receive 8 models
[CpuSample] - 7f880821fe80
[CpuSample]   > putp
[CpuSample]   > /lib/x86_64-linux-gnu/libtinfo.so.5.9
[CpuSample]   - 555edd78bc3e
[CpuSample]   - 6096258d4c544155
[CpuSample]     > vmxarea
[CpuSample]     > [kallsyms]
[CpuSample] - 7f8807c79c39
[CpuSample]   > vfprintf
[CpuSample]   > /lib/x86_64-linux-gnu/libc-2.26.so
[CpuSample]   - 7f8807d4aa8b
[CpuSample]     > __vsprintf_chk
[CpuSample]     > /lib/x86_64-linux-gnu/libc-2.26.so
[CpuSample]   - 7ffd30c0ac40
[CpuSample] - 7f8807c89167
[CpuSample]   > _IO_vfscanf
[CpuSample]   > /lib/x86_64-linux-gnu/libc-2.26.so
[CpuSample]   - 7f8807c9f20a
[CpuSample]     > vsscanf
[CpuSample]     > /lib/x86_64-linux-gnu/libc-2.26.so
[CpuSample]   - 2520642520642520
[CpuSample]     > vmxarea
[CpuSample]     > [kallsyms]
[CpuSample] - 7f8807c8a43d
[CpuSample]   > _IO_vfscanf
[CpuSample]   > /lib/x86_64-linux-gnu/libc-2.26.so
[CpuSample]   - 7f8807c9f20a
[CpuSample]     > vsscanf
[CpuSample]     > /lib/x86_64-linux-gnu/libc-2.26.so
[CpuSample]   - 2520642520642520
[CpuSample]     > vmxarea
[CpuSample]     > [kallsyms]
[CpuSample] - 7f8807da4282
[CpuSample]   - 6565656565656565
[CpuSample]     > vmxarea
[CpuSample]     > [kallsyms]
[CpuSample] - 7f8807cabd21
[CpuSample]   > _IO_default_xsputn
[CpuSample]   > /lib/x86_64-linux-gnu/libc-2.26.so
[CpuSample] - 7f8807c89621
[CpuSample]   > _IO_vfscanf
[CpuSample]   > /lib/x86_64-linux-gnu/libc-2.26.so
[CpuSample]   - 7f8807c9f20a
[CpuSample]     > vsscanf
[CpuSample]     > /lib/x86_64-linux-gnu/libc-2.26.so
[CpuSample]   - 20646c2520646c25
[CpuSample]     > vmxarea
[CpuSample]     > [kallsyms]
[CpuSample] - 7f8807c8a467
[CpuSample]   > _IO_vfscanf
[CpuSample]   > /lib/x86_64-linux-gnu/libc-2.26.so
[CpuSample]   - 7f8807c9f20a
[CpuSample]     > vsscanf
[CpuSample]     > /lib/x86_64-linux-gnu/libc-2.26.so
[CpuSample]   - 2520642520642520
[CpuSample]     > vmxarea
[CpuSample]     > [kallsyms]
```

