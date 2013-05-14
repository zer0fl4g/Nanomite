#Nanomite - Graphical Debugger for x64 and x86 on Windows

## Features
- Debugging x86 and x64 (+ WOW64) processes
- Breakpoints
    - Software (Int3)
	- Memory (Page Guard)
	- Hardware (DR CPU Regs)
- Step In
- Step Over
- Step Out
- Step back to user code
- Attaching
- Detaching
- Single Step Tracing
- Supporting child processes
- Supporting multithreading
- Display source code (if found)
- Detailed view of:
	- disassembly
	- windows
	- handles
	- debug strings
	- threads
	- (child)processes
	- exceptions
	- loaded modules
	- strings
	- callstack
	- stack
	- memory
	- heap
	- cpu registers
	- functions
	- pe header

## Changelog
###Version 0.1 beta 12
+ fixed a bug in Hex View which may showed wrong data
+ fixed a bug in Heap View which caused a crash when copying the while line to clipboard
+ added PatchManager
+ removed processes we can´t access from the attach dialog

####Notes:

###For the full changelog click [here](https://github.com/zer0fl4g/Nanomite/blob/master/changelog.md)