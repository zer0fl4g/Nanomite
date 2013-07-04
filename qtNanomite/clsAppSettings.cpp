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

void clsAppSettings::SaveDebuggerSettings(clsDebugger *pDebugger)
{
	readWriteMutex->lockInline();

	userSettings->setValue("DebugChilds", pDebugger->dbgSettings.bDebugChilds);
	userSettings->setValue("AutoLoadSym", pDebugger->dbgSettings.bAutoLoadSymbols);
	userSettings->setValue("BreakOnNewDLL", pDebugger->dbgSettings.bBreakOnNewDLL);
	userSettings->setValue("BreakOnNewTID", pDebugger->dbgSettings.bBreakOnNewTID);
	userSettings->setValue("BreakOnNewPID", pDebugger->dbgSettings.bBreakOnNewPID);
	userSettings->setValue("BreakOnExDLL", pDebugger->dbgSettings.bBreakOnExDLL);
	userSettings->setValue("BreakOnExTID", pDebugger->dbgSettings.bBreakOnExTID);
	userSettings->setValue("BreakOnExPID", pDebugger->dbgSettings.bBreakOnExPID);
	userSettings->setValue("BreakOnModuleEP", pDebugger->dbgSettings.bBreakOnModuleEP);
	userSettings->setValue("BreakOnSystemEP", pDebugger->dbgSettings.bBreakOnSystemEP);
	userSettings->setValue("BreakOnTLS", pDebugger->dbgSettings.bBreakOnTLS);
	userSettings->setValue("ExceptionAssist", pDebugger->dbgSettings.bUseExceptionAssist);
	userSettings->setValue("SUSPENDTYPE", QString("%1").arg(pDebugger->dbgSettings.dwSuspendType));
	userSettings->setValue("DefaultExceptionMode", QString("%1").arg(pDebugger->dbgSettings.dwDefaultExceptionMode));

	int exceptionHCounter = 0;
	QString exceptionString = userSettings->value(QString("EXCEPTION%1").arg(exceptionHCounter)).toString();
	while(exceptionString.contains(":"))
	{
		userSettings->remove(QString("EXCEPTION%1").arg(exceptionHCounter));
		exceptionHCounter++;
		exceptionString = userSettings->value(QString("EXCEPTION%1").arg(exceptionHCounter)).toString();
	}

	for(size_t i = 0;i < pDebugger->ExceptionHandler.size();i++)
	{
		userSettings->setValue(QString("EXCEPTION%1").arg(i),QString("%1:%2").arg(pDebugger->ExceptionHandler[i].dwExceptionType,8,16,QChar('0')).arg(pDebugger->ExceptionHandler[i].dwAction));
	}

	userSettings->sync();
	readWriteMutex->unlockInline();
}

void clsAppSettings::SaveDisassemblerColor(qtNanomiteDisAsColorSettings *pDisassemlberColor)
{
	readWriteMutex->lockInline();

	userSettings->setValue("COLOR_BP", pDisassemlberColor->colorBP);
	userSettings->setValue("COLOR_CALL", pDisassemlberColor->colorCall);
	userSettings->setValue("COLOR_JUMP", pDisassemlberColor->colorJump);
	userSettings->setValue("COLOR_MOVE", pDisassemlberColor->colorMove);
	userSettings->setValue("COLOR_STACK", pDisassemlberColor->colorStack);
	userSettings->setValue("COLOR_MATH", pDisassemlberColor->colorMath);

	userSettings->sync();
	readWriteMutex->unlockInline();
}

void clsAppSettings::SaveDefaultJITDebugger(QString savedJIT)
{
	readWriteMutex->lockInline();

	userSettings->setValue("defaultJIT", savedJIT);

	userSettings->sync();
	readWriteMutex->unlockInline();
}

void clsAppSettings::LoadDebuggerSettings(clsDebugger *pDebugger)
{
	readWriteMutex->lockInline();

	pDebugger->dbgSettings.bDebugChilds = userSettings->value("DebugChilds").toBool();
	pDebugger->dbgSettings.bAutoLoadSymbols = userSettings->value("AutoLoadSym").toBool();
	pDebugger->dbgSettings.bBreakOnNewDLL = userSettings->value("BreakOnNewDLL").toBool();
	pDebugger->dbgSettings.bBreakOnNewTID = userSettings->value("BreakOnNewTID").toBool();
	pDebugger->dbgSettings.bBreakOnNewPID = userSettings->value("BreakOnNewPID").toBool();
	pDebugger->dbgSettings.bBreakOnExDLL = userSettings->value("BreakOnExDLL").toBool();
	pDebugger->dbgSettings.bBreakOnExTID = userSettings->value("BreakOnExTID").toBool();
	pDebugger->dbgSettings.bBreakOnExPID = userSettings->value("BreakOnExPID").toBool();
	pDebugger->dbgSettings.bBreakOnModuleEP = userSettings->value("BreakOnModuleEP").toBool();
	pDebugger->dbgSettings.bBreakOnSystemEP = userSettings->value("BreakOnSystemEP").toBool();
	pDebugger->dbgSettings.bBreakOnTLS = userSettings->value("BreakOnTLS").toBool();
	pDebugger->dbgSettings.bUseExceptionAssist = userSettings->value("ExceptionAssist").toBool();
	pDebugger->dbgSettings.dwSuspendType = userSettings->value("SUSPENDTYPE").toInt();
	pDebugger->dbgSettings.dwDefaultExceptionMode = userSettings->value("DefaultExceptionMode").toInt();

	pDebugger->CustomExceptionRemoveAll();
	int i = 0;
	QString exceptionString = userSettings->value(QString("EXCEPTION%1").arg(i)).toString();

	while(exceptionString.contains(":"))
	{
		QStringList exceptionElements = exceptionString.split(":");
		pDebugger->CustomExceptionAdd(exceptionElements.value(0).toULong(0,16),exceptionElements.value(1).toULong(0,16),NULL);
		i++;
		exceptionString = userSettings->value(QString("EXCEPTION%1").arg(i)).toString();
	}

	userSettings->sync();
	readWriteMutex->unlockInline();
}

void clsAppSettings::LoadDisassemblerColor(qtNanomiteDisAsColorSettings *pDisassemlberColor)
{
	readWriteMutex->lockInline();

	pDisassemlberColor->colorBP = userSettings->value("COLOR_BP").toString();
	pDisassemlberColor->colorCall = userSettings->value("COLOR_CALL").toString();
	pDisassemlberColor->colorJump = userSettings->value("COLOR_JUMP").toString();
	pDisassemlberColor->colorMove = userSettings->value("COLOR_MOVE").toString();
	pDisassemlberColor->colorStack = userSettings->value("COLOR_STACK").toString();
	pDisassemlberColor->colorMath = userSettings->value("COLOR_MATH").toString();

	userSettings->sync();
	readWriteMutex->unlockInline();
}

void clsAppSettings::LoadDefaultJITDebugger(QString& savedJIT)
{
	readWriteMutex->lockInline();

	savedJIT = userSettings->value("defaultJIT").toString();

	userSettings->sync();
	readWriteMutex->unlockInline();
}

void clsAppSettings::CheckIfFirstRun()
{
	readWriteMutex->lockInline();

	bool bFirstRun = userSettings->value("IsFirstRun").toBool();
	if(!bFirstRun)
	{
		userSettings->setValue("IsFirstRun",true);
		WriteDefaultSettings();
	}

	userSettings->sync();
	readWriteMutex->unlockInline();
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

void clsAppSettings::WriteDefaultSettings()
{
	//readWriteMutex->lockInline();

	userSettings->setValue("DebugChilds", true);
	userSettings->setValue("AutoLoadSym", true);
	userSettings->setValue("BreakOnNewDLL", false);
	userSettings->setValue("BreakOnNewTID", false);
	userSettings->setValue("BreakOnNewPID", false);
	userSettings->setValue("BreakOnExDLL", false);
	userSettings->setValue("BreakOnExTID", false);
	userSettings->setValue("BreakOnExPID", false);
	userSettings->setValue("BreakOnModuleEP", true);
	userSettings->setValue("BreakOnSystemEP", false);
	userSettings->setValue("BreakOnTLS", false);
	userSettings->setValue("ExceptionAssist", false);
	userSettings->setValue("SUSPENDTYPE", 0);
	userSettings->setValue("DefaultExceptionMode", 0);
	userSettings->setValue("COLOR_BP", "Red");
	userSettings->setValue("COLOR_CALL", "Green");
	userSettings->setValue("COLOR_JUMP", "Dark green");
	userSettings->setValue("COLOR_MOVE", "Blue");
	userSettings->setValue("COLOR_STACK", "Gray");
	userSettings->setValue("COLOR_MATH", "Magenta");

	int i = 0;
	QString exceptionString = userSettings->value(QString("EXCEPTION%1").arg(i)).toString();
	while(exceptionString.contains(":"))
	{
		userSettings->remove(QString("EXCEPTION%1").arg(i));
		i++;
		exceptionString = userSettings->value(QString("EXCEPTION%1").arg(i)).toString();
	}

	userSettings->sync();
	//readWriteMutex->unlockInline();
	return;
}