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
###Version 0.1 beta 8
+ fixed a crash in attaching to a process where we don´t have a file path
+ fixed a bug which ignored DbgBreakPoint on attaching
+ fixed a bug which caused double breaking in case we set a breakpoint while beeing on the entrypoint
+ fixed a crash in pe import reader
+ fixed a crash (see github issue #1)
+ fixed a bug in HexView which didn´t display data on x64 processes
+ fixed a possible crash when opening invalid non pe files
+ fixed a small bug in "Restart"
+ added display of current function in windowtitle
+ added support for drag and drop of files
+ added possibility to remove breakpoints with "F2" (needs to be a selected row in disassembler)
+ added "Step back to user code"

####Notes:
	- "Step back to user code"
		- If you use this the debugger will continue the execution until you get to the first function 
		  which is located in the main module

###For the full changelog click [here](https://github.com/zer0fl4g/Nanomite/blob/master/changelog.md)