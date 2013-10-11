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
#include "qtDLGOption.h"
#include "qtDLGExceptionEdit.h"

#include "clsMemManager.h"

qtDLGOption::qtDLGOption(QWidget *parent, Qt::WFlags flags)
	: QDialog(parent, flags)
{
	setupUi(this);
	this->setFixedSize(this->width(),this->height());
	tblCustomExceptions->horizontalHeader()->setFixedHeight(21);

	m_pMainWindow = qtDLGNanomite::GetInstance();
	m_pSettings = clsAppSettings::SharedInstance();

#ifdef _AMD64_
	lblCurrentJIT->setText("x64");
	lblCurrentJITWOW64->setText("x86");
	lblDefaultJIT->setText("x64");
	lblDefaultJITWOW64->setText("x86");

#else
	lblCurrentJIT->setText("x86");
	lblCurrentJITWOW64->setText("x86");
	lblDefaultJIT->setText("x86");
	lblDefaultJITWOW64->setText("x86");

	lineCurrentWOW64->setEnabled(false);
	lineOrgWOW64->setEnabled(false);
#endif

	OnLoad();

	// Events for the GUI
	connect(tblCustomExceptions,SIGNAL(customContextMenuRequested(const QPoint &)),this,SLOT(OnRightClickCustomException(const QPoint &)));
	connect(btnClose,SIGNAL(clicked()),this,SLOT(OnClose()));
	connect(btnReload,SIGNAL(clicked()),this,SLOT(OnReload()));
	connect(btnSave,SIGNAL(clicked()),this,SLOT(OnSave()));
	connect(pbSetNanomite,SIGNAL(clicked()),this,SLOT(OnSetNanomiteDefault()));
	connect(pbRestoreOrg,SIGNAL(clicked()),this,SLOT(OnRestoreOrg()));
	connect(new QShortcut(QKeySequence(QKeySequence::Delete),this),SIGNAL(activated()),this,SLOT(OnExceptionRemove()));
}

qtDLGOption::~qtDLGOption()
{
}

void qtDLGOption::OnRightClickCustomException(const QPoint qPoint)
{
	QMenu menu;

	if(tblCustomExceptions->rowCount() <= 0)
	{
		menu.addAction(new QAction("Insert Exception",this));
	}
	else
	{
		m_selectedRow = tblCustomExceptions->indexAt(qPoint).row();
		if(m_selectedRow >= 0)
		{
			menu.addAction(new QAction("Insert Exception",this));
			menu.addAction(new QAction("Edit Exception",this));
			menu.addAction(new QAction("Delete Exception",this));
		}
		else
			menu.addAction(new QAction("Insert Exception",this));
	}

	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));
	menu.exec(QCursor::pos());
}

void qtDLGOption::OnClose()
{
	close();
}

void qtDLGOption::OnReload()
{
	m_pSettings->WriteDefaultSettings();
	m_pSettings->LoadDebuggerSettings(m_pMainWindow->coreDebugger);
	m_pSettings->LoadDisassemblerColor(m_pMainWindow->qtNanomiteDisAsColor);
	m_pSettings->LoadDefaultJITDebugger(m_originalJIT,m_originalJITWOW64);

	OnLoad();
}

