The source code of this class is located at [CpuSampleModel.hpp](../../include/LiveProfiler/Models/CpuSampleModel.hpp).

CpuSampleModel represent a point of execution.

# Getters in CpuSampleModel

### getIp

Returns the next instruction pointer following the instruction pointer the program is executing.

### getPid

Returns the id of the executing process.

### getTid

Returns the id of the executing thread.

### getSymbolName

Returns the symbol name associated with the instruction pointer, may be nullptr.

### getCallChainIps

Returns the result of the backtrace made at the time of execution.

### getCallChainSymbolNames

Returns the symbol names associated with instruction pointers in call chain, may contains nullptr.
The result of getCallChainIps and getCallChainSymbolNames should have the same size.

