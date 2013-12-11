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

#include "qtDLGNanomite.h"

clsAppSettings::clsAppSettings()
	: QObject()
{
	userSettings = new QSettings(QSettings::NativeFormat, QSettings::UserScope, "Nanomite", "Nanomite");

	readWriteMutex = new QMutex();

	m_pDebugger = qtDLGNanomite::GetInstance()->coreDebugger;
	m_pColors	= qtDLGNanomite::GetInstance()->disasColor;
}

clsAppSettings::~clsAppSettings()
{
	readWriteMutex->lock();

	userSettings->sync();
	
	readWriteMutex->unlock();

	delete userSettings;	
	delete readWriteMutex;

	readWriteMutex	= NULL;
	userSettings	= NULL;
	m_pDebugger		= NULL;
	m_pColors		= NULL;
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

void clsAppSettings::SaveDebuggerSettings()
{
	readWriteMutex->lockInline();

	userSettings->setValue("DebugChilds", m_pDebugger->dbgSettings.bDebugChilds);
	userSettings->setValue("AutoLoadSym", m_pDebugger->dbgSettings.bAutoLoadSymbols);
	userSettings->setValue("UseMSSym", m_pDebugger->dbgSettings.bUseMSSymbols);
	userSettings->setValue("KillOnExit", m_pDebugger->dbgSettings.bKillOnExit);
	userSettings->setValue("BreakOnNewDLL", m_pDebugger->dbgSettings.bBreakOnNewDLL);
	userSettings->setValue("BreakOnNewTID", m_pDebugger->dbgSettings.bBreakOnNewTID);
	userSettings->setValue("BreakOnNewPID", m_pDebugger->dbgSettings.bBreakOnNewPID);
	userSettings->setValue("BreakOnExDLL", m_pDebugger->dbgSettings.bBreakOnExDLL);
	userSettings->setValue("BreakOnExTID", m_pDebugger->dbgSettings.bBreakOnExTID);
	userSettings->setValue("BreakOnExPID", m_pDebugger->dbgSettings.bBreakOnExPID);
	userSettings->setValue("BreakOnModuleEP", m_pDebugger->dbgSettings.bBreakOnModuleEP);
	userSettings->setValue("BreakOnSystemEP", m_pDebugger->dbgSettings.bBreakOnSystemEP);
	userSettings->setValue("BreakOnTLS", m_pDebugger->dbgSettings.bBreakOnTLS);
	userSettings->setValue("ExceptionAssist", m_pDebugger->dbgSettings.bUseExceptionAssist);
	userSettings->setValue("SUSPENDTYPE", QString("%1").arg(m_pDebugger->dbgSettings.dwSuspendType));
	userSettings->setValue("DefaultExceptionMode", QString("%1").arg(m_pDebugger->dbgSettings.dwDefaultExceptionMode));

	int exceptionHCounter = 0;
	QString exceptionString = userSettings->value(QString("EXCEPTION%1").arg(exceptionHCounter)).toString();
	while(exceptionString.contains(":"))
	{
		userSettings->remove(QString("EXCEPTION%1").arg(exceptionHCounter));
		exceptionHCounter++;
		exceptionString = userSettings->value(QString("EXCEPTION%1").arg(exceptionHCounter)).toString();
	}

	for(int i = 0;i < m_pDebugger->ExceptionHandler.size();i++)
	{
		userSettings->setValue(QString("EXCEPTION%1").arg(i),QString("%1:%2").arg(m_pDebugger->ExceptionHandler[i].dwExceptionType,8,16,QChar('0')).arg(m_pDebugger->ExceptionHandler[i].dwAction));
	}

	userSettings->sync();
	readWriteMutex->unlockInline();
}

void clsAppSettings::SaveDisassemblerColor()
{
	readWriteMutex->lockInline();

	userSettings->setValue("COLOR_BP", m_pColors->colorBP);
	userSettings->setValue("COLOR_CALL", m_pColors->colorCall);
	userSettings->setValue("COLOR_JUMP", m_pColors->colorJump);
	userSettings->setValue("COLOR_MOVE", m_pColors->colorMove);
	userSettings->setValue("COLOR_STACK", m_pColors->colorStack);
	userSettings->setValue("COLOR_MATH", m_pColors->colorMath);

	userSettings->sync();
	readWriteMutex->unlockInline();
}

void clsAppSettings::SaveDefaultJITDebugger(QString savedJIT, QString savedJITWOW64)
{
	readWriteMutex->lockInline();

	userSettings->setValue("defaultJIT", savedJIT);
	userSettings->setValue("defaultJITWOW64", savedJITWOW64);

	userSettings->sync();
	readWriteMutex->unlockInline();
}

void clsAppSettings::LoadDebuggerSettings()
{
	readWriteMutex->lockInline();
	
	userSettings->sync();
	m_pDebugger->dbgSettings.bDebugChilds = userSettings->value("DebugChilds").toBool();
	m_pDebugger->dbgSettings.bAutoLoadSymbols = userSettings->value("AutoLoadSym").toBool();
	m_pDebugger->dbgSettings.bUseMSSymbols = userSettings->value("UseMSSym").toBool();
	m_pDebugger->dbgSettings.bKillOnExit = userSettings->value("KillOnExit").toBool();
	m_pDebugger->dbgSettings.bBreakOnNewDLL = userSettings->value("BreakOnNewDLL").toBool();
	m_pDebugger->dbgSettings.bBreakOnNewTID = userSettings->value("BreakOnNewTID").toBool();
	m_pDebugger->dbgSettings.bBreakOnNewPID = userSettings->value("BreakOnNewPID").toBool();
	m_pDebugger->dbgSettings.bBreakOnExDLL = userSettings->value("BreakOnExDLL").toBool();
	m_pDebugger->dbgSettings.bBreakOnExTID = userSettings->value("BreakOnExTID").toBool();
	m_pDebugger->dbgSettings.bBreakOnExPID = userSettings->value("BreakOnExPID").toBool();
	m_pDebugger->dbgSettings.bBreakOnModuleEP = userSettings->value("BreakOnModuleEP").toBool();
	m_pDebugger->dbgSettings.bBreakOnSystemEP = userSettings->value("BreakOnSystemEP").toBool();
	m_pDebugger->dbgSettings.bBreakOnTLS = userSettings->value("BreakOnTLS").toBool();
	m_pDebugger->dbgSettings.bUseExceptionAssist = userSettings->value("ExceptionAssist").toBool();
	m_pDebugger->dbgSettings.dwSuspendType = userSettings->value("SUSPENDTYPE").toInt();
	m_pDebugger->dbgSettings.dwDefaultExceptionMode = userSettings->value("DefaultExceptionMode").toInt();

	m_pDebugger->CustomExceptionRemoveAll();
	int i = 0;
	QString exceptionString = userSettings->value(QString("EXCEPTION%1").arg(i)).toString();

	while(exceptionString.contains(":"))
	{
		QStringList exceptionElements = exceptionString.split(":");
		m_pDebugger->CustomExceptionAdd(exceptionElements.value(0).toULong(0,16),exceptionElements.value(1).toULong(0,16),NULL);
		i++;
		exceptionString = userSettings->value(QString("EXCEPTION%1").arg(i)).toString();
	}

	readWriteMutex->unlockInline();
}

void clsAppSettings::LoadDisassemblerColor()
{
	readWriteMutex->lockInline();

	userSettings->sync();
	m_pColors->colorBP = userSettings->value("COLOR_BP").toString();
	m_pColors->colorCall = userSettings->value("COLOR_CALL").toString();
	m_pColors->colorJump = userSettings->value("COLOR_JUMP").toString();
	m_pColors->colorMove = userSettings->value("COLOR_MOVE").toString();
	m_pColors->colorStack = userSettings->value("COLOR_STACK").toString();
	m_pColors->colorMath = userSettings->value("COLOR_MATH").toString();

	readWriteMutex->unlockInline();
}

void clsAppSettings::LoadDefaultJITDebugger(QString& savedJIT, QString& savedJITWOW64)
{
	readWriteMutex->lockInline();

	userSettings->sync();
	savedJIT = userSettings->value("defaultJIT").toString();
	savedJITWOW64 = userSettings->value("defaultJITWOW64").toString();

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
	userSettings->setValue("UseMSSym", false);
	userSettings->setValue("KillOnExit", false);
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

	for(int i = 0; i < 5; i++)
	{
		userSettings->setValue(QString("RECENTFILE%1").arg(i),"");
	}

	userSettings->sync();
	//readWriteMutex->unlockInline();
	return;
}

void clsAppSettings::SaveRecentDebuggedFiles(QStringList recentDebuggedFiles)
{
	readWriteMutex->lockInline();

	for(int i = 0; i < 5; i++)
	{
		userSettings->setValue(QString("RECENTFILE%1").arg(i), recentDebuggedFiles.value(i));
	}

	userSettings->sync();
	readWriteMutex->unlockInline();
}

void clsAppSettings::LoadRecentDebuggedFiles(QStringList &recentDebuggedFiles)
{
	readWriteMutex->lockInline();

	userSettings->sync();
	for(int i = 0; i < 5; i++)
	{
		recentDebuggedFiles.append(userSettings->value(QString("RECENTFILE%1").arg(i)).toString());
	}

	readWriteMutex->unlockInline();
}