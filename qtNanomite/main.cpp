#include "qtDLGNanomite.h"
#include "clsCrashHandler.h"
#include "clsMemManager.h"

#include <QtGui/QApplication>
//#include <QDebug>

int main(int argc, char *argv[])
{
	AddVectoredExceptionHandler(1,clsCrashHandler::ErrorReporter);

	clsMemManager clsMManage = clsMemManager();
	//Tests - 500bytes, 100000 rounds
	//Test using malloc and free:  8750 
	//Test using clsMemManager:  31
	//
	//Test - 1014bytes, 100000 rounds
	//Test using malloc and free:  9187 
	//Test using clsMemManager:  31

	//DWORD dwStartTick = GetTickCount();

	//DWORD pMem[100000];
	//for(int i = 0; i < 100000; i++)
	//{
	//	pMem[i] = (DWORD)malloc(512);
	//}

	//for(int i = 0; i < 100000; i++)
	//{
	//	free((void*)pMem[i]);
	//}
	//qDebug() << "Test using malloc and free: " << GetTickCount() - dwStartTick;

	//
	//dwStartTick = GetTickCount();
	//for(int i = 0; i < 100000; i++)
	//{
	//	pMem[i] = (DWORD)clsMemManager::CAlloc(512);
	//}
	//
	//for(int i = 0; i < 100000; i++)
	//{
	//	clsMemManager::CFree((void*)pMem[i]);
	//}
	//qDebug() << "Test using clsMemManager: " << GetTickCount() - dwStartTick;

	QSettings adminTest("HKEY_LOCAL_MACHINE", QSettings::NativeFormat);
	QVariant currentValue = adminTest.value("(Default)");
	adminTest.setValue("(Default)", currentValue);
	adminTest.sync();
	if (adminTest.status() == QSettings::AccessError)
		MessageBoxW(NULL,L"You didn´t start the debugger with admin rights!\r\nThis could cause problems with some features!",L"Nanomite",MB_OK);

	QApplication a(argc, argv);
	qtDLGNanomite w;
	w.show();

#ifdef _DEBUG
	return a.exec(); 
#else
	// ugly workaround for cruntime crash caused by new override!
	TerminateProcess(GetCurrentProcess(),a.exec());
#endif
}
