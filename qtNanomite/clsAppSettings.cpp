/*
 * 	This file is part of Nanomite.
 *
 *    Nanomite is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    Nanomite is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with Nanomite.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "clsAppSettings.h"
#include "clsMemManager.h"

clsAppSettings::clsAppSettings()
	: QObject()
{
	userSettings = new QSettings(QSettings::NativeFormat, QSettings::UserScope, "Nanomite", "Nanomite");

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
