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
#include "clsMemPool.h"
#include <new>

#ifdef _DEBUG
	#include <QDebug>
#endif

clsMemPool::clsMemPool(DWORD64 &PoolBufferBase, DWORD64 &PoolBufferSize, size_t ulUnitNum, size_t ulUnitSize) :
    m_pMemBlock(NULL), m_pAllocatedMemBlock(NULL), m_pFreeMemBlock(NULL), 
    m_ulBlockSize(ulUnitNum * (ulUnitSize + sizeof(struct _Unit))), 
    m_ulUnitSize(ulUnitSize)
{    
	InitializeCriticalSection(&CriticalSection);

#ifdef _DEBUG
	countAlloc = NULL;
	countFree = NULL;
	averageSize = NULL;
#endif

	m_pMemBlock = malloc(m_ulBlockSize);
    PoolBufferBase = (DWORD64)m_pMemBlock;
	PoolBufferSize = m_ulBlockSize;

    if(NULL != m_pMemBlock)
    {
        for(unsigned long i=0; i<ulUnitNum; i++)
        {
            struct _Unit *pCurUnit = (struct _Unit *)((char *)m_pMemBlock + i * (ulUnitSize + sizeof(struct _Unit)));
            
            pCurUnit->pPrev = NULL;
            pCurUnit->pNext = m_pFreeMemBlock;
            
            if(NULL != m_pFreeMemBlock)
            {
                m_pFreeMemBlock->pPrev = pCurUnit;
            }
            m_pFreeMemBlock = pCurUnit;
        }
    }    
} 

clsMemPool::~clsMemPool()
{
	EnterCriticalSection(&CriticalSection);

#ifdef _DEBUG
	qDebug() << "There have been " << countAlloc << " allocs";
	qDebug() << "There have been" << countFree << " frees";
	if(countAlloc > 0)
		qDebug() << "Average size was " << averageSize / countAlloc << " bytes";
#endif

	if(m_pMemBlock != NULL)
		free(m_pMemBlock);
	LeaveCriticalSection(&CriticalSection);
	DeleteCriticalSection(&CriticalSection);
}

void* clsMemPool::Alloc(size_t ulSize)
{
	EnterCriticalSection(&CriticalSection);
    if(ulSize > m_ulUnitSize || NULL == m_pMemBlock || NULL == m_pFreeMemBlock)
    {
#ifdef _DEBUG
		if(NULL == m_pFreeMemBlock)
			qDebug() << "Out of Blocks!!";

		if(ulSize > m_ulUnitSize)
			qDebug() << "Alloc too huge! - " << ulSize << " bytes";
#endif
		LeaveCriticalSection(&CriticalSection);
        return malloc(ulSize);
    }
	
#ifdef _DEBUG
	averageSize += ulSize;
	countAlloc++;
#endif

	// take a free block from the top
    struct _Unit *pCurUnit = m_pFreeMemBlock;
	// set the new top block
    m_pFreeMemBlock = pCurUnit->pNext;
    if(NULL != m_pFreeMemBlock)
    {
		// top is valid, nothing before
        m_pFreeMemBlock->pPrev = NULL;
    }
	// else we run out of blocks

	// top of alloc blocks gets our next
    pCurUnit->pNext = m_pAllocatedMemBlock;
    
    if(NULL != m_pAllocatedMemBlock)
    {
		// set the new top to our current
        m_pAllocatedMemBlock->pPrev = pCurUnit; 
    }
	// replace top
    m_pAllocatedMemBlock = pCurUnit;

	LeaveCriticalSection(&CriticalSection);

    return (void *)((char *)pCurUnit + sizeof(struct _Unit));
}

void clsMemPool::Free(void* p)
{
	EnterCriticalSection(&CriticalSection);
    if(m_pMemBlock != NULL && m_pMemBlock < p && p < (void *)((char *)m_pMemBlock + m_ulBlockSize))
    {
        struct _Unit *pCurUnit = (struct _Unit *)((char *)p - sizeof(struct _Unit));

#ifdef _DEBUG
		countFree++;
#endif

		// set top to our next
        m_pAllocatedMemBlock = pCurUnit->pNext;
        if(NULL != m_pAllocatedMemBlock)
        {
			// top doesn´t have prev
            m_pAllocatedMemBlock->pPrev = NULL;
        }

		// current top into our next
        pCurUnit->pNext = m_pFreeMemBlock;
        if(NULL != m_pFreeMemBlock)
        {
			// set our as top
             m_pFreeMemBlock->pPrev = pCurUnit;
        }
		// replace top
        m_pFreeMemBlock = pCurUnit;
    }
    else
    {
        free(p);
    }
	LeaveCriticalSection(&CriticalSection);
}

void* clsMemPool::operator new(size_t size)
{
	return malloc(size);
}

void clsMemPool::operator delete(void *p) throw()
{
	free(p);
}