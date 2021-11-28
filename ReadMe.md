# About
MemProfiler is a memory profiling library designed as a simple include to track allocations, frees, and memory leaks across your application.

While not as versatile as tools like Valgrind, the aim of this project is to provide a simple way of profiling your application in terms of memory based activities with the goal of identifying potential issues.

# Requirements
C++11 onwards

# Usage
In order to use this library, simply add `#include "MemProfiler.h"` to your `main.cpp` file.

While your application is running MemProfiler will save all allocations / frees to a `log.txt` file. Once execution is complete, the memory data checked to find any memory leaks. This is then saved to log.txt along with stack trace information.

# Logging options
By default MemProfiler will log all memory allocations, de-allocations as well as errors. To disable this and only log errors set the `LOG_ALL` definition in `MemProfiler.h` to false.

MemProfiler will also output all errors to stdout by default, to disable this and only log errors to file via stderr set the `OUTPUT_STDOUT` definition in `MemProfiler.h` to false.

# Example output
Once the program finishes execution MemProfiler will check allocations against frees and compile a list of errors in the following format:
```===! ERROR !===
[ERROR] - Dangling pointer at memory address: 0x1af6eb0
[ERROR] - Memory leak size: 4 bytes
[ERROR] - Stack Trace: 
./main(_ZN11MemProfiler11MemProfiler8allocateEPvm+0x35) [0x402afd]
./main(_Znwm+0x37) [0x40227d]
./main(_Z6myFunci+0x15) [0x4022ca]
./main(main+0x12) [0x4022ef]
/usr/lib/libc.so.6(__libc_start_main+0xd5) [0x7f5a09e0cb25]
./main(_start+0x2a) [0x40218a]
```

A stack trace is provided and `may` contain function names to assist with debugging, as long as debug symbols are enabled in your compiler. As compilers vary across platforms this information may or may not be complete gibberish, YMMV. 

Note: The default stack trace depth is 20, this can be modified in the `MemStack.h` file, see const: `BACKTRACE_DEPTH`
