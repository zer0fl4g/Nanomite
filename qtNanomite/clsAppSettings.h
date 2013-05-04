#ifndef CLSAPPSETTINGS_H
#define CLSAPPSETTINGS_H

#include <QObject>
#include <qmainwindow.h>
#include <qmutex.h>
#include <qsettings.h>

class clsAppSettings : public QObject
{
	Q_OBJECT

public:
	void SaveWindowState(QMainWindow* window);
	bool RestoreWindowState(QMainWindow* window);
	void ResetWindowState();

	static clsAppSettings* SharedInstance();

protected:
	//QSettings *appSettings;
	QSettings *userSettings;

private:
	static clsAppSettings *instance;
	QMutex *readWriteMutex;

	clsAppSettings();
	~clsAppSettings();
};

#endif // CLSAPPSETTINGS_H
