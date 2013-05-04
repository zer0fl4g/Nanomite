#include "clsAppSettings.h"

clsAppSettings::clsAppSettings()
	: QObject()
{
	userSettings = new QSettings(QSettings::Format::NativeFormat, QSettings::Scope::UserScope, "Nanomite", "Nanomite");

	readWriteMutex = new QMutex();
}

clsAppSettings::~clsAppSettings()
{
	readWriteMutex->lock();

	userSettings->sync();
	
	readWriteMutex->unlock();

	delete userSettings;
	
	delete readWriteMutex;

	readWriteMutex = NULL;
	userSettings = NULL;
}

void clsAppSettings::SaveWindowState(QMainWindow* window)
{
	readWriteMutex->lockInline();
	
	userSettings->setValue("mainWindowState", window->saveState());
	userSettings->setValue("mainWindowGeometry", window->saveGeometry());
	userSettings->sync();
	
	readWriteMutex->unlockInline();
}

bool clsAppSettings::RestoreWindowState(QMainWindow* window)
{
	bool result = true;
	readWriteMutex->lockInline();

	userSettings->sync();

	result &= window->restoreGeometry(userSettings->value("mainWindowGeometry").toByteArray());
	result &= window->restoreState(userSettings->value("mainWindowState").toByteArray());

	readWriteMutex->unlockInline();

	return result;
}

void clsAppSettings::ResetWindowState()
{
	userSettings->remove("mainWindowGeometry");
	userSettings->remove("mainWindowState");
}

clsAppSettings* clsAppSettings::instance = NULL;
clsAppSettings* clsAppSettings::SharedInstance()
{
	QMutex mutex;

	mutex.lockInline();

	if (instance == NULL)
	{
		instance = new clsAppSettings();
	}

	mutex.unlockInline();

	return clsAppSettings::instance;
	
}
