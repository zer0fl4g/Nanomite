#include "qtDLGNanomite.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	qtDLGNanomite w;
	w.show();
	return a.exec();
}