void qtDLGOption::OnSave()
{
	QSettings newJIT("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows NT\\CurrentVersion\\AeDebug", QSettings::NativeFormat);
	newJIT.setValue("Debugger",lineCurrent->text());
	newJIT.setValue("Auto","0");
	newJIT.sync();

#ifdef _AMD64_
	QSettings newJITWOW64("HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows NT\\CurrentVersion\\AeDebug", QSettings::NativeFormat);
	newJITWOW64.setValue("Debugger",lineCurrentWOW64->text());
	newJITWOW64.setValue("Auto","0");
	newJITWOW64.sync();
#endif

//	if(newJIT.status() != QSettings::NoError)
//		MessageBoxW(NULL,L"ERROR, could not write the default jit!\r\nDo you have Admin rights?",L"Nanomite",MB_OK);
	
	if(cbExceptionAssist->isChecked())
		m_pMainWindow->coreDebugger->dbgSettings.bUseExceptionAssist = true;
	else
		m_pMainWindow->coreDebugger->dbgSettings.bUseExceptionAssist = false;

	if(cbModuleEP->isChecked())
		m_pMainWindow->coreDebugger->dbgSettings.bBreakOnModuleEP = true;
	else
		m_pMainWindow->coreDebugger->dbgSettings.bBreakOnModuleEP = false;

	if(cbSystemEP->isChecked())
		m_pMainWindow->coreDebugger->dbgSettings.bBreakOnSystemEP = true;
	else
		m_pMainWindow->coreDebugger->dbgSettings.bBreakOnSystemEP = false;

	if(cbTLS->isChecked())
		m_pMainWindow->coreDebugger->dbgSettings.bBreakOnTLS = true;
	else
		m_pMainWindow->coreDebugger->dbgSettings.bBreakOnTLS = false;

	if(cbLoadSym->isChecked())
		m_pMainWindow->coreDebugger->dbgSettings.bAutoLoadSymbols = true;
	else
		m_pMainWindow->coreDebugger->dbgSettings.bAutoLoadSymbols = false;

	if(cbDebugChild->isChecked())
		m_pMainWindow->coreDebugger->dbgSettings.bDebugChilds = true;
	else
		m_pMainWindow->coreDebugger->dbgSettings.bDebugChilds = false;

	if(cbSuspendThread->isChecked())
		m_pMainWindow->coreDebugger->dbgSettings.dwSuspendType = 1;
	else
		m_pMainWindow->coreDebugger->dbgSettings.dwSuspendType = 0;

	if(cbIgEx->isChecked())
		m_pMainWindow->coreDebugger->dbgSettings.dwDefaultExceptionMode = 1;
	else
		m_pMainWindow->coreDebugger->dbgSettings.dwDefaultExceptionMode = 0;

	if(cbBreakOnNewDLL->isChecked())
		m_pMainWindow->coreDebugger->dbgSettings.bBreakOnNewDLL = true;
	else
		m_pMainWindow->coreDebugger->dbgSettings.bBreakOnNewDLL = false;

	if(cbBreakOnNewTID->isChecked())
		m_pMainWindow->coreDebugger->dbgSettings.bBreakOnNewTID = true;	
	else
		m_pMainWindow->coreDebugger->dbgSettings.bBreakOnNewTID = false;

	if(cbBreakOnNewPID->isChecked())
		m_pMainWindow->coreDebugger->dbgSettings.bBreakOnNewPID = true;
	else
		m_pMainWindow->coreDebugger->dbgSettings.bBreakOnNewPID = false;

	if(cbBreakOnExDLL->isChecked())
		m_pMainWindow->coreDebugger->dbgSettings.bBreakOnExDLL = true;
	else
		m_pMainWindow->coreDebugger->dbgSettings.bBreakOnExDLL = false;

	if(cbBreakOnExTID->isChecked())
		m_pMainWindow->coreDebugger->dbgSettings.bBreakOnExTID = true;	
	else
		m_pMainWindow->coreDebugger->dbgSettings.bBreakOnExTID = false;

	if(cbBreakOnExPID->isChecked())
		m_pMainWindow->coreDebugger->dbgSettings.bBreakOnExPID = true;
	else
		m_pMainWindow->coreDebugger->dbgSettings.bBreakOnExPID = false;

	m_pMainWindow->coreDebugger->CustomExceptionRemoveAll();
	for(int i = 0; i < tblCustomExceptions->rowCount(); i++)
	{	
		if(tblCustomExceptions->item(i,0)->text().length() > 0 && tblCustomExceptions->item(i,1)->text().length() > 0)
		{
			m_pMainWindow->coreDebugger->CustomExceptionAdd(tblCustomExceptions->item(i,0)->text().toULong(0,16), tblCustomExceptions->item(i,1)->text().toInt(),NULL);
		}
	}

	m_pMainWindow->qtNanomiteDisAsColor->colorBP = comboBP->currentText();
	m_pMainWindow->qtNanomiteDisAsColor->colorCall = comboCall->currentText();
	m_pMainWindow->qtNanomiteDisAsColor->colorStack = comboStack->currentText();
	m_pMainWindow->qtNanomiteDisAsColor->colorJump = comboJump->currentText();
	m_pMainWindow->qtNanomiteDisAsColor->colorMove = comboMove->currentText();
	m_pMainWindow->qtNanomiteDisAsColor->colorMath = comboMath->currentText();

	m_pSettings->SaveDebuggerSettings(m_pMainWindow->coreDebugger);
	m_pSettings->SaveDisassemblerColor(m_pMainWindow->qtNanomiteDisAsColor);
	m_pSettings->SaveDefaultJITDebugger(m_originalJIT,m_originalJITWOW64);

	QMessageBox::information(this, "Nanomite", "Your settings have been saved!", QMessageBox::Ok, QMessageBox::Ok);
}

void qtDLGOption::OnLoad()
{
	m_pSettings->LoadDefaultJITDebugger(m_originalJIT, m_originalJITWOW64);
	
	if(m_pMainWindow->coreDebugger->dbgSettings.bAutoLoadSymbols)
		cbLoadSym->setChecked(true);
	else
		cbLoadSym->setChecked(false);

	if(m_pMainWindow->coreDebugger->dbgSettings.bDebugChilds)
		cbDebugChild->setChecked(true);
	else
		cbDebugChild->setChecked(false);

	if(m_pMainWindow->coreDebugger->dbgSettings.dwSuspendType)
		cbSuspendThread->setChecked(true);
	else
		cbSuspendThread->setChecked(false);
		
	if(m_pMainWindow->coreDebugger->dbgSettings.dwDefaultExceptionMode)
		cbIgEx->setChecked(true);
	else
		cbIgEx->setChecked(false);
	
	if(m_pMainWindow->coreDebugger->dbgSettings.bBreakOnNewDLL)
		cbBreakOnNewDLL->setChecked(true);
	else
		cbBreakOnNewDLL->setChecked(false);
		
	if(m_pMainWindow->coreDebugger->dbgSettings.bBreakOnNewTID)
		cbBreakOnNewTID->setChecked(true);
	else
		cbBreakOnNewTID->setChecked(false);

	if(m_pMainWindow->coreDebugger->dbgSettings.bBreakOnNewPID)
		cbBreakOnNewPID->setChecked(true);
	else
		cbBreakOnNewPID->setChecked(false);
	
	if(m_pMainWindow->coreDebugger->dbgSettings.bBreakOnExDLL)
		cbBreakOnExDLL->setChecked(true);
	else
		cbBreakOnExDLL->setChecked(false);
		
	if(m_pMainWindow->coreDebugger->dbgSettings.bBreakOnExTID)
		cbBreakOnExTID->setChecked(true);
	else
		cbBreakOnExTID->setChecked(false);
	
	if(m_pMainWindow->coreDebugger->dbgSettings.bBreakOnExPID)
		cbBreakOnExPID->setChecked(true);
	else
		cbBreakOnExPID->setChecked(false);
	
	if(m_pMainWindow->coreDebugger->dbgSettings.bBreakOnModuleEP)
		cbModuleEP->setChecked(true);
	else
		cbModuleEP->setChecked(false);
	
	if(m_pMainWindow->coreDebugger->dbgSettings.bBreakOnSystemEP)
		cbSystemEP->setChecked(true);
	else
		cbSystemEP->setChecked(false);
		
	if(m_pMainWindow->coreDebugger->dbgSettings.bBreakOnTLS)
		cbTLS->setChecked(true);
	else
		cbTLS->setChecked(false);

	if(m_pMainWindow->coreDebugger->dbgSettings.bUseExceptionAssist)
		cbExceptionAssist->setChecked(true);
	else
		cbExceptionAssist->setChecked(false);

	tblCustomExceptions->setRowCount(0);
	for(int i = 0; i < m_pMainWindow->coreDebugger->ExceptionHandler.size(); i++)
	{
		tblCustomExceptions->insertRow(tblCustomExceptions->rowCount());

		tblCustomExceptions->setItem(tblCustomExceptions->rowCount() - 1,0,
			new QTableWidgetItem(QString("%1").arg(m_pMainWindow->coreDebugger->ExceptionHandler[i].dwExceptionType,8,16,QChar('0'))));

		tblCustomExceptions->setItem(tblCustomExceptions->rowCount() - 1,1,
			new QTableWidgetItem(QString().sprintf("%d",m_pMainWindow->coreDebugger->ExceptionHandler[i].dwAction)));
	}

	int itemIndex = NULL;
	if((itemIndex = getIndex(m_pMainWindow->qtNanomiteDisAsColor->colorBP)) != -1)
		comboBP->setCurrentIndex(itemIndex);

	if((itemIndex = getIndex(m_pMainWindow->qtNanomiteDisAsColor->colorCall)) != -1)
		comboCall->setCurrentIndex(itemIndex);

	if((itemIndex = getIndex(m_pMainWindow->qtNanomiteDisAsColor->colorStack)) != -1)
		comboStack->setCurrentIndex(itemIndex);

	if((itemIndex = getIndex(m_pMainWindow->qtNanomiteDisAsColor->colorJump)) != -1)
		comboJump->setCurrentIndex(itemIndex);

	if((itemIndex = getIndex(m_pMainWindow->qtNanomiteDisAsColor->colorMove)) != -1)
		comboMove->setCurrentIndex(itemIndex);

	if((itemIndex = getIndex(m_pMainWindow->qtNanomiteDisAsColor->colorMath)) != -1)
		comboMath->setCurrentIndex(itemIndex);

	// Read JIT Settings
	QSettings JIT("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows NT\\CurrentVersion\\AeDebug",QSettings::NativeFormat);
	QString JITString = JIT.value("Debugger",0).toString();
	if(!JITString.contains("Nanomite.exe"))
	{
		lineOrg->setText(JITString);
		lineCurrent->setText(JITString);
		m_originalJIT = JITString;
		// save org jit to settings
	}
	else
	{
		lineOrg->setText(m_originalJIT);
		lineCurrent->setText(JITString);
	}

#ifdef _AMD64_
	QSettings JITWOW64("HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows NT\\CurrentVersion\\AeDebug", QSettings::NativeFormat);
	QString WOW64JITString = JITWOW64.value("Debugger",0).toString();
	if(!WOW64JITString.contains("Nanomite.exe"))
	{
		lineOrgWOW64->setText(WOW64JITString);
		lineCurrentWOW64->setText(WOW64JITString);
		m_originalJITWOW64 = WOW64JITString;
		// save org jit to settings
	}
	else
	{
		lineOrgWOW64->setText(m_originalJITWOW64);
		lineCurrentWOW64->setText(WOW64JITString);
	}
#endif
}

