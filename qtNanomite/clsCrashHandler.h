#ifndef CLSCRASHHANDLER_H
#define CLSCRASHHANDLER_H

#include <Windows.h>

class clsCrashHandler
{
public:

	static LONG CALLBACK ErrorReporter(PEXCEPTION_POINTERS pExceptionPtrs);

};
#endif