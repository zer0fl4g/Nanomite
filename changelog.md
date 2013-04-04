#Nanomite - Graphical Debugger for x64 and x86 on Windows

## Changelog
###Version 0.1 beta 10
+ fixed display of wrong function offset in callstack
+ fixed a bug which didn´t break on EP if "break on system ep" was selected
+ added display of current PID/TID in mainwindow title

###Version 0.1 beta 9
+ fixed a bug in disassembler
+ fixed a bug in wow64 StepIn
+ fixed a crash when suspending a process and then StepIn
+ fixed a bug which didn´t display all modules in callstack
+ fixed a crash in loading imports of files without IAT
+ fixed a crash in "Goto Offset" context menu
+ small gui improvements
+ added Single Step Tracer
+ added memory dumper
+ removed error message if you cancel the file selection

####Notes:
	- Single Step Tracer
		- only a part is displayed in the window. Use mouse scroll to navigate (will be improved)
	- MemoryDumper
		- RightClick in MemoryView or HeapView shows you the option to dump the selected segment.

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

###Version 0.1 beta 7
+ fixed some small handling bugs
+ fixed a bug in disassembler which did not replace old protection on memory after disassembling
+ fixed a bug which did not show terminated processes in DetailView
+ fixed a bug which did not show terminated threads in DetailView
+ fixed a bug which did not clean up memory on manual debugge stop
+ improved DB handler
+ added resolve of jump conditions to improve StepOver
+ added "Return" and "Backspace" Hotkey to navigate in Disassembler
+ added "Clear Log" context menu in LogBox
+ added "Show Source" context menu in Disassembler
+ added "Goto Function" context menu in Callstack
+ added a crash handler
+ added Source Viewer
+ added memory pool for performance improvment and memory leak reduction
+ added mouse scrolling in disassembler and stack
+ added direkt run of target after using menu to select a file

####Notes:
	- CrashHandler
		- if Nanomite crashs a dumpfile will be written to the application folder. 
		  Please send me this file via zer0fl4g[at]gmail[dot]com
	- Hotkey "Return"
		- when you selected a jump / call / ... you can follow this instruction using the "Return" key
	- Hotkey "Backspace"
		- steps back when you used "Return" to follow a call
	- Source Viewer 
		- double click on source line in Callstack view. A new Window will open and show the source code (if found)
		- right click in disassembler opens source view also
	- Memory Pool
		- redericted malloc / new / delete / free to the memory pool
		- heap fragmentation reduction
		- increasing performance

###Version 0.1 beta 6
+ fixed a crash in Step Over
+ fixed load of colors in option window
+ fixed a dead lock when using detach
+ fixed memory overhead in hexview
+ fixed a display issue of the time in log when the debugge finished
+ improved internal PEFile handling
+ added unload of symbols if a DLL gets unloaded during runtime
+ added some more instructions to syntax highlighter
+ added highlight of current EIP
+ added highlight of BPs
+ added possibility to remove BPs from BPManager
+ added auto completion for apis in BPManager
+ added DB Interface
+ added command line support

####Notes:	
	- BPManager
		-	Use the "DEL" Key to remove the entries from BPManager
		-	Type a module name and the box will propose you found apis that match your entry 
			e.g type "Ker" and the BPManager will show all imports of the processes found with Ker* -> Kernel32::*

###Version 0.1 beta 5
+ fixed missing registers in x64 RegView
+ improved entrypoint handling
+ improved the BPManager
+ added some hotkeys
+ added Step Over
+ added refill on mainwindow resize to match size
+ added RegEdit
+ added basic coloring

####Notes:
	- Hotkeys:	STRG + O = open new file
				STRG + B = breakpoint manager
				STRG + F4 = stop debugging
				F12 = options
				F9 = start debugging / continue
				F8 = step over
				F7 = step in
				F2 = set software breakpoint on selected row (a row must be selected in Disassembler)
	
	- RegEdit:	Double click on the regview to open it
	- Colors:	Can be edited via Options Dialog (F12)

###Version 0.1 beta 4:
+ fixed different crashs in disassembler
+ fixed dependencies of cruntime
+ fixed the restart icon
+ fixed little bug in DetachFromProcess
+ improved speed and memory usage of disassembler
+ added a check for valid file
+ added a check for admin rights + warning
+ added right click menu in RegView (send to Disassembler)
+ added right click menu in Disassembler (Goto Offset) 
+ added possibility to resize and maximize the mainwindow
+ changed window style to Qt Plastique

####Notes:
- dependencies:
	- For developers:	You will need a QT Framework which has been compiled with /MT ( or /MTd) else you
						have a dependencie of the cruntime even if qtNanomite has been compiled without.
						If you need help to compile your QT this way just drop me an Email / PM.
	- For all:			I will place the needed QT Dlls into the repro and you shouldn´t need the cruntime to be installed anymore.


###Version 0.1 beta 3:
+ fixed a bug which displayed crap on some x64 Addresses
+ fixed a crash in the Breakpoint Manager
+ fixed RegView for Wow64
+ added dynamic load of Wow64 APIs (first step to XP64)
+ added right click menu in HeapView (send to HexView)
+ added right click menu in MemoryView (send to HexView)
+ added resizability to the different sub windows
+ added dynamic row calc to stack view (prepare for dynamic main window)
+ added own class and thread for disassembler


###Version 0.1 beta 2:
+ Ported to QT 4.8.4
+ Added possibility to ignore custom exceptions in options dialog
+ Added possibility to reload a default config in options dialog
+ Fixed a bug in the detach function
+ Fixed a crash in CleanWorkSpace
+ Improved Breakpoint Manager