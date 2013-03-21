#ifndef CLSMEMPOOLNEW_H
#define CLSMEMPOOLNEW_H
             
#include <Windows.h>
#include <new>

class clsMemPoolNew
{
public:
    clsMemPoolNew(DWORD64 &PoolBufferBase, DWORD64 &PoolBufferSize, unsigned long lUnitNum = 50, unsigned long lUnitSize = 1024);
    ~clsMemPoolNew();
    
    void* Alloc(unsigned long ulSize);
    void Free(void* p);

	// need to override this here again 
	// else the clsMemPool will not be able to create itself
	void *operator new(size_t size) throw(std::bad_alloc);
	void operator delete(void *p) throw();

private:
    LPVOID pPoolMemoryBase;
	LPVOID pPoolBitmask;

	DWORD64 PoolMemorySize;
	DWORD64 PoolUnitCount;
	DWORD64 PoolUnitSize;

#ifdef _DEBUG
	DWORD64 countAlloc,countFree,averageSize;
#endif
		
	CRITICAL_SECTION CriticalSection;
};
#endif //CLSMEMPOOLNEW_H