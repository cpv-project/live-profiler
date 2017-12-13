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

### getResult

Generate the result.
The result type is defined as:

``` c++
using SymbolNameAndCountType = std::pair<std::shared_ptr<SymbolName>, std::size_t>;

class ResultType {
public:
	const std::vector<SymbolNameAndCountType>& getTopInclusiveSymbolNames() const&;
	const std::vector<SymbolNameAndCountType>& getTopExclusiveSymbolNames() const&;
	std::size_t getTotalSampleCount() const;
};
```

# Example

Example code: [Main.cpp](../../examples/CpuSampleFrequencyAnalyzer/Main.cpp)

Run example:

``` bash
sh run.sh a.out 10000
```

The output is like:

``` text
top 16 inclusive symbol names:
No. Overhead Samples SymbolName
  1     1.71   50964 make(int, NodePool&)
  2     0.50   14860 apr_palloc
  3     0.47   13905 main._omp_fn.0
  4     0.30    8969 GOMP_parallel
  5     0.26    7776 vmxarea
  6     0.22    6496 Node::check() const
  7     0.01     279 apr_pool_clear
  8     0.01     185 main
  9     0.01     185 __libc_start_main
 10     0.00      47 apr_allocator_destroy
 11     0.00      25 apr_pool_destroy
 12     0.00      11 __munmap
 13     0.00       2 __vsprintf_chk
 14     0.00       2 mmap
 15     0.00       1 _IO_default_xsputn
 16     0.00       1 vfprintf

top 11 exclusive symbol names:
No. Overhead Samples SymbolName
  1     0.50   14860 apr_palloc
  2     0.23    6793 make(int, NodePool&)
  3     0.19    5749 Node::check() const
  4     0.04    1122 main._omp_fn.0
  5     0.01     279 apr_pool_clear
  6     0.00      47 apr_allocator_destroy
  7     0.00      25 apr_pool_destroy
  8     0.00      11 __munmap
  9     0.00       2 mmap
 10     0.00       1 _IO_default_xsputn
 11     0.00       1 vfprintf
```

