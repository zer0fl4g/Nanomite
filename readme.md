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
+ fixed a bug which lead to a crash if a wow64 file has ordinal imports
+ fixed a bug in the disassembly view which caused ungentle down scrolling
+ fixed a crash on context menus if not debugging something
+ added display of current PID/TID in mainwindow title
+ added function view
+ added and cleaned context menus

####Notes:
	- function view
		- parsed the memory of the loaded targets and scanns for functions.

###For the full changelog click [here](https://github.com/zer0fl4g/Nanomite/blob/master/changelog.md)