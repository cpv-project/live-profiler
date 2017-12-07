# Header only library for real time performance analysis

There are already many profiling tools on the market,
but not much integrations well with other services.<br/>
So I decided to write a profiler library that can help peoples build their own profiler or APM agent.<br/>
This library is intend to be high performance and easy to understand.<br/>

The short term goal is to support cpu sampling on linux for native applications.<br/>
Other platform and feature support may come later.<br/>

# Design

There are some concepts in the design:

- Model: The model contains the data that needs to be analyzed
- Collector: The collector collect the model data in real time
- Analyzer: The analyzers take the model data and generate the report in real time
- Profiler: The entry point class coordinate collector and analyzers

``` text
+--------------------------------------------------------------+
| Profiler                                                     |
|                                                +----------+  |
|                                       +--------> Analyzer |  |
|                                       |        +----------+  |
|                                       |                      |
|  +-----------+   +----------+   +-----+----+   +----------+  |
|  | Collector +->-+Model Data+->-+Model Data+---> Analyzer |  |
|  +-----------+   +----------+   +-----+----+   +----------+  |
|                                       |                      |
|                                       |        +----------+  |
|                                       +--------> Analyzer |  |
|                                                +----------+  |
|                                                              |
+--------------------------------------------------------------+
```

Different to many profiler, model data is handed over to analyzer in real time,<br/>
the analyzer has the right to choose whether to incremental update the report or analysis at once,<br/>
incremental updates can reduce memory footprint and allocation, which improves performance.

The profiler should have exactly one collector and may have one or more analyzers,<br/>
this is because some collector may use io multiplexing mechanism to wait for events,<br/>
and other may periodically polls the data, mixing them will create a lot of problems.<br/>
For now if you want to use multiple collectors you should create multiple profilers and run them in different threads.<br/>
Unlike collector, analyzer should be non blocking, so multiple analyzers is allowed.<br/>

Each analyzer may return different types of report,<br/>
you can dump them to console, generate a graph, or send to an APM service,<br/>
anyway you should write your own code to handle the report.<br/>

# Implemented Collectors

TODO

# Implemented Analyzers

TODO

# Requirement

C++ compiler support at least c++14

# How To Use

TODO

# Examples

TODO

# Coding Standards

You should follow the rules below if you want to contribute.

- Use tabs instead of spaces
- For class names, use camel case and start with a upper case (e.g. SomeClass)
- For function names, use camel case and start with a lower case (e.g. someFunction)
- For local variable names, use camel case and start with a lower case (e.g. someInt)
- For global variable names, use camel case and start with a upper case (e.g. SomeGlobalValue)
- For class member names, use camel case and start with a lower case and ends with `_` (e.g. someMember_)
- Write comments for every public class and function, make code simple
- Exceptions thrown should be based on ProfilerException, and the message should contains function name
- Avoid memory allocation as much as possible, use FreeListAllocator to reuse instances

# License

LICENSE: MIT LICENSE<br/>
Copyright © 2017 303248153@github<br/>
If you have any license issue please contact 303248153@qq.com.

