#ifndef qHelperClass
#define qHelperClass

#include <QStringlist.h>
#include <QString>

#include "clsDebugger/clsDebugger.h"
#include "qtDLGNanomite.h"

class clsHelperClass
{
public:
	clsHelperClass();
	~clsHelperClass();

	static bool WriteToSettingsFile(clsDebugger *_coreDebugger,qtNanomiteDisAsColorSettings *qtNanomiteDisAsColor);
	static bool ReadFromSettingsFile(clsDebugger *_coreDebugger,qtNanomiteDisAsColorSettings *qtNanomiteDisAsColor);
	static bool MenuLoadNewFile(clsDebugger *_coreDebugger);
	static std::vector<std::wstring> split(const std::wstring& s,const std::wstring& f);
	static bool LoadSymbolForAddr(std::wstring& sFuncName,std::wstring& sModName,quint64 dwOffset,HANDLE hProc);
	static std::string convertWSTRtoSTR(std::wstring FileName);
	static std::wstring convertSTRtoWSTR(std::string FileName);
};

#endif // QTDLGNANOMITE_H
