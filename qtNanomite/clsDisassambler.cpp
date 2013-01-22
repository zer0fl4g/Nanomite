#include "clsDisassambler.h"
#include "clsAPIImport.h"

#include <TlHelp32.h>

#define BEA_ENGINE_STATIC
#define BEA_USE_STDCALL
#include "BeaEngine.h"

clsDisassambler::clsDisassambler()
{

}

clsDisassambler::~clsDisassambler()
{

}

bool clsDisassambler::CalcIfNewInsertIsNeeded()
{
	DWORD64 dwStartOffset = NULL,dwEndOffset = 0;
	if(!CalcMemoryRange(&dwStartOffset,&dwEndOffset))
		return false;

	if(_dwStartOffset != dwStartOffset &&
		_dwEndOffset != dwEndOffset)
	{
		_dwStartOffset = dwStartOffset;
		_dwEndOffset = dwEndOffset;
		return true;
	}
	return false;
}

bool clsDisassambler::InsertNewDisassambly(HANDLE hProc,DWORD64 dwEIP)
{
	if(_hProc == INVALID_HANDLE_VALUE && dwEIP == NULL)
		return false;

	_dwEIP = dwEIP;
	_hProc = hProc;

	if(CalcIfNewInsertIsNeeded())
	{
		SectionDisAs.clear();
		this->start();
		return true;
	}
	else 
		return false;
}

void clsDisassambler::SyntaxHighLight(QTableWidgetItem *newItem)
{

}

bool clsDisassambler::CalcMemoryRange(DWORD64 *StartOffset,DWORD64 *EndOffset)
{
	MODULEENTRY32 pModEntry;
	pModEntry.dwSize = sizeof(MODULEENTRY32);
	MEMORY_BASIC_INFORMATION mbi;

	DWORD64 dwAddress = NULL;
	while(VirtualQueryEx((HANDLE)_hProc,(LPVOID)dwAddress,&mbi,sizeof(mbi)))
	{
		if(_dwEIP >= (DWORD64)mbi.BaseAddress && _dwEIP <= ((DWORD64)mbi.BaseAddress + mbi.RegionSize))
		{
			*StartOffset = (DWORD64)mbi.BaseAddress;
			*EndOffset = (DWORD64)mbi.RegionSize + (DWORD64)mbi.BaseAddress;
			return true;
		}
		dwAddress += mbi.RegionSize;
	}
	return false;
}

void clsDisassambler::run()
{
	if(_dwStartOffset == 0 || _dwEndOffset == 0)
		return;

	DWORD64 dwSize = _dwEndOffset - _dwStartOffset;
	DWORD	dwOldProtection = 0,
			dwNewProtection = PAGE_EXECUTE_READWRITE;
	LPVOID pBuffer = malloc(dwSize);

	if(VirtualProtectEx(_hProc,(LPVOID)_dwStartOffset,dwSize,dwNewProtection,&dwOldProtection) &&
		ReadProcessMemory(_hProc,(LPVOID)_dwStartOffset,pBuffer,dwSize,NULL))
	{
		DISASM newDisAss;
		bool bContinueDisAs = true;
		int iLen = 0;
		DisAsDataRow newRow;
		BYTE bBuffer;
	
		memset(&newDisAss, 0, sizeof(DISASM));

		newDisAss.EIP = (DWORD64)pBuffer;
		newDisAss.VirtualAddr = _dwStartOffset;
#ifdef _AMD64_
		newDisAss.Archi = 64;
#else
		newDisAss.Archi = 0;
#endif

		PTCHAR sTemp = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));
		while(bContinueDisAs)
		{
			newDisAss.SecurityBlock = (int)_dwEndOffset - newDisAss.VirtualAddr;

			iLen = Disasm(&newDisAss);

			if (iLen == OUT_OF_BLOCK)
				bContinueDisAs = false;
			else
			{			
				memset(sTemp,0,MAX_PATH *  sizeof(TCHAR));
				
				// OpCodez
				int iTempLen = ((newDisAss.Instruction.Opcode == 0x00 && iLen == 2) ? 1 : ((iLen == UNKNOWN_OPCODE) ? 0 : iLen));
				for(size_t i = 0;i < iTempLen;i++)
				{
					memcpy(&bBuffer,(LPVOID)((DWORD64)newDisAss.EIP + i),1);
					wsprintf(sTemp,L"%s %02X",sTemp,bBuffer);
				}
				newRow.OpCodes = QString::fromWCharArray(sTemp);

				// Instruction
				if(newDisAss.Instruction.Opcode == 0x00 && iLen == 2)
					wsprintf(sTemp,L"%s",L"db 00");
				else
					wsprintf(sTemp,L"%S",newDisAss.CompleteInstr);	
				newRow.ASM = QString::fromWCharArray(sTemp);

				// Comment
				//memset(sTemp,NULL,MAX_PATH * sizeof(TCHAR));
				//if(strstr(newDisAss.Instruction.Mnemonic,"call") != 0 ||
				//	strstr(newDisAss.Instruction.Mnemonic,"jmp") != 0 ||
				//	strstr(newDisAss.Instruction.Mnemonic,"push") != 0 ||
				//	strstr(newDisAss.Instruction.Mnemonic,"jnz") != 0 ||
				//	strstr(newDisAss.Instruction.Mnemonic,"je") != 0 ||
				//	strstr(newDisAss.Instruction.Mnemonic,"jl") != 0 ||
				//	strstr(newDisAss.Instruction.Mnemonic,"jng") != 0 ||
				//	strstr(newDisAss.Instruction.Mnemonic,"jne") != 0)
				//{
					//wstring sFuncName,sModName;
					//coreDebugger->LoadSymbolForAddr(sFuncName,sModName,newDisAss.Instruction.AddrValue);
					//if(sFuncName.length() > 0 && sModName.length() > 0)
					//	wsprintf(sTemp,L"%s.%s",sModName.c_str(),sFuncName.c_str());
					//else if(sModName.length() > 0 && sFuncName.length() == 0)
					//	wsprintf(sTemp,L"%s.0x%08X",sModName.c_str(),newDisAss.VirtualAddr);
				//}
				//else
				//{
					//if(newDisAss.Instruction.Opcode != 0xCC &&
					//	newDisAss.Instruction.Opcode != 0x90)
					//{
					//	wstring sFuncName,sModName;
					//	coreDebugger->LoadSymbolForAddr(sFuncName,sModName,newDisAss.VirtualAddr);
					//	if(sFuncName.length() > 0)
					//		wsprintf(sTemp,L"%s",sFuncName.c_str());
					//	else 
					//		wsprintf(sTemp,L"%s",L"");
					//}
					//else
				//		wsprintf(sTemp,L"%s",L"");
				//}

				//strTemp = QString::fromWCharArray(sTemp);
				//tblDisAs->setItem(itemIndex - 1,3,new QTableWidgetItem(strTemp));

				swprintf(sTemp,L"%016I64X",newDisAss.VirtualAddr);
				newRow.Offset = QString::fromWCharArray(sTemp);
				SectionDisAs.insert(QString::fromWCharArray(sTemp),newRow);
			}

			newDisAss.EIP = newDisAss.EIP + ((iLen == UNKNOWN_OPCODE) ? 1 : ((newDisAss.Instruction.Opcode == 0x00 && iLen == 2) ? iLen -= 1 : iLen));
			newDisAss.VirtualAddr = newDisAss.VirtualAddr + ((iLen == UNKNOWN_OPCODE) ? 1 : ((newDisAss.Instruction.Opcode == 0x00 && iLen == 2) ? iLen -= 1 : iLen));
			if (newDisAss.VirtualAddr >= _dwEndOffset)
				bContinueDisAs = false;
		}
		free(sTemp);
	}

	bool bProtect = VirtualProtectEx(_hProc,(LPVOID)_dwStartOffset,dwSize,dwOldProtection,NULL);
	free(pBuffer);
	
	emit DisAsFinished(_dwEIP);
}