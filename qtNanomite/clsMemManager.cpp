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
#include <new>

clsMemManager* clsMemManager::pThis = NULL;

clsMemManager::clsMemManager()
{    
	pThis = this;

	// Create a 5000x200Byte Pool
	PoolUnitCount_200 = 5000;
	PoolUnitSize_200 = 200;
	pPool_200 = new clsMemPool(PoolBufferBase_200,PoolBufferSize_200,PoolUnitCount_200,PoolUnitSize_200);
	PoolBufferEnd_200 = PoolBufferBase_200 + PoolBufferSize_200;

	// Create a 50000x50Byte Pool
	PoolUnitCount_50 = 50000;
	PoolUnitSize_50 = 50;
	pPool_50 = new clsMemPool(PoolBufferBase_50,PoolBufferSize_50,PoolUnitCount_50,PoolUnitSize_50);
	PoolBufferEnd_50 = PoolBufferBase_50 + PoolBufferSize_50;
} 

clsMemManager::~clsMemManager()
{
	delete pPool_200;
	delete pPool_50;
}

void* clsMemManager::Alloc(unsigned long ulSize, bool bUseMemPool)
{
	if(ulSize <= PoolUnitSize_200 && ulSize > PoolUnitSize_50) // huge pool
		return pPool_200->Alloc(ulSize);
	else if(ulSize <= PoolUnitSize_50 && ulSize > 0) // little pool
		return pPool_50->Alloc(ulSize);
	else if(!bUseMemPool) // don´t use pool 
		return malloc(ulSize);
	else if(ulSize > PoolUnitSize_200) // alloc request too huge
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
	pThis->Free(p);
}

void* clsMemManager::CAlloc(unsigned long ulSize,bool bUseMemPool)
{
	return pThis->Alloc(ulSize,bUseMemPool);
}



// override our new/delete
void *operator new(size_t size) throw(std::bad_alloc)
{
    return clsMemManager::CAlloc(size);
}

void operator delete(void *p) throw()
{
    return clsMemManager::CFree(p);
}

void *operator new(size_t size, const std::nothrow_t &) throw() 
{
    return clsMemManager::CAlloc(size);
}

void operator delete(void *p, const std::nothrow_t &) throw() 
{
    return clsMemManager::CFree(p);
}

void *operator new[](size_t size) throw(std::bad_alloc)
{
    return clsMemManager::CAlloc(size);
}

void operator delete[](void *p) throw()
{
    return clsMemManager::CFree(p);
}

void *operator new[](size_t size, const std::nothrow_t &) throw() 
{
    return clsMemManager::CAlloc(size);
}

void operator delete[](void *p, const std::nothrow_t &) throw() 
{
    return clsMemManager::CFree(p);
}

