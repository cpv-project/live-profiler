The source code of this class is located at [BaseAnalyzer.hpp](../../include/LiveProfiler/Analyzers/BaseAnalyzer.hpp).

BaseAnalyzer is the base class for the analyzer.

# The terms of analyzer

- The analyzer should be able to reset the state to it's initial state
- The analyzer should be able to receive performance data at any time
- The analyzer should provide a function named `getResult`, this function can return any type
- The analysis of performance data can be done in real time, or at the time of `getResult`
- The analyzer should not know there is a class called Profiler

# Functions in analyzer

### reset (implemented in child class)

Reset the state to it's initial state.

### feed (implemented in child class)

Receive performance data.

