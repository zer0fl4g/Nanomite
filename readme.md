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

## Changelog
###Version 0.1 beta 10
+ fixed display of wrong function offset in callstack
+ fixed a bug which didn´t break on module ep if "break on system ep" was selected
+ added display of current PID/TID in mainwindow title

###For the full changelog click [here](https://github.com/zer0fl4g/Nanomite/blob/master/changelog.md)