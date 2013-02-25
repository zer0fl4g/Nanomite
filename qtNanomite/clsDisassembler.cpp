#include "clsDisassembler.h"
#include "clsAPIImport.h"
#include "clsHelperClass.h"
#include "clsSymbolAndSyntax.h"

#include <string>
#include <TlHelp32.h>

#define BEA_ENGINE_STATIC
#define BEA_USE_STDCALL
#include "BeaEngine.h"

using namespace std;

clsDisassembler::clsDisassembler()
{
	_dwStartOffset = 0;_dwEndOffset = 0;_bEndOfSection = false;_bStartOfSection = false;
}

clsDisassembler::~clsDisassembler()
{

}

bool clsDisassembler::IsNewInsertNeeded()
{
	if(_dwEIP <= _dwStartOffset || _dwEIP >= _dwEndOffset || SectionDisAs.count() <= 0)
	{
		_dwStartOffset = _dwEIP - 250;
		_dwEndOffset = _dwEIP + 250;

		return IsNewInsertPossible();
	}
	return false;
}

bool clsDisassembler::IsNewInsertPossible()
{
	MODULEENTRY32 pModEntry;
	pModEntry.dwSize = sizeof(MODULEENTRY32);
	MEMORY_BASIC_INFORMATION mbi;

	quint64 dwAddress = NULL;
	while(VirtualQueryEx(_hProc,(LPVOID)dwAddress,&mbi,sizeof(mbi)))
	{
		quint64 dwBaseBegin = (quint64)mbi.BaseAddress,
				dwBaseEnd	= ((quint64)mbi.BaseAddress + mbi.RegionSize);

		if(_dwEIP >= dwBaseBegin && _dwEIP <= dwBaseEnd)
		{
			switch (mbi.State)
			{
				case MEM_COMMIT:
					{
						switch (mbi.Type)
						{
							case MEM_IMAGE:		
							case MEM_MAPPED:
								return true;
								break;
						}
						break;
					}
			}
		}
		dwAddress += mbi.RegionSize;
	}
	return false;
}

bool clsDisassembler::InsertNewDisassembly(HANDLE hProc,quint64 dwEIP)
{
	if(_hProc == INVALID_HANDLE_VALUE || dwEIP == NULL)
		return false;

	_dwEIP = dwEIP;
	_hProc = hProc;

	if(IsNewInsertNeeded())
	{
		SectionDisAs.clear();
		this->start();
		return true;
	}
	else 
		return false;
}

void clsDisassembler::run()
{
	if(_dwStartOffset == 0 || _dwEndOffset == 0)
		return;

	quint64 dwSize = _dwEndOffset - _dwStartOffset;
	DWORD	dwOldProtection = 0,
			dwNewProtection = PAGE_EXECUTE_READWRITE;
	LPVOID pBuffer = malloc(dwSize);
	clsSymbolAndSyntax DataVisualizer(_hProc);

	if(VirtualProtectEx(_hProc,(LPVOID)_dwStartOffset,dwSize,dwNewProtection,&dwOldProtection) &&
		ReadProcessMemory(_hProc,(LPVOID)_dwStartOffset,pBuffer,dwSize,NULL))
	{
		DISASM newDisAss;
		bool bContinueDisAs = true;
		int iLen = 0;
		DisAsDataRow newRow;
		BYTE bBuffer;
		wstring sFuncName,sModName;

		memset(&newDisAss, 0, sizeof(DISASM));

		newDisAss.EIP = (quint64)pBuffer;
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
					memcpy(&bBuffer,(LPVOID)((quint64)newDisAss.EIP + i),1);
					wsprintf(sTemp,L"%s %02X",sTemp,bBuffer);
				}
				newRow.OpCodes = QString::fromWCharArray(sTemp);

				// Instruction
				if(newDisAss.Instruction.Opcode == 0x00 && iLen == 2)
					wsprintf(sTemp,L"%s",L"db 00");
				else
					wsprintf(sTemp,L"%S",newDisAss.CompleteInstr);	
				newRow.ASM = QString::fromWCharArray(sTemp);
				
				// Comment/Symbol && itemStyle		
				DataVisualizer.CreateDataForRow(&newRow);

				newRow.Offset = QString("%1").arg(newDisAss.VirtualAddr,16,16,QChar('0')).toUpper();
				SectionDisAs.insert(newRow.Offset,newRow);
			}

			newDisAss.EIP = newDisAss.EIP + ((iLen == UNKNOWN_OPCODE) ? 1 : ((newDisAss.Instruction.Opcode == 0x00 && iLen == 2) ? iLen -= 1 : iLen));
			newDisAss.VirtualAddr = newDisAss.VirtualAddr + ((iLen == UNKNOWN_OPCODE) ? 1 : ((newDisAss.Instruction.Opcode == 0x00 && iLen == 2) ? iLen -= 1 : iLen));
			if (newDisAss.VirtualAddr >= _dwEndOffset)
				bContinueDisAs = false;
		}
		free(sTemp);
	}
	else
	{
		free(pBuffer);
		MessageBox(NULL,L"Access Denied! Can´t disassemble this buffer :(",L"Nanomite",MB_OK);
		return;
	}

	bool bProtect = VirtualProtectEx(_hProc,(LPVOID)_dwStartOffset,dwSize,dwOldProtection,&dwNewProtection);
	free(pBuffer);

	QMap<QString,DisAsDataRow>::iterator iEnd = SectionDisAs.end();iEnd--;
	_dwEndOffset = iEnd.key().toULongLong(0,16);

	emit DisAsFinished(_dwEIP);
}