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

### Hotkeys
- most windows can be closed with "esc"
- most windows can be reloaded with "F5"
- STRG + C can be used to copy data from selected field in tables
- MainWindow
	+ STRG + B		= Breakpoint Manager
	+ STRG + O		= Open new file
	+ STRG + R		= Restart debugging
	+ STRG + I		= Show detail info window
	+ STRG + T		= Show trace window
	+ STRG + P		= Show patch window
	+ STRG + A		= Show attach window
	+ STRG + S		= Stop debugging
	+ STRG + D		= Detach from debuggee
	+ ALT + 1		= Show memory window
	+ ALT + 2		= Show heap window
	+ ALT + 3		= Show string window
	+ ALT + 4		= Show debug output window
	+ ALT + 5		= Show handle window
	+ ALT + 6		= Show windows window
	+ ALT + 7		= Show peeditor window
	+ ALT + 8		= Show function window
	+ ALT + 9		= Show process privilege window
	+ ALT			= Select menu bar and navigate with arrows
	+ F6			= Step Out
	+ F7			= Step In
	+ F8			= Step Over
	+ F9			= Start/Continue debugging
	+ F12			= Options
	+ Break			= Suspend debugging
- Attach Dialog
	+ Return		= Attach to selected process
- Breakpoint Manager
	+ Del			= Remove selected breakpoint
- Disassembler Window
	+ F2			= Set software breakpoint to selected line
	+ Return		= Follow selected jump, call
	+ Backspace		= Go back after goto/follow
	+ Escape		= Close window
- Function Window
	+ Double Click	= Show selected function in diassembler
	+ Return		= Show selected function in diassembler
- Options
	+ Del			= Remove selected custom exception
- Patch Manager
	+ Del			= Remove selected patch
- Register Window
	+ Double Click	= Opens reg edit window

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
+ fixed paths in attach dialog with SystemRoot enviroment string
+ fixed handling of "call * ptr []" and "jmp * ptr []"
+ fixed some handle and memory leaks
+ added saving of input in goto dialog 
+ added support of functions in goto dialog
+ added different hotkeys see hotkey list for all of them
+ added type column in attach dialog
+ added state update when doing a trace
+ added trace to selected disassembly line
+ added toggle breakpoint on selected disassembly line to context menu
+ added display of FPU, MMX and SSE register
+ updated to qt 4.8.5
+ updated nasm to 2.10.09
+ updated file open dialog to remove annoying messagebox for commandline
+ updated the internal pe handling
+ updated resize event of Disassembler and Stack
+ updated Stack scroll
+ updated PID dropdown to be only displayed if more then 1 process is running
+ updated disassembler logic

####Notes:
	- function in the goto dialog should look like this: "module::function"
	  e.g KERNEL32::IsDebuggerPresent

###For the full changelog click [here](https://github.com/zer0fl4g/Nanomite/blob/master/changelog.md)