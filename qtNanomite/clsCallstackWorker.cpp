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
#include "clsMemManager.h"
#include "clsCallstackWorker.h"

#include "dbghelp.h"

#define THREAD_GETSET_CONTEXT	(0x0018) 

clsCallstackWorker::clsCallstackWorker()
{

}

clsCallstackWorker::~clsCallstackWorker()
{

}

void clsCallstackWorker::setCallstackData(callstackData processingData)
{
	m_processingData = processingData;
}

void clsCallstackWorker::run()
{
	HANDLE hThread = OpenThread(THREAD_GETSET_CONTEXT, false, m_processingData.threadID);
	PSYMBOL_INFOW pSymbol = (PSYMBOL_INFOW)malloc(sizeof(SYMBOL_INFOW) + MAX_PATH * 2);
	DWORD dwMaschineMode = NULL;
	quint64 dwDisplacement = NULL;
	LPVOID pContext;
	QList<callstackDisplay> callstackDisplayData;
	callstackDisplay newDisplayData = { 0 };
	STACKFRAME64 stackFr = { 0 };
	IMAGEHLP_LINEW64 imgSource = { 0 };
	IMAGEHLP_MODULEW64 imgMod = { 0 };

	stackFr.AddrPC.Mode = AddrModeFlat;
	stackFr.AddrFrame.Mode = AddrModeFlat;
	stackFr.AddrStack.Mode = AddrModeFlat;

#ifdef _AMD64_
	if(m_processingData.isWOW64)
	{
		dwMaschineMode = IMAGE_FILE_MACHINE_I386;
		WOW64_CONTEXT cTT = m_processingData.wowProcessContext;
		pContext = &cTT;

		stackFr.AddrPC.Offset = cTT.Eip;
		stackFr.AddrFrame.Offset = cTT.Ebp;
		stackFr.AddrStack.Offset = cTT.Esp;	
	}
	else
	{
		dwMaschineMode = IMAGE_FILE_MACHINE_AMD64;
		CONTEXT cTT = m_processingData.processContext;
		pContext = &cTT;

		stackFr.AddrPC.Offset = cTT.Rip;
		stackFr.AddrFrame.Offset = cTT.Rbp;
		stackFr.AddrStack.Offset = cTT.Rsp;	
	}
#else
	dwMaschineMode = IMAGE_FILE_MACHINE_I386;
	CONTEXT cTT = m_processingData.processContext;
	pContext = &cTT;

	stackFr.AddrPC.Offset = cTT.Eip;
	stackFr.AddrFrame.Offset = cTT.Ebp;
	stackFr.AddrStack.Offset = cTT.Esp;	

#endif
	
	do
	{
		if(!StackWalk64(dwMaschineMode, m_processingData.processHandle, hThread, &stackFr, pContext, NULL, SymFunctionTableAccess64, SymGetModuleBase64, 0))        
			break;

		memset(&imgSource, 0, sizeof(IMAGEHLP_LINEW64));
		imgSource.SizeOfStruct = sizeof(IMAGEHLP_LINEW64);

		newDisplayData.stackAddress		= stackFr.AddrStack.Offset;
		newDisplayData.currentOffset	= stackFr.AddrPC.Offset;
		newDisplayData.returnOffset		= stackFr.AddrReturn.Offset;


		memset(&imgMod, 0, sizeof(IMAGEHLP_MODULEW64));
		imgMod.SizeOfStruct = sizeof(IMAGEHLP_MODULEW64);
		memset(pSymbol, 0, sizeof(SYMBOL_INFOW) + MAX_PATH * 2);
		pSymbol->SizeOfStruct = sizeof(SYMBOL_INFOW);
		pSymbol->MaxNameLen = MAX_PATH;

		SymGetModuleInfoW64(m_processingData.processHandle, newDisplayData.currentOffset, &imgMod);
		SymFromAddrW(m_processingData.processHandle, newDisplayData.currentOffset, &dwDisplacement, pSymbol);
		newDisplayData.currentFunctionName = QString::fromWCharArray(pSymbol->Name);
		newDisplayData.currentModuleName = QString::fromWCharArray(imgMod.ModuleName);


		memset(&imgMod, 0, sizeof(IMAGEHLP_MODULEW64));
		imgMod.SizeOfStruct = sizeof(IMAGEHLP_MODULEW64);
		memset(pSymbol, 0, sizeof(SYMBOL_INFOW) + MAX_PATH * 2);
		pSymbol->SizeOfStruct = sizeof(SYMBOL_INFOW);
		pSymbol->MaxNameLen = MAX_PATH;

		SymGetModuleInfoW64(m_processingData.processHandle, newDisplayData.returnOffset, &imgMod);
		SymFromAddrW(m_processingData.processHandle, newDisplayData.returnOffset , &dwDisplacement, pSymbol);
		newDisplayData.returnModuleName = QString::fromWCharArray(imgMod.ModuleName);
		newDisplayData.returnFunctionName = QString::fromWCharArray(pSymbol->Name);
		
		if(SymGetLineFromAddrW64(m_processingData.processHandle, newDisplayData.currentOffset, (PDWORD)&dwDisplacement, &imgSource))
		{

			newDisplayData.sourceFilePath		= QString::fromWCharArray(imgSource.FileName);
			newDisplayData.sourceLineNumber		= imgSource.LineNumber;
		}
		else
		{
			newDisplayData.sourceFilePath		= "";
			newDisplayData.sourceLineNumber		= 0;
		}

		callstackDisplayData.append(newDisplayData);

	}while(stackFr.AddrReturn.Offset != 0);

	free(pSymbol);
	CloseHandle(hThread);

	emit OnCallstackFinished(callstackDisplayData);
}