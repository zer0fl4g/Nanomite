#Nanomite - Graphical Debugger for x64 and x86 on Windows

## Changelog
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