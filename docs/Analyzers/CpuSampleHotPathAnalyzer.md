The source code of this class is located at [CpuSampleHotPathAnalyzer.hpp](../../include/LiveProfiler/Analyzers/CpuSampleHotPathAnalyzer.hpp).

CpuSampleFrequencyAnalyzer is an analyzer used to find out which call path takes the most cpu usage.

The structure of the result is like:

- root 100 (1.00)
	- A 50 (0.50)
	- B 35 (0.35)
		- C 25 (0.25)
		- D 5 (0.05)

The missing number means there are some samples have none symbol name.
 
# Functions in CpuSampleFrequencyAnalyzer

### getResult

Generate the result.
The result type is defined as:

``` c++
class NodeType {
public:
	std::size_t getCount() const;
	const std::unordered_map<std::shared_ptr<SymbolName>, std::unique_ptr<NodeType>>& getChilds() const&;
};

class ResultType {
public:
	const std::unique_ptr<NodeType>& getRoot() const&;
	std::size_t getTotalSampleCount() const;
};
```

# Example

Example code: [Main.cpp](../../examples/CpuSampleHotPathAnalyzer/Main.cpp)

Run example:

``` bash
sh run.sh a.out 10000
```

The output is like:

``` text
- SymbolName Samples (Overhead)
- root 48038 (1.00)
  - main._omp_fn.0 29988 (0.62)
    - make(int, NodePool&) 23590 (0.49)
      - make(int, NodePool&) 22953 (0.48)
        - make(int, NodePool&) 20486 (0.43)
          - make(int, NodePool&) 16162 (0.34)
            - make(int, NodePool&) 12036 (0.25)
              - make(int, NodePool&) 7793 (0.16)
                - make(int, NodePool&) 4200 (0.09)
                  - apr_palloc 1955 (0.04)
                  - make(int, NodePool&) 1045 (0.02)
                - apr_palloc 2017 (0.04)
              - apr_palloc 2843 (0.06)
            - apr_palloc 2355 (0.05)
          - apr_palloc 3011 (0.06)
        - apr_palloc 2027 (0.04)
      - apr_palloc 517 (0.01)
    - Node::check() const 6210 (0.13)
      - Node::check() const 1072 (0.02)
    - apr_palloc 129 (0.00)
  - vmxarea 16842 (0.35)
    - GOMP_parallel 16095 (0.34)
      - main._omp_fn.0 12685 (0.26)
        - make(int, NodePool&) 5584 (0.12)
          - apr_palloc 2454 (0.05)
          - make(int, NodePool&) 1344 (0.03)
        - apr_palloc 2965 (0.06)
        - Node::check() const 2509 (0.05)
      - apr_palloc 2882 (0.06)
      - Node::check() const 118 (0.00)
      - apr_pool_clear 93 (0.00)
      - make(int, NodePool&) 84 (0.00)
    - __libc_start_main 658 (0.01)
      - main 658 (0.01)
        - make(int, NodePool&) 377 (0.01)
          - make(int, NodePool&) 377 (0.01)
            - make(int, NodePool&) 377 (0.01)
              - make(int, NodePool&) 377 (0.01)
                - make(int, NodePool&) 377 (0.01)
                  - make(int, NodePool&) 377 (0.01)
                    - make(int, NodePool&) 377 (0.01)
                      - make(int, NodePool&) 372 (0.01)
                        - make(int, NodePool&) 363 (0.01)
                          - make(int, NodePool&) 324 (0.01)
                            - make(int, NodePool&) 167 (0.00)
                              - make(int, NodePool&) 41 (0.00)
                              - apr_palloc 37 (0.00)
                            - apr_palloc 130 (0.00)
                          - apr_palloc 29 (0.00)
                        - apr_palloc 7 (0.00)
                      - apr_palloc 4 (0.00)
        - Node::check() const 281 (0.01)
          - Node::check() const 157 (0.00)
    - apr_allocator_destroy 35 (0.00)
    - __munmap 5 (0.00)
    - apr_palloc 1 (0.00)
  - apr_palloc 608 (0.01)
  - apr_pool_clear 346 (0.01)
  - apr_pool_destroy 37 (0.00)
  - apr_allocator_destroy 18 (0.00)
  - Node::check() const 12 (0.00)
  - mmap 8 (0.00)
  - make(int, NodePool&) 2 (0.00)
  - _IO_file_xsputn 1 (0.00)
  - __munmap 1 (0.00)
```