int qtDLGOption::getIndex(QString itemColor)
{
	if(itemColor.compare("White") == 0)
		return 0;
	else if(itemColor.compare("Black") == 0)
		return 1;
	else if(itemColor.compare("Red") == 0)
		return 2;
	else if(itemColor.compare("Green") == 0)
		return 3;
	else if(itemColor.compare("Dark green") == 0)
		return 4;
	else if(itemColor.compare("Blue") == 0)
		return 5;
	else if(itemColor.compare("Dark blue") == 0)
		return 6;
	else if(itemColor.compare("Cyan") == 0)
		return 7;
	else if(itemColor.compare("Dark cyan") == 0)
		return 8;
	else if(itemColor.compare("Magenta") == 0)
		return 9;
	else if(itemColor.compare("Dark magenta") == 0)
		return 10;
	else if(itemColor.compare("Yellow") == 0)
		return 11;
	else if(itemColor.compare("Dark yellow") == 0)
		return 12;
	else if(itemColor.compare("Gray") == 0)
		return 13;
	else if(itemColor.compare("Dark gray") == 0)
		return 14;
	else if(itemColor.compare("Light gray") == 0)
		return 15;
	return 0;
}

void qtDLGOption::OnSetNanomiteDefault()
{
	lineCurrent->setText(QString("\"%1\" %2").arg(QCoreApplication::applicationFilePath()).arg("-p %ld"));
#ifdef _AMD64_
	lineCurrentWOW64->setText(QString("\"%1\" %2").arg(QCoreApplication::applicationFilePath()).arg("-p %ld"));
#endif
}

void qtDLGOption::OnRestoreOrg()
{
	lineCurrent->setText(lineOrg->text());
#ifdef _AMD64_
	lineCurrentWOW64->setText(lineOrgWOW64->text());
#endif
}

void qtDLGOption::OnExceptionRemove()
{
	QList<QTableWidgetItem *>selectedItems = tblCustomExceptions->selectedItems();
	if(selectedItems.count() <= 0) return;

	for(int i = 0; i < tblCustomExceptions->rowCount(); i++)
	{
		if(tblCustomExceptions->item(i,0)->text().toULong(0,16) == selectedItems.value(0)->text().toULong(0,16))
			tblCustomExceptions->removeRow(i);
	}
}

void qtDLGOption::MenuCallback(QAction* pAction)
{
	if(QString().compare(pAction->text(),"Insert Exception") == 0)
	{
		qtDLGExceptionEdit newExceptionEdit(this,Qt::Window);
		connect(&newExceptionEdit,SIGNAL(OnInsertNewException(DWORD,int)),this,SLOT(OnInsertNewException(DWORD,int)));
		newExceptionEdit.exec();
	}
	else if(QString().compare(pAction->text(),"Edit Exception") == 0)
	{
		qtDLGExceptionEdit newExceptionEdit(this,Qt::Window,tblCustomExceptions->item(m_selectedRow,0)->text().toULong(0,16),
			tblCustomExceptions->item(m_selectedRow,1)->text().toInt());
		connect(&newExceptionEdit,SIGNAL(OnInsertNewException(DWORD,int)),this,SLOT(OnInsertNewException(DWORD,int)));
		newExceptionEdit.exec();
	}
	else if(QString().compare(pAction->text(),"Delete Exception") == 0)
	{
		OnExceptionRemove();
	}
}

void qtDLGOption::OnInsertNewException(DWORD exceptionCode, int handleException)
{
	for(int i = 0; i < tblCustomExceptions->rowCount(); i++)
	{
		if(tblCustomExceptions->item(i,0)->text().toULong(0,16) == exceptionCode)
		{
			tblCustomExceptions->item(i,1)->setText(QString("%1").arg(handleException));
			return;
		}
	}

	tblCustomExceptions->insertRow(tblCustomExceptions->rowCount());
	tblCustomExceptions->setItem(tblCustomExceptions->rowCount() - 1,0,	new QTableWidgetItem(QString("%1").arg(exceptionCode,8,16,QChar('0'))));
	tblCustomExceptions->setItem(tblCustomExceptions->rowCount() - 1,1,	new QTableWidgetItem(QString("%1").arg(handleException)));
}