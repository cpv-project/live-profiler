The source code of this class is located at [BaseInterceptor.hpp](../../include/LiveProfiler/Interceptors/BaseInterceptor.hpp).

BaseInterceptor is the base class for the interceptor.

# The terms of interceptor

- The interceptor should be able to reset the state to it's initial state
- The interceptor should be able to alter performance data at any time
- The interceptor should leave performance data in a valid state after alter
- The interceptor should not know there is a class called Profiler

# Functions in interceptor

### reset (implemented in child class)

Reset the state to it's initial state.

### alter (implemented in child class)

Alter performance data.

