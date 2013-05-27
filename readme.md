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
+ fixed a bug which lead to incorrect restarts on slow systems
+ fixed scrollbar in Trace View
+ fixed display of ascii strings in ascii view
+ fixed a possible crash in disassembler
+ fixed a bug which caused double calling of some functions in context menus
+ fixed a bug in Hex View which may showed wrong data
+ fixed a bug in Heap View which caused a crash when copying the whole line to clipboard
+ added PatchManager
+ added commandline options
+ added possibility to set process priorities
+ added display of current priority in detail view - context menu
+ added display of segment registers in Reg View
+ removed processes we can´t access from the attach dialog

####Notes:
	- Patches can be saved to file (only on the debugged one)
	- Commandline options
		- "-s": specifies a file
		- "-c": specifies the commandline for the target if not given you will be asked later
		- "-p": attachs to the given pid

###For the full changelog click [here](https://github.com/zer0fl4g/Nanomite/blob/master/changelog.md)