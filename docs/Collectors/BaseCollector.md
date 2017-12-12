The source code of this class is located at [BaseCollector.hpp](../../include/LiveProfiler/Collectors/BaseCollector.hpp).

BaseCollector is the base class for the collector.

# The terms of collector

- The collector should be able to reset the state to it's initial state
- The collector should support enabling and disabling collection
- The collector should support collecting performance data for the specified timeout period
- The collector should return different data each time `collect` is called
- The collector should not know there is a class called Profiler

# Functions in collector

### reset (implemented in child class)

Reset the state to it's initial state.

### enable (implemented in child class)

Enable performance data collection.

### collect (implemented in child class)

Collect performance data for the specified timeout period.

### disable (implemented in child class)

Disable performance data collection.

