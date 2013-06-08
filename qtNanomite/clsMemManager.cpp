/* 
 *  This file is part of Nanomite. 
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
#include <new> 
  
clsMemManager* clsMemManager::pThis = NULL; 

#ifdef MEMLEAKTRACE
bool clsMemManager::IsInternalAlloc = false;
#endif

clsMemManager::clsMemManager() 
{     
    pThis = this; 
	
#ifdef MEMLEAKTRACE
	InitializeCriticalSection(&CriticalSection);
#endif

    // Create a 5000x200Byte Pool 
    PoolUnitCount_200 = 5000; 
    PoolUnitSize_200 = 200; 
    pPool_200 = new clsMemPool(PoolBufferBase_200,PoolBufferSize_200,PoolUnitCount_200,PoolUnitSize_200); 
    PoolBufferEnd_200 = PoolBufferBase_200 + PoolBufferSize_200; 
  
    // Create a 75000x50Byte Pool 
    PoolUnitCount_50 = 75000; 
    PoolUnitSize_50 = 50; 
    pPool_50 = new clsMemPool(PoolBufferBase_50,PoolBufferSize_50,PoolUnitCount_50,PoolUnitSize_50); 
    PoolBufferEnd_50 = PoolBufferBase_50 + PoolBufferSize_50; 
}  
  
clsMemManager::~clsMemManager() 
{ 
#ifdef MEMLEAKTRACE 
    printRest();
    allocList.clear(); 
#endif

    delete pPool_200; 
    delete pPool_50; 

#ifdef MEMLEAKTRACE
	DeleteCriticalSection(&CriticalSection);
#endif
} 
  
void* clsMemManager::Alloc(size_t ulSize) 
{ 
    if(ulSize <= PoolUnitSize_200 && ulSize > PoolUnitSize_50) // huge pool 
        return pPool_200->Alloc(ulSize); 
    else if(ulSize <= PoolUnitSize_50 && ulSize > 0) // little pool 
        return pPool_50->Alloc(ulSize); 
    else if(ulSize > PoolUnitSize_200) // alloc request too huge 
        return malloc(ulSize); 
    else // don´t use pool  
        return malloc(ulSize); 
} 
  
void clsMemManager::Free(void* p) 
{ 
    // is p from us? 
    if((DWORD64)p >= PoolBufferBase_200 && (DWORD64)p <= PoolBufferEnd_200) 
        pPool_200->Free(p); 
    else if((DWORD64)p >= PoolBufferBase_50 && (DWORD64)p <= PoolBufferEnd_50) 
        pPool_50->Free(p); 
    else 
        free(p); // no pass it to default 
} 
  
void clsMemManager::CFree(void* p) 
{ 
#ifdef MEMLEAKTRACE
	EnterCriticalSection(&pThis->CriticalSection);
	pThis->IsInternalAlloc = true;
    pThis->allocList.remove((DWORD64)p);
	pThis->IsInternalAlloc = false;
	LeaveCriticalSection(&pThis->CriticalSection);
#endif 
  
    pThis->Free(p); 
} 
  
void* clsMemManager::CAlloc(size_t ulSize) 
{ 
#ifdef MEMLEAKTRACE 
    LPVOID newAllocOffset = pThis->Alloc(ulSize); 

	EnterCriticalSection(&pThis->CriticalSection);
	pThis->IsInternalAlloc = true;
	//LeaveCriticalSection(&pThis->CriticalSection);

    allocdata newAlloc; 
      
    newAlloc.dataOffset = (DWORD64) newAllocOffset; 
    newAlloc.dataSize = ulSize; 
  
    HANDLE	hProc = GetCurrentProcess(),
			hThread = GetCurrentThread(); 
    PSYMBOL_INFOW pSymbol = (PSYMBOL_INFOW)pThis->Alloc(sizeof(SYMBOL_INFOW) + MAX_SYM_NAME); 
#ifdef _AMD64_
    DWORD dwMaschineMode = IMAGE_FILE_MACHINE_AMD64; 
#else
	DWORD dwMaschineMode = IMAGE_FILE_MACHINE_I386;
#endif
	LPVOID pContext; 
    STACKFRAME64 stackFr = {0}; 
	stackFr.AddrPC.Mode = AddrModeFlat; 
	stackFr.AddrFrame.Mode = AddrModeFlat; 
	stackFr.AddrStack.Mode = AddrModeFlat; 

	IMAGEHLP_LINEW64 imgSource = {0}; 
	IMAGEHLP_MODULEW64 imgMod = {0}; 
	std::wstring sFuncName, 
        sFuncMod;
    quint64 dwReturnTo, 
        dwEIP, 
        dwDisplacement; 
    BOOL bSuccess = SymInitialize(hProc,NULL,true); 
  
    CONTEXT context; 
    pContext = &context; 
    context.ContextFlags = CONTEXT_ALL; 
    RtlCaptureContext(&context); 
	
#ifdef _AMD64_
	stackFr.AddrPC.Offset = context.Rip; 
    stackFr.AddrFrame.Offset = context.Rbp; 
    stackFr.AddrStack.Offset = context.Rsp;  
#else
	stackFr.AddrPC.Offset = context.Eip; 
	stackFr.AddrFrame.Offset = context.Ebp; 
	stackFr.AddrStack.Offset = context.Esp;  
#endif  

	do
    { 
        bSuccess = StackWalk64(dwMaschineMode,hProc,hThread,&stackFr,pContext,NULL,SymFunctionTableAccess64,SymGetModuleBase64,0); 
  
        if(!bSuccess)         
            break; 
  
        dwEIP = stackFr.AddrPC.Offset; 
        dwReturnTo = stackFr.AddrReturn.Offset;   
  
        memset(&imgMod,0,sizeof(IMAGEHLP_MODULEW64)); 
        imgMod.SizeOfStruct = sizeof(IMAGEHLP_MODULEW64); 
        memset(pSymbol,0,sizeof(SYMBOL_INFOW) + MAX_SYM_NAME); 
        pSymbol->SizeOfStruct = sizeof(SYMBOL_INFOW); 
        pSymbol->MaxNameLen = MAX_SYM_NAME; 
  
        bSuccess = SymGetModuleInfoW64(hProc,dwEIP,&imgMod); 
        bSuccess = SymFromAddrW(hProc,dwEIP,&dwDisplacement,pSymbol); 
        sFuncName = pSymbol->Name;        
        sFuncMod = imgMod.ModuleName;        

        callstackdata newCallstack;
        newCallstack.callFrom = dwReturnTo; 
        newCallstack.currentOffset = dwEIP; 
        newCallstack.functionName = QString().fromStdWString(sFuncName); 
        newCallstack.moduleName = QString().fromStdWString(sFuncMod);

		memset(&imgSource,0,sizeof(IMAGEHLP_LINEW64)); 
		imgSource.SizeOfStruct = sizeof(IMAGEHLP_LINEW64); 

		bSuccess = SymGetLineFromAddrW64(hProc,dwEIP,(PDWORD)&dwDisplacement,&imgSource); 
		if(bSuccess)
		{
			newCallstack.sourceFile = QString().fromStdWString(imgSource.FileName);
			newCallstack.sourceLine = imgSource.LineNumber;
		}

        newAlloc.callstackTrace.append(newCallstack); 
	
	}while(stackFr.AddrReturn.Offset != 0); 
  
    free(pSymbol); 
    CloseHandle(hThread); 
	CloseHandle(hProc);
  
    pThis->allocList.insert((DWORD64)newAllocOffset,newAlloc); 

	//EnterCriticalSection(&pThis->CriticalSection);
	pThis->IsInternalAlloc = false;
	LeaveCriticalSection(&pThis->CriticalSection);

    return newAllocOffset; 
  
#else 
    return pThis->Alloc(ulSize); 
#endif 
} 
  
#ifdef MEMLEAKTRACE 
void clsMemManager::printRest() 
{ 
    for(QMap<DWORD64,allocdata>::const_iterator i = allocList.constBegin(); i != allocList.constEnd(); ++i) 
    { 
		qDebug("--------------------------------------------------------------------------");
		qDebug("Leak size: %s",qPrintable(QString("%1").arg(i->dataSize,16,16,QChar('0'))));
		qDebug("Callstack: ");
		qDebug("|------Offset-----|----called from----|----------------Module::Function--------------|-------------Sourcefile and Sourceline-------------------------");
		for(QList<callstackdata>::const_iterator csTrace = i->callstackTrace.constBegin(); csTrace != i->callstackTrace.constEnd(); ++csTrace) 
		{ 
			if(csTrace->sourceFile.length() > 0 && csTrace->sourceLine > 0)
			{
				int fillLen = 50 - (csTrace->moduleName.length() + 2 + csTrace->functionName.length());
				if(fillLen <= 0)
					fillLen = 1;
				
				QString filler = QString().leftJustified(fillLen);

				qDebug(" %s\t%s\t%s::%s%s%s:%d",
					qPrintable(QString("%1").arg(csTrace->currentOffset,16,16,QChar('0'))),
					qPrintable(QString("%1").arg(csTrace->callFrom,16,16,QChar('0'))),
					qPrintable(csTrace->moduleName),
					qPrintable(csTrace->functionName),
					qPrintable(filler),
					qPrintable(csTrace->sourceFile),
					csTrace->sourceLine);
			}
			else
			{
				qDebug(" %s\t%s\t%s::%s",
					qPrintable(QString("%1").arg(csTrace->currentOffset,16,16,QChar('0'))),
					qPrintable(QString("%1").arg(csTrace->callFrom,16,16,QChar('0'))),
					qPrintable(csTrace->moduleName),
					qPrintable(csTrace->functionName));
			}

		}
    } 
} 

bool clsMemManager::GetInternalAlloc()
{
	EnterCriticalSection(&pThis->CriticalSection);
	bool bTemp = pThis->IsInternalAlloc;
	LeaveCriticalSection(&pThis->CriticalSection);
	return bTemp;
}

#endif 
  
// override our new/delete 
void *operator new(size_t size) 
{ 
#ifdef MEMLEAKTRACE
	if(clsMemManager::GetInternalAlloc())
		return malloc(size);
#endif
    return clsMemManager::CAlloc(size); 
} 
  
void operator delete(void *p) 
{ 
#ifdef MEMLEAKTRACE
	if(clsMemManager::GetInternalAlloc())
		return free(p);
#endif
    return clsMemManager::CFree(p); 
} 
  
void *operator new[](size_t size) 
{ 
#ifdef MEMLEAKTRACE
	if(clsMemManager::GetInternalAlloc())
		return malloc(size);
#endif
    return clsMemManager::CAlloc(size); 
} 
  
void operator delete[](void *p) 
{ 
#ifdef MEMLEAKTRACE
	if(clsMemManager::GetInternalAlloc())
		return free(p);
#endif
    return clsMemManager::CFree(p); 
} 