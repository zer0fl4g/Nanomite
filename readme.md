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
###Version 0.1 beta 11
+ fixed a bug in options which didn´t save the correct settings
+ fixed display of exceptions if no symbols have been found
+ fixed a bug which didn´t display exceptions if a breakpoint was set on this offset
+ improvements on AttachDlg
+ added cleanup on debugge termination
+ added process patching

###For the full changelog click [here](https://github.com/zer0fl4g/Nanomite/blob/master/changelog.md)