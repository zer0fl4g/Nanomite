#ifndef clsCALLBACK
#define clsCALLBACK

#include <Windows.h>
#include <string>
#include <QTCore>

class clsCallbacks : public QObject
{
	Q_OBJECT
public:
	clsCallbacks();
	~clsCallbacks();

	static bool CALLBACK EnumWindowCallBack(HWND hWnd,LPARAM lParam);

public slots:
		int OnLog(tm *timeInfo,std::wstring sLog);
		int OnThread(DWORD dwPID,DWORD dwTID,DWORD64 dwEP,bool bSuspended,DWORD dwExitCode,bool bFound);
		int OnPID(DWORD dwPID,std::wstring sFile,DWORD dwExitCode,DWORD64 dwEP,bool bFound);
		int OnException(std::wstring sFuncName,std::wstring sModName,DWORD64 dwOffset,DWORD64 dwExceptionCode,DWORD dwPID,DWORD dwTID);
		int OnDbgString(std::wstring sMessage,DWORD dwPID);
		int OnDll(std::wstring sDLLPath,DWORD dwPID,DWORD64 dwEP,bool bLoaded);
		int OnCallStack(DWORD64 dwStackAddr,DWORD64 dwReturnTo,std::wstring sReturnToFunc,std::wstring sModuleName,DWORD64 dwEIP,std::wstring sFuncName,std::wstring sFuncModule,std::wstring sSourceFilePath,int iSourceLineNum);
};

#endif
