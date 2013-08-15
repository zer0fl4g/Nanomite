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
- STRG + C can be used to copy data from selected field in table - for more check context menu
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
	+ Double Click	= Send offset to disassembler
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
- Trace Window
	+ Double Click	= Send offset to disassembler

## Changelog
###Version 0.1 beta 15
+ fixed a bug which did not reload the gui when deleting a patch from patchmanager using hotkey
+ fixed a bug which caused a break when continue was used after a trace
+ fixed a bug which returned wrong offset when adding a breakpoint to a wow64 process
+ fixed a bug which did not clean up properly if using the "recent file" menu to debug new process
+ added save file dialog to memory dump
+ added double click handler in trace view and bp manager to send a offset to disassembler window
+ added possibility to create a full process dump in detailview - process tab - context menu
+ updated function view algorithm
+ updated winapi messagebox to qt

###For the full changelog click [here](https://github.com/zer0fl4g/Nanomite/blob/master/changelog.md)