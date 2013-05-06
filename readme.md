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
+ fixed a bug which made register editor not working in x64
+ fixed a bug which displayed wrong modules in window view
+ fixed a bug which didn´t display exceptions if a breakpoint was set on this offset
+ fixed a bug in disassembler which may caused application crashes due to wrong memory protection
+ fixed a possible crash in context menus
+ improvements on AttachDlg
+ added cleanup on debugge termination
+ added process patching
+ added more context menus to DetailView
+ added display of mainthread in DetailView
+ added F5 Hotkey to reload some views
+ added possibility to save debug log to file
+ added possibility to copy data to clipboard
+ added possibility to break on new Processes, Threads or DLL loads
+ added dockable widgets to the mainview

####Notes:
	- You have now the possibility to patch the memory of a process. Currently it is not possible to save this
	  changes to disk but this will be integrated also.
	- Some context menus offer the possibility to copy the data from the table to the clipboard

###For the full changelog click [here](https://github.com/zer0fl4g/Nanomite/blob/master/changelog.md)