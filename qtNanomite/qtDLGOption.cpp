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

#include "clsMemManager.h"
#include "clsHelperClass.h"

qtDLGOption::qtDLGOption(QWidget *parent, Qt::WFlags flags)
	: QDialog(parent, flags)
{
	setupUi(this);
	this->setFixedSize(this->width(),this->height());
	this->setStyleSheet(clsHelperClass::LoadStyleSheet());
	//this->setAttribute(Qt::WA_DeleteOnClose,true);

	tblCustomExceptions->insertRow(tblCustomExceptions->rowCount());
	tblCustomExceptions->setItem(tblCustomExceptions->rowCount() - 1,0,new QTableWidgetItem(""));
	tblCustomExceptions->setItem(tblCustomExceptions->rowCount() - 1,1,new QTableWidgetItem(""));

	OnLoad();

	// Events for the GUI
	//connect(tblCustomExceptions,SIGNAL(customContextMenuRequested(const QPoint &)),this,SLOT(OnRightClickCustomException(const QPoint &)));
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

void qtDLGOption::OnRightClickCustomException(const QPoint &)
{
	tblCustomExceptions->insertRow(tblCustomExceptions->rowCount());
	tblCustomExceptions->setItem(tblCustomExceptions->rowCount() - 1,0,new QTableWidgetItem(""));
	tblCustomExceptions->setItem(tblCustomExceptions->rowCount() - 1,1,new QTableWidgetItem(""));

	return;
}

void qtDLGOption::OnClose()
{
	close();
}

void qtDLGOption::OnReload()
{
	qtDLGNanomite* myMainWindow = qtDLGNanomite::GetInstance();

	myMainWindow->coreDebugger->dbgSettings.bBreakOnSystemEP = false;
	myMainWindow->coreDebugger->dbgSettings.bBreakOnTLS = false;
	myMainWindow->coreDebugger->dbgSettings.bBreakOnModuleEP = true;
	myMainWindow->coreDebugger->dbgSettings.bAutoLoadSymbols = true;
	myMainWindow->coreDebugger->dbgSettings.bDebugChilds = true;
	myMainWindow->coreDebugger->dbgSettings.dwSuspendType = 0;
	myMainWindow->coreDebugger->dbgSettings.dwDefaultExceptionMode = 0;
	myMainWindow->coreDebugger->dbgSettings.bBreakOnNewDLL = false;
	myMainWindow->coreDebugger->dbgSettings.bBreakOnNewPID = false;
	myMainWindow->coreDebugger->dbgSettings.bBreakOnNewTID = false;
	myMainWindow->coreDebugger->dbgSettings.bBreakOnExDLL = false;
	myMainWindow->coreDebugger->dbgSettings.bBreakOnExPID = false;
	myMainWindow->coreDebugger->dbgSettings.bBreakOnExTID = false;

	myMainWindow->coreDebugger->CustomExceptionRemoveAll();
	myMainWindow->coreDebugger->CustomExceptionAdd(EXCEPTION_ACCESS_VIOLATION,1,NULL);
	myMainWindow->coreDebugger->CustomExceptionAdd(EXCEPTION_PRIV_INSTRUCTION,1,NULL);
	myMainWindow->coreDebugger->CustomExceptionAdd(EXCEPTION_ILLEGAL_INSTRUCTION,1,NULL);
	myMainWindow->coreDebugger->CustomExceptionAdd(EXCEPTION_INT_DIVIDE_BY_ZERO,1,NULL);
	tblCustomExceptions->setRowCount(0);
	tblCustomExceptions->insertRow(tblCustomExceptions->rowCount());
	tblCustomExceptions->setItem(tblCustomExceptions->rowCount() - 1,0,	new QTableWidgetItem(""));
	tblCustomExceptions->setItem(tblCustomExceptions->rowCount() - 1,1,	new QTableWidgetItem(""));

	cbModuleEP->setChecked(false);
	cbSystemEP->setChecked(false);
	cbTLS->setChecked(false);
	cbLoadSym->setChecked(true);
	cbDebugChild->setChecked(true);
	cbSuspendThread->setChecked(false);
	cbIgEx->setChecked(false);
	cbBreakOnNewDLL->setChecked(false);
	cbBreakOnNewTID->setChecked(false);
	cbBreakOnNewPID->setChecked(false);
	cbBreakOnExDLL->setChecked(false);
	cbBreakOnExTID->setChecked(false);
	cbBreakOnExPID->setChecked(false);

	myMainWindow->qtNanomiteDisAsColor->colorBP = "Red";
	myMainWindow->qtNanomiteDisAsColor->colorCall = "Green";
	myMainWindow->qtNanomiteDisAsColor->colorStack = "Dark green";
	myMainWindow->qtNanomiteDisAsColor->colorJump = "Blue";
	myMainWindow->qtNanomiteDisAsColor->colorMove = "Gray";
	myMainWindow->qtNanomiteDisAsColor->colorMath = "Magenta";

	comboBP->setCurrentIndex(2);
	comboCall->setCurrentIndex(3);
	comboJump->setCurrentIndex(5);
	comboMove->setCurrentIndex(13);
	comboStack->setCurrentIndex(4);
	comboMath->setCurrentIndex(9);

	clsHelperClass::WriteToSettingsFile(myMainWindow->coreDebugger,myMainWindow->qtNanomiteDisAsColor,m_originalJIT);

	OnLoad();
}

void qtDLGOption::OnSave()
{
	QSettings newJIT("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows NT\\CurrentVersion\\AeDebug",QSettings::NativeFormat);
	newJIT.setValue("Debugger",lineCurrent->text());
	newJIT.setValue("Auto","0");
	newJIT.sync();
	if(newJIT.status() != QSettings::NoError)
		MessageBoxW(NULL,L"ERROR, could not write the default jit!\r\nDo you have Admin rights?",L"Nanomite",MB_OK);
	
	qtDLGNanomite* myMainWindow = qtDLGNanomite::GetInstance();

	if(cbModuleEP->isChecked())
		myMainWindow->coreDebugger->dbgSettings.bBreakOnModuleEP = true;
	else
		myMainWindow->coreDebugger->dbgSettings.bBreakOnModuleEP = false;

	if(cbSystemEP->isChecked())
		myMainWindow->coreDebugger->dbgSettings.bBreakOnSystemEP = true;
	else
		myMainWindow->coreDebugger->dbgSettings.bBreakOnSystemEP = false;

	if(cbTLS->isChecked())
		myMainWindow->coreDebugger->dbgSettings.bBreakOnTLS = true;
	else
		myMainWindow->coreDebugger->dbgSettings.bBreakOnTLS = false;

	if(cbLoadSym->isChecked())
		myMainWindow->coreDebugger->dbgSettings.bAutoLoadSymbols = true;
	else
		myMainWindow->coreDebugger->dbgSettings.bAutoLoadSymbols = false;

	if(cbDebugChild->isChecked())
		myMainWindow->coreDebugger->dbgSettings.bDebugChilds = true;
	else
		myMainWindow->coreDebugger->dbgSettings.bDebugChilds = false;

	if(cbSuspendThread->isChecked())
		myMainWindow->coreDebugger->dbgSettings.dwSuspendType = 1;
	else
		myMainWindow->coreDebugger->dbgSettings.dwSuspendType = 0;

	if(cbIgEx->isChecked())
		myMainWindow->coreDebugger->dbgSettings.dwDefaultExceptionMode = 1;
	else
		myMainWindow->coreDebugger->dbgSettings.dwDefaultExceptionMode = 0;

	if(cbBreakOnNewDLL->isChecked())
		myMainWindow->coreDebugger->dbgSettings.bBreakOnNewDLL = true;
	else
		myMainWindow->coreDebugger->dbgSettings.bBreakOnNewDLL = false;

	if(cbBreakOnNewTID->isChecked())
		myMainWindow->coreDebugger->dbgSettings.bBreakOnNewTID = true;	
	else
		myMainWindow->coreDebugger->dbgSettings.bBreakOnNewTID = false;

	if(cbBreakOnNewPID->isChecked())
		myMainWindow->coreDebugger->dbgSettings.bBreakOnNewPID = true;
	else
		myMainWindow->coreDebugger->dbgSettings.bBreakOnNewPID = false;

	if(cbBreakOnExDLL->isChecked())
		myMainWindow->coreDebugger->dbgSettings.bBreakOnExDLL = true;
	else
		myMainWindow->coreDebugger->dbgSettings.bBreakOnExDLL = false;

	if(cbBreakOnExTID->isChecked())
		myMainWindow->coreDebugger->dbgSettings.bBreakOnExTID = true;	
	else
		myMainWindow->coreDebugger->dbgSettings.bBreakOnExTID = false;

	if(cbBreakOnExPID->isChecked())
		myMainWindow->coreDebugger->dbgSettings.bBreakOnExPID = true;
	else
		myMainWindow->coreDebugger->dbgSettings.bBreakOnExPID = false;

	myMainWindow->coreDebugger->CustomExceptionRemoveAll();
	for(int i = 0; i < tblCustomExceptions->rowCount(); i++)
	{	
		if(tblCustomExceptions->item(i,0)->text().length() > 0 && tblCustomExceptions->item(i,1)->text().length() > 0)
		{
			bool bExists = false;
			for(int iCheck = 0; iCheck < myMainWindow->coreDebugger->ExceptionHandler.size(); iCheck++)
			{
				if(myMainWindow->coreDebugger->ExceptionHandler[iCheck].dwExceptionType == tblCustomExceptions->item(i,0)->text().toULong())
					bExists = true;
			}

			if(!bExists)
				myMainWindow->coreDebugger->CustomExceptionAdd(tblCustomExceptions->item(i,0)->text().toULong(0,16),
				tblCustomExceptions->item(i,1)->text().toInt(),NULL);
			else
				MessageBox(NULL,QString("The exception : %1 does already exists!").arg(tblCustomExceptions->item(i,0)->text().toULong(0,16)).toStdWString().c_str(),
				L"Nanomite - Option",MB_OK);
		}
	}

	myMainWindow->qtNanomiteDisAsColor->colorBP = comboBP->currentText();
	myMainWindow->qtNanomiteDisAsColor->colorCall = comboCall->currentText();
	myMainWindow->qtNanomiteDisAsColor->colorStack = comboStack->currentText();
	myMainWindow->qtNanomiteDisAsColor->colorJump = comboJump->currentText();
	myMainWindow->qtNanomiteDisAsColor->colorMove = comboMove->currentText();
	myMainWindow->qtNanomiteDisAsColor->colorMath = comboMath->currentText();

	if(clsHelperClass::WriteToSettingsFile(myMainWindow->coreDebugger,myMainWindow->qtNanomiteDisAsColor,m_originalJIT))
		MessageBox(NULL,L"Your settings have been saved!",L"Nanomite - Option",MB_OK);
}

void qtDLGOption::OnLoad()
{
	qtDLGNanomite* myMainWindow = qtDLGNanomite::GetInstance();
	if(!m_NoRead)
		clsHelperClass::ReadFromSettingsFile(myMainWindow->coreDebugger,myMainWindow->qtNanomiteDisAsColor,m_originalJIT);
	
	if(myMainWindow->coreDebugger->dbgSettings.bAutoLoadSymbols)
		cbLoadSym->setChecked(true);
	if(myMainWindow->coreDebugger->dbgSettings.bDebugChilds)
		cbDebugChild->setChecked(true);
	if(myMainWindow->coreDebugger->dbgSettings.dwSuspendType)
		cbSuspendThread->setChecked(true);
	if(myMainWindow->coreDebugger->dbgSettings.dwDefaultExceptionMode)
		cbIgEx->setChecked(true);
	if(myMainWindow->coreDebugger->dbgSettings.bBreakOnNewDLL)
		cbBreakOnNewDLL->setChecked(true);
	if(myMainWindow->coreDebugger->dbgSettings.bBreakOnNewTID)
		cbBreakOnNewTID->setChecked(true);
	if(myMainWindow->coreDebugger->dbgSettings.bBreakOnNewPID)
		cbBreakOnNewPID->setChecked(true);
	if(myMainWindow->coreDebugger->dbgSettings.bBreakOnExDLL)
		cbBreakOnExDLL->setChecked(true);
	if(myMainWindow->coreDebugger->dbgSettings.bBreakOnExTID)
		cbBreakOnExTID->setChecked(true);
	if(myMainWindow->coreDebugger->dbgSettings.bBreakOnExPID)
		cbBreakOnExPID->setChecked(true);
	if(myMainWindow->coreDebugger->dbgSettings.bBreakOnModuleEP)
		cbModuleEP->setChecked(true);
	if(myMainWindow->coreDebugger->dbgSettings.bBreakOnSystemEP)
		cbSystemEP->setChecked(true);
	if(myMainWindow->coreDebugger->dbgSettings.bBreakOnTLS)
		cbTLS->setChecked(true);

	tblCustomExceptions->setRowCount(0);
	for(size_t i = 0;i < myMainWindow->coreDebugger->ExceptionHandler.size();i++)
	{
		tblCustomExceptions->insertRow(tblCustomExceptions->rowCount());

		tblCustomExceptions->setItem(tblCustomExceptions->rowCount() - 1,0,
			new QTableWidgetItem(QString("%1").arg(myMainWindow->coreDebugger->ExceptionHandler[i].dwExceptionType,8,16,QChar('0'))));

		tblCustomExceptions->setItem(tblCustomExceptions->rowCount() - 1,1,
			new QTableWidgetItem(QString().sprintf("%d",myMainWindow->coreDebugger->ExceptionHandler[i].dwAction)));
	}
	tblCustomExceptions->insertRow(tblCustomExceptions->rowCount());
	tblCustomExceptions->setItem(tblCustomExceptions->rowCount() - 1,0,	new QTableWidgetItem(""));
	tblCustomExceptions->setItem(tblCustomExceptions->rowCount() - 1,1,	new QTableWidgetItem(""));

	int itemIndex = NULL;
	if((itemIndex = getIndex(myMainWindow->qtNanomiteDisAsColor->colorBP)) != -1)
		comboBP->setCurrentIndex(itemIndex);

	if((itemIndex = getIndex(myMainWindow->qtNanomiteDisAsColor->colorCall)) != -1)
		comboCall->setCurrentIndex(itemIndex);

	if((itemIndex = getIndex(myMainWindow->qtNanomiteDisAsColor->colorStack)) != -1)
		comboStack->setCurrentIndex(itemIndex);

	if((itemIndex = getIndex(myMainWindow->qtNanomiteDisAsColor->colorJump)) != -1)
		comboJump->setCurrentIndex(itemIndex);

	if((itemIndex = getIndex(myMainWindow->qtNanomiteDisAsColor->colorMove)) != -1)
		comboMove->setCurrentIndex(itemIndex);

	if((itemIndex = getIndex(myMainWindow->qtNanomiteDisAsColor->colorMath)) != -1)
		comboMath->setCurrentIndex(itemIndex);

	// Read JIT Settings
	QSettings JIT("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows NT\\CurrentVersion\\AeDebug",QSettings::NativeFormat);
	QString JITString = JIT.value("Debugger",0).toString();
	if(!JITString.contains("Nanomite.exe"))
	{
		lineOrg->setText(JITString);
		lineCurrent->setText(JITString);
		m_originalJIT = JITString.toStdWString();
		// save org jit to settings
	}
	else
	{
		lineOrg->setText(QString::fromStdWString(m_originalJIT));
		lineCurrent->setText(JITString);
	}
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
}

void qtDLGOption::OnRestoreOrg()
{
	lineCurrent->setText(lineOrg->text());
}

void qtDLGOption::showEvent(QShowEvent *event)
{
	m_NoRead = true;
	OnLoad();
	m_NoRead = false;
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