#include "clsDisassembler.h"
#include "clsAPIImport.h"

#include <TlHelp32.h>

#define BEA_ENGINE_STATIC
#define BEA_USE_STDCALL
#include "BeaEngine.h"

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
							case MEM_MAPPED: return true;    break;
						}
						break;
					}
			}
		}

		//if(_dwEIP >= dwBaseBegin && _dwEIP <= dwBaseEnd)
		//{
		//	if(_dwStartOffset >= dwBaseBegin && _dwEndOffset <= dwBaseEnd)
		//	{
		//		_bEndOfSection = false;
		//		_bStartOfSection = false;
		//		return true;
		//	}
		//	else if(_dwStartOffset >= dwBaseBegin && _dwEndOffset > dwBaseEnd)
		//	{
		//		_bStartOfSection = false;

		//		if(!_bEndOfSection)
		//		{
		//			_bEndOfSection = true;
		//			_dwEndOffset = dwBaseEnd;
		//			return true;
		//		}
		//		return false;
		//	}
		//	else if(_dwStartOffset < dwBaseBegin && _dwEndOffset <= dwBaseEnd)
		//	{
		//		_bEndOfSection = false;

		//		if(!_bStartOfSection)
		//		{
		//			_bStartOfSection = true;
		//			_dwStartOffset = dwBaseBegin;
		//			return true;
		//		}
		//		return false;
		//	}
		//}
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

void clsDisassembler::SyntaxHighLight(QTableWidgetItem *newItem)
{

}

void clsDisassembler::run()
{
	if(_dwStartOffset == 0 || _dwEndOffset == 0)
		return;

	quint64 dwSize = _dwEndOffset - _dwStartOffset;
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
	else
	{
		free(pBuffer);
		MessageBox(NULL,L"Access Denied! Can´t disassemble this buffer :(",L"Nanomite",MB_OK);
		return;
	}

	bool bProtect = VirtualProtectEx(_hProc,(LPVOID)_dwStartOffset,dwSize,dwOldProtection,NULL);
	free(pBuffer);
	
	QMap<QString,DisAsDataRow>::iterator iEnd = SectionDisAs.end();iEnd--;
	_dwEndOffset = iEnd.key().toULongLong(0,16);

	emit DisAsFinished(_dwEIP);
}