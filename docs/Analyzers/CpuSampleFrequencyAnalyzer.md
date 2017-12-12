The source code of this class is located at [CpuSampleFrequencyAnalyzer.hpp](../../include/LiveProfiler/Analyzers/CpuSampleFrequencyAnalyzer.hpp).

CpuSampleFrequencyAnalyzer is an analyzer used to find out which symbol names appear most frequently.

There two different rankings:
- Top Inclusive Symbol Names: The functions that uses the most cpu, include the functions it called
- Top Exclusive Symbol Names: The functions that uses the most cpu, not include the functions it called

# Functions in CpuSampleFrequencyAnalyzer

### setInclusiveTraceLevel

Set how many levels should be considered for inclusive sampling, the default value is 3.
For example:

```
- a
  - b
  - c
  - d
  - e
(e calls d calls c calls b calls a)
```

If inclusive trace level is 3, then `[a, b, c, d]` will be treated as inclusive sample.

# Example

Example code: [Main.cpp](../../examples/CpuSampleFrequencyAnalyzer/Main.cpp)

Run example:

``` bash
sh run.sh a.out 10000
```

The output is like:

``` text
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

