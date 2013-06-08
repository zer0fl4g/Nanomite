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
#ifndef CLSMEMMANAGER_H 
#define CLSMEMMANAGER_H 
  
#include <Windows.h> 
  
#include "clsMemPool.h" 
 
// use "MEMLEAKTRACE" only to trace memory leaks. It will slowdown the debugger
// and increase ram usage a lot!
#define MEMLEAKTRACE 
#ifdef MEMLEAKTRACE   
#include <dbghelp.h> 
#include <string> 
  
#include <QList> 
#include <QMap> 
#include <QString> 
#include <QDebug> 
#endif 
  
class clsMemManager 
{ 
public: 
#ifdef MEMLEAKTRACE 
	static bool IsInternalAlloc;
	static bool GetInternalAlloc();
#endif

	clsMemManager(); 
    ~clsMemManager(); 
      
    void* Alloc(size_t ulSize); 
    void Free(void* p); 
  
    static void* CAlloc(size_t ulSize); 
    static void CFree(void* p); 
  
private: 
    clsMemPool *pPool_200; 
  
    DWORD64 PoolBufferBase_200; 
    DWORD64 PoolBufferEnd_200; 
    DWORD64 PoolBufferSize_200; 
    DWORD64 PoolUnitCount_200; 
    DWORD64 PoolUnitSize_200; 
  
    clsMemPool *pPool_50; 
  
    DWORD64 PoolBufferBase_50; 
    DWORD64 PoolBufferEnd_50; 
    DWORD64 PoolBufferSize_50; 
    DWORD64 PoolUnitCount_50; 
    DWORD64 PoolUnitSize_50; 
  
    static clsMemManager *pThis; 
  
  
#ifdef MEMLEAKTRACE 
    struct callstackdata 
    { 
        DWORD64 callFrom; 
        DWORD64 currentOffset; 
        QString moduleName; 
        QString functionName; 
        QString sourceFile; 
        size_t sourceLine; 
    }; 
  
    struct allocdata 
    { 
        DWORD64 dataOffset; 
        size_t dataSize; 
        QList<callstackdata> callstackTrace; 
    }; 
  
    QMap<DWORD64,allocdata> allocList; 
	
	CRITICAL_SECTION CriticalSection;

    void printRest(); 
#endif 
}; 
#endif //CLSMEMMANAGER_H