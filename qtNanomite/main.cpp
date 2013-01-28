#include "qtDLGNanomite.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
    QSettings adminTest("HKEY_LOCAL_MACHINE", QSettings::NativeFormat);
    QVariant currentValue = adminTest.value("(Default)");
    adminTest.setValue("(Default)", currentValue);
    adminTest.sync();
    if (adminTest.status() == QSettings::AccessError)
		MessageBoxW(NULL,L"You didn´t start the debugger with admin rights!\r\nThis could cause problems with some features!",L"Nanomite",MB_OK);

	QApplication a(argc, argv);
	qtDLGNanomite w;
	w.show();
	return a.exec();
}
