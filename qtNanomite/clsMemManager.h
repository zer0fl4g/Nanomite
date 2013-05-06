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
#ifndef CLSMEMMANAGER_H
#define CLSMEMMANAGER_H

#include <Windows.h>

#include "clsMemPool.h"

class clsMemManager
{
public:
    clsMemManager();
    ~clsMemManager();
    
    void* Alloc(unsigned long ulSize, bool bUseMemPool);
    void Free(void* p);

	static void* CAlloc(unsigned long ulSize, bool bUseMemPool = true);
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
};
#endif //CLSMEMMANAGER_H