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
	static bool IsWindowsXP();
	static bool SetThreadPriorityByTid(DWORD ThreadID, int threadPrio);
	static bool LoadSymbolForAddr(std::wstring& sFuncName,std::wstring& sModName,quint64 dwOffset,HANDLE hProc);
	static DWORD GetMainThread(DWORD ThreadID);
	static void LoadSourceForAddr(std::wstring &FileName,int &LineNumber,quint64 dwOffset,HANDLE hProc);
	static std::vector<std::wstring> split(const std::wstring& s,const std::wstring& f);
	static std::string convertWSTRtoSTR(std::wstring FileName);
	static std::wstring convertSTRtoWSTR(std::string FileName);
	static std::wstring clsHelperClass::replaceAll(std::wstring orgString,std::wstring oldString,std::wstring newString);
	static PTCHAR reverseStrip(PTCHAR lpString, TCHAR lpSearchString);
	static QString LoadStyleSheet();

};

#endif // QTDLGNANOMITE_H
