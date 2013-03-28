#ifndef CLSMEMDUMP_H
#define CLSMEMDUMP_H

#include <Windows.h>

class clsMemDump
{
public:
	clsMemDump::clsMemDump(HANDLE hProc, PTCHAR FileName, DWORD64 BaseOffset, DWORD64 Size);
	clsMemDump::~clsMemDump();
};

#endif