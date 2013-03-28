#Nanomite - Graphical Debugger for x64 and x86 on Windows

## Features
- Debugging x86 and x64 (+ WOW64) processes
- Breakpoints
    - Software (Int3)
	- Memory (Page Guard)
	- Hardware (DR CPU Regs)
- Step In
- Step Over
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

## Changelog
###Version 0.1 beta 9
+ fixed a bug in disassembler
+ fixed a bug in wow64 StepIn
+ fixed a crash when suspending a Process and then StepIn
+ fixed a bug which didn´t display all modules in callstack
+ fixed a crash in loading imports of files without IAT
+ fixed a crash in "Goto Offset" context menu
+ small gui improvements
+ added Single Step Tracer
+ added memory dumper
+ removed error message if you cancel the file selection

###For the full changelog click [here](https://github.com/zer0fl4g/Nanomite/blob/master/changelog.md)