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
+ fixed a bug in breakpoint manager which deleted the wrong bp when removing a selected bp
+ fixed a bug in breakpoint manager which created unusable breakpoints
+ fixed a bug in breakpoint manager which may resolved ModuleName::APIName to wrong offset
+ fixed a bug in assembler which double loaded the gui
+ fixed a bug in hardware breakpoints which did not activate them in running processes
+ fixed a bug in hardware breakpoints which did not activate them on the current thread
+ fixed a bug where by detaching from a suspended process didn't resume the process
+ fixed a bug which did not handle hardware breakpoints for wow64 targets
+ fixed a bug which showed a wrong menu if child processes where present in the debugging session
+ fixed a bug which reloaded the disassembler to the wrong offset after adding a new patch
+ fixed handling of "call * ptr []" and "jmp * ptr []"
+ fixed some handle and memory leaks
+ added saving of input in goto dialog 
+ added support of functions in goto dialog
+ added different hotkeys see notes for more details
+ added type column in attach dialog
+ added state update when doing a trace
+ added trace to selected disassembly line
+ added toggle breakpoint on selected disassembly line to context menu
+ updated to qt 4.8.5
+ updated nasm to 2.10.09
+ updated file open dialog to remove annoying messagebox for commandline
+ updated the internal pe handling
+ updated resizeevent of Disassembler and Stack
+ updated Stack scroll

####Notes:
	- function in the goto dialog should look like this: "module::function"
	  e.g KERNEL32::IsDebuggerPresent
	- added hotkeys:
		- "escape" to assembler and goto dialog to close the window
		- "f5" to attach dialog to reload the processlist
		- "double click" in function widget to view a function in diassembler

###For the full changelog click [here](https://github.com/zer0fl4g/Nanomite/blob/master/changelog.md)