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
#include "clsDisassembler.h"
#include "clsAPIImport.h"
#include "clsMemManager.h"
#include "clsHelperClass.h"
#include "clsSymbolAndSyntax.h"

#define BEA_ENGINE_STATIC
#define BEA_USE_STDCALL
#include <BeaEngine.h>

clsDisassembler::clsDisassembler()
{
	m_startOffset = 0;m_endOffset = 0;m_startPage = 0;
	m_isWorking = false;

	connect(this,SIGNAL(finished()),this,SLOT(OnThreadFinished()),Qt::QueuedConnection);
}

clsDisassembler::~clsDisassembler()
{

}

bool clsDisassembler::InsertNewDisassembly(HANDLE hProc,quint64 dwEIP,bool bClear)
{
	if(hProc == INVALID_HANDLE_VALUE || hProc == NULL|| dwEIP == NULL || m_isWorking)
		return false;

	if(m_searchedOffset == dwEIP && !bClear && SectionDisAs.count() > 0)
		return false;

	if(bClear) 
		SectionDisAs.clear();

	m_isWorking = true;
	m_searchedOffset = dwEIP;
	m_processHandle = hProc;

	if(IsNewInsertNeeded())
	{
		SectionDisAs.clear();
		this->start();
		return true;
	}
	
	m_isWorking = false;
	return false;
}

bool clsDisassembler::IsNewInsertNeeded()
{
	if(m_searchedOffset <= m_startOffset || m_searchedOffset >= m_endOffset || SectionDisAs.count() <= 0)
		return IsNewInsertPossible();

	return false;
}

bool clsDisassembler::IsNewInsertPossible()
{
	quint64 PageBase = NULL,
			PageEnd	= NULL;

	if(!GetPageRangeForOffset(m_searchedOffset, PageBase, PageEnd))
		return false;

	if((m_searchedOffset - 300) <= PageBase)
	{
		quint64 PageBaseBelow = NULL,
				PageEndBelow = NULL;

		if(GetPageRangeForOffset(m_searchedOffset - 300, PageBaseBelow, PageEndBelow))
		{
			m_startOffset = m_searchedOffset - 300;
			m_startPage = PageBaseBelow;
		}
		else
			m_startPage = m_startOffset = PageBase;
	}
	else
	{
		m_startOffset = m_searchedOffset - 300;
		m_startPage = PageBase;
	}

	if((m_searchedOffset + 300) >= PageEnd)
	{
		quint64 PageBaseAbove = NULL,
				PageEndAbove = NULL;

		if(GetPageRangeForOffset(m_searchedOffset + 300, PageBaseAbove, PageEndAbove))
			m_endOffset = m_searchedOffset + 300;
		else
			m_endOffset = PageEnd;
	}
	else
		m_endOffset = m_searchedOffset + 300;

	return true;
}

void clsDisassembler::run()
{
	if(m_startOffset == 0 || m_endOffset == 0 || m_startPage == 0)
		return;

	quint64 dwSize = m_endOffset - m_startPage;
	LPVOID pBuffer = malloc(dwSize);
	clsSymbolAndSyntax DataVisualizer(m_processHandle);

	clsBreakpointManager::RemoveSBPFromMemory(true, GetProcessId(m_processHandle));

	if(ReadProcessMemory(m_processHandle,(LPVOID)m_startPage,pBuffer,dwSize,NULL))
	{
		clsBreakpointManager::RemoveSBPFromMemory(false, GetProcessId(m_processHandle));

		DISASM newDisAss;
		bool bContinueDisAs = true;
		int iLen = 0;
		DisAsDataRow newRow;
		BYTE bBuffer;

		memset(&newDisAss, 0, sizeof(DISASM));

		newDisAss.EIP = (quint64)pBuffer;
		newDisAss.VirtualAddr = m_startPage;
#ifdef _AMD64_
		newDisAss.Archi = 64;
#else
		newDisAss.Archi = 0;
#endif

		PTCHAR sTemp = (PTCHAR)clsMemManager::CAlloc(64 * sizeof(TCHAR));
		while(bContinueDisAs)
		{
			newDisAss.SecurityBlock = (int)m_endOffset - newDisAss.VirtualAddr;

			iLen = Disasm(&newDisAss);
			if (iLen == OUT_OF_BLOCK)
				bContinueDisAs = false;
			else if(iLen == UNKNOWN_OPCODE)
				iLen = 1;
			else if(newDisAss.VirtualAddr >= m_startOffset && newDisAss.VirtualAddr <= m_endOffset)
			{	
				// OpCodez
				if(newDisAss.Instruction.Opcode == 0x00 && iLen == 2)
					iLen = 1;

				if(iLen > 0) 
				{
					memset(sTemp,0,64 *  sizeof(TCHAR));
					
					for(size_t i = 0;i < iLen;i++)
					{
						memcpy(&bBuffer, (LPVOID)((quint64)newDisAss.EIP + i), 1);
						wsprintf(sTemp, L"%s %02X", sTemp, bBuffer);
					}
					newRow.OpCodes = QString::fromWCharArray(sTemp);

					// Instruction
					wsprintf(sTemp, L"%S", newDisAss.CompleteInstr);	
					newRow.ASM = QString::fromWCharArray(sTemp);
				
					// Comment/Symbol && itemStyle		
					DataVisualizer.CreateDataForRow(&newRow);

					newRow.Offset = QString("%1").arg(newDisAss.VirtualAddr,16,16,QChar('0')).toUpper();
					SectionDisAs.insert(newRow.Offset,newRow);
				}
			}

			newDisAss.EIP += iLen;
			newDisAss.VirtualAddr += iLen;
			

			if (newDisAss.VirtualAddr >= m_endOffset)
				bContinueDisAs = false;
		}
		clsMemManager::CFree(sTemp);
	}
	else
	{
		clsBreakpointManager::RemoveSBPFromMemory(false, GetProcessId(m_processHandle));
		free(pBuffer);
		MessageBox(NULL,L"Access Denied! Can not disassemble this buffer :(",L"Nanomite",MB_OK);
		return;
	}
	
	free(pBuffer);

	if(SectionDisAs.count() > 0)
	{
		QMap<QString,DisAsDataRow>::iterator iEnd = SectionDisAs.end();iEnd--;
		m_endOffset = iEnd.key().toULongLong(0,16);
		m_startOffset = SectionDisAs.begin().value().Offset.toULongLong(0,16);
	}
	else
	{
		m_endOffset = 0;
		m_startOffset = 0;
	}

	emit DisAsFinished(m_searchedOffset);
}

bool clsDisassembler::GetPageRangeForOffset(quint64 IP, quint64 &PageBase, quint64 &PageEnd)
{
	MEMORY_BASIC_INFORMATION mbi;
	quint64 dwAddress = IP;
	
	if(VirtualQueryEx(m_processHandle,(LPVOID)dwAddress,&mbi,sizeof(mbi)))
	{		
		switch (mbi.State)
		{
			case MEM_COMMIT:
				{
					switch (mbi.Type)
					{
						case MEM_IMAGE:		
						case MEM_MAPPED:
							if((mbi.Protect & PAGE_EXECUTE) ||
								(mbi.Protect & PAGE_EXECUTE_READ) ||
								(mbi.Protect & PAGE_EXECUTE_READWRITE) ||
								(mbi.Protect & PAGE_EXECUTE_WRITECOPY))
							{
								PageBase = (quint64)mbi.BaseAddress;
								PageEnd = (quint64)mbi.BaseAddress + mbi.RegionSize;

								return true;
							}
							break;
					}
					break;
				}
		}
	}

	return false;
}

void clsDisassembler::OnThreadFinished()
{
	m_isWorking = false;
}