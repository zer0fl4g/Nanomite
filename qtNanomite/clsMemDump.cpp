/*
 * 	This file is part of Nanomite.
 *
 *    Nanomite is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    Nanomite is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with Nanomite.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "clsMemDump.h"
#include "clsMemManager.h"

clsMemDump::clsMemDump(HANDLE hProc, PTCHAR FileBaseName, DWORD64 BaseOffset, DWORD Size)
{
	bool	isProtectionChanged = false;
	DWORD	OldProtection	= NULL,
			NewProtection	= PAGE_READWRITE,
			BytesWrote		= NULL;
	SIZE_T	BytesReaded		= NULL;
	LPVOID pBuffer			= malloc(Size);

	if(!ReadProcessMemory(hProc,(LPVOID)BaseOffset,pBuffer,Size,&BytesReaded))
	{
		if(!VirtualProtectEx(hProc,(LPVOID)BaseOffset,Size,NewProtection,&OldProtection))
		{
			MessageBoxW(NULL,L"Failed to access Memory!",L"Nanomite",MB_OK);
			free(pBuffer);
			return;
		}
		isProtectionChanged = true;

		if(!ReadProcessMemory(hProc,(LPVOID)BaseOffset,pBuffer,Size,&BytesReaded))
		{
			MessageBoxW(NULL,L"Failed to read Memory!",L"Nanomite",MB_OK);
			free(pBuffer);
			return;
		}
	}
	
	PTCHAR FileName = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));
	if(wcslen(FileBaseName) <= 0)
		wsprintf(FileName,L"NANOMITEDUMP_%016I64X-%016I64X_%08X.bin",BaseOffset,BaseOffset + Size,Size);
	else
		wsprintf(FileName,L"%s_%016I64X-%016I64X_%08X.bin",FileBaseName,BaseOffset,BaseOffset + Size,Size);

	HANDLE hFile = CreateFile(FileName, GENERIC_READ | GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		MessageBoxW(NULL,L"Failed to create File!",L"Nanomite",MB_OK);
		free(FileName);
		free(pBuffer);
		return;
	}

	if(!WriteFile(hFile,pBuffer,Size,&BytesWrote,NULL))
		MessageBoxW(NULL,L"Failed to write to File!",L"Nanomite",MB_OK);

	free(FileName);
	free(pBuffer);
	CloseHandle(hFile);

	if(isProtectionChanged && !VirtualProtectEx(hProc,(LPVOID)BaseOffset,Size,OldProtection,&NewProtection))
		MessageBoxW(NULL,L"Failed to reprotect the Memory!",L"Nanomite",MB_OK);
	
	MessageBoxW(NULL,L"Memory Dump finished!",L"Nanomite",MB_OK);
}

clsMemDump::~clsMemDump()
{

}