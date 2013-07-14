#Nanomite - Graphical Debugger for x64 and x86 on Windows

## Features
- Debugging x86 and x64 (+ WOW64) processes
- Breakpoints
    - Software
	- Memory
	- Hardware
- Step In
- Step Over
- Step Out
- Step back to user code
- Attaching
- Detaching
- Single Step Tracing
- Supporting child processes
- Supporting multithreading
- Display source code
- Detailed view of:
	- disassembly
	- windows
	- handles
	- debug strings
	- threads
	- (child)processes
	- exceptions
	- process privileges
	- loaded modules
	- strings
	- callstack
	- stack
	- memory
	- heap
	- cpu registers
	- functions
	- pe header
	- TEB/TBI
	- PEB/PBI

## Changelog
###Version 0.1 beta 14
+ fixed a bug in the options not showing exception wich have been saved using the exception assistant
+ fixed a bug when stepping over a return
+ fixed a bug in breakpoint manager which deleted the wrong bp when the user wants to remove the selected bp
+ fixed a bug in breakpoint manager which created unusable breakpoints
+ fixed some handle leaks
+ added f5 hotkey to attach dialog to reload the processlist
+ added type column in attach dialog

###For the full changelog click [here](https://github.com/zer0fl4g/Nanomite/blob/master/changelog.md)