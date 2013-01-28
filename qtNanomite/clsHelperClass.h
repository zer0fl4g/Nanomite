#ifndef qHelperClass
#define qHelperClass

#include "clsDebugger/clsDebugger.h"

class clsHelperClass
{
public:
	clsHelperClass();
	~clsHelperClass();

	static bool WriteToSettingsFile(clsDebugger *_coreDebugger);
	static bool ReadFromSettingsFile(clsDebugger *_coreDebugger);
	static bool MenuLoadNewFile(clsDebugger *_coreDebugger);
	static std::vector<std::wstring> split(const std::wstring& s,const std::wstring& f);
	static bool LoadSymbolForAddr(std::wstring& sFuncName,std::wstring& sModName,quint64 dwOffset,HANDLE hProc);
};

#endif // QTDLGNANOMITE_H
