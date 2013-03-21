#include "clsMemPoolNew.h"
#include <new>

#ifdef _DEBUG
	#include <QDebug>
#endif

clsMemPoolNew::clsMemPoolNew(DWORD64 &PoolBufferBase, DWORD64 &PoolBufferSize, unsigned long ulUnitNum, unsigned long ulUnitSize)
{    
	InitializeCriticalSection(&CriticalSection);

#ifdef _DEBUG
	countAlloc = NULL;
	countFree = NULL;
	averageSize = NULL;
#endif

	PoolBufferSize = PoolMemorySize = (PoolUnitCount = ulUnitNum) * (PoolUnitSize = ulUnitSize);
	pPoolMemoryBase = malloc(PoolMemorySize);
	pPoolBitmask = malloc(PoolUnitCount);
	PoolBufferBase = (DWORD64)pPoolMemoryBase;

	memset(pPoolBitmask,0x00,PoolUnitCount);
} 

clsMemPoolNew::~clsMemPoolNew()
{
	EnterCriticalSection(&CriticalSection);

#ifdef _DEBUG
	qDebug() << "There have been " << countAlloc << " allocs";
	qDebug() << "There have been" << countFree << " frees";
	if(countAlloc > 0)
		qDebug() << "Average size was " << averageSize / countAlloc << " bytes";
#endif

	if(pPoolMemoryBase != NULL)
		free(pPoolMemoryBase);

	LeaveCriticalSection(&CriticalSection);
	DeleteCriticalSection(&CriticalSection);
}

void* clsMemPoolNew::Alloc(unsigned long ulSize)
{
	EnterCriticalSection(&CriticalSection);

    if(ulSize > PoolUnitSize || pPoolMemoryBase == NULL)
    {
#ifdef _DEBUG
		qDebug() << "Alloc too huge! - " << ulSize << " bytes";
#endif
		LeaveCriticalSection(&CriticalSection);
        return malloc(ulSize);
    }
	
#ifdef _DEBUG
	averageSize += ulSize;
	countAlloc++;
#endif

	PDWORD64 BitMaskPointer = NULL;
	int stepCounter = 0;

	for(DWORD64 freeBlockFinder = 0; freeBlockFinder < PoolUnitCount; freeBlockFinder += 64)
	{
		BitMaskPointer = (PDWORD64)((PCHAR)pPoolBitmask + freeBlockFinder);

		if(*BitMaskPointer != 0xFFFFFFFFFFFFFFFF)
		{
			// not all units alloced here!
			for(int i = 0; i < 64; i++)
			{
				bool temp = (*(PDWORD64)((PCHAR)pPoolBitmask + freeBlockFinder) & (2^i)) ? true : false;
				if(!temp)
				{
					*(PDWORD64)((PCHAR)pPoolBitmask + freeBlockFinder) |= 2^i;
					return ((PCHAR)pPoolMemoryBase + (PoolUnitSize * ((stepCounter * 64) + i)));
				}
			}
		}
		stepCounter++;
	}

#ifdef _DEBUG
	qDebug() << "We run out of Free Blocks!";
#endif

	LeaveCriticalSection(&CriticalSection);
    return malloc(ulSize); // if we end here we didn´t have any free blocks left!
}

void clsMemPoolNew::Free(void* p)
{
	EnterCriticalSection(&CriticalSection);
    if(pPoolMemoryBase != NULL && pPoolMemoryBase < p && p < (void *)((char *)pPoolMemoryBase + PoolMemorySize))
    {
        
#ifdef _DEBUG
		countFree++;
#endif


		
    }
    else
    {
        free(p);
    }

	LeaveCriticalSection(&CriticalSection);
}

void* clsMemPoolNew::operator new(size_t size) throw(std::bad_alloc)
{
	return malloc(size);
}

void clsMemPoolNew::operator delete(void *p) throw()
{
	free(p);
}