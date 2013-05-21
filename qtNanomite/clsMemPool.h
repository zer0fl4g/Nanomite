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
#ifndef CLSMEMPOOL_H
#define CLSMEMPOOL_H
             
// based on http://www.codeproject.com/Articles/27487/Why-to-use-memory-pool-and-how-to-implement-it
// modded to my needs

#include <Windows.h>
#include <new>

class clsMemPool
{
public:
    clsMemPool(DWORD64 &PoolBufferBase, DWORD64 &PoolBufferSize, size_t lUnitNum = 50, size_t lUnitSize = 1024);
    ~clsMemPool();
    
    void* Alloc(size_t ulSize);
    void Free(void* p);

	// need to override this here again 
	// else the clsMemPool will not be able to create itself
	void *operator new(size_t size);
	void operator delete(void *p);

private:
    struct _Unit
    {
        struct _Unit *pPrev;
		struct _Unit *pNext;
    };

    void* m_pMemBlock;

    struct _Unit*    m_pAllocatedMemBlock;
    struct _Unit*    m_pFreeMemBlock;

    size_t    m_ulUnitSize;
    size_t    m_ulBlockSize;

#ifdef _DEBUG
	DWORD64 countAlloc,countFree,averageSize;
#endif
		
	CRITICAL_SECTION CriticalSection;
};
#endif //CLSMEMPOOL_H