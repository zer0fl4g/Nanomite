#include "qtDLGOption.h"
#include "qtDLGNanomite.h"

#include "clsHelperClass.h"

qtDLGOption::qtDLGOption(QWidget *parent, Qt::WFlags flags)
	: QDialog(parent, flags)
{
	setupUi(this);
	this->setFixedSize(this->width(),this->height());

	tblCustomExceptions->insertRow(tblCustomExceptions->rowCount());
	tblCustomExceptions->setItem(tblCustomExceptions->rowCount() - 1,0,new QTableWidgetItem(""));
	tblCustomExceptions->setItem(tblCustomExceptions->rowCount() - 1,1,new QTableWidgetItem(""));

	OnLoad();

	// Events for the GUI
	connect(tblCustomExceptions,SIGNAL(customContextMenuRequested(const QPoint &)),this,SLOT(OnRightClickCustomException(const QPoint &)));
	connect(btnClose,SIGNAL(clicked()),this,SLOT(OnClose()));
	connect(btnReload,SIGNAL(clicked()),this,SLOT(OnReload()));
	connect(btnSave,SIGNAL(clicked()),this,SLOT(OnSave()));
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

	myMainWindow->coreDebugger->dbgSettings.dwBreakOnEPMode = 0;
	myMainWindow->coreDebugger->dbgSettings.bAutoLoadSymbols = true;
	myMainWindow->coreDebugger->dbgSettings.bDebugChilds = true;
	myMainWindow->coreDebugger->dbgSettings.dwSuspendType = 0;
	myMainWindow->coreDebugger->dbgSettings.dwDefaultExceptionMode = 0;

	myMainWindow->coreDebugger->CustomExceptionRemoveAll();
	myMainWindow->coreDebugger->CustomExceptionAdd(EXCEPTION_ACCESS_VIOLATION,1,NULL);
	myMainWindow->coreDebugger->CustomExceptionAdd(EXCEPTION_PRIV_INSTRUCTION,1,NULL);
	myMainWindow->coreDebugger->CustomExceptionAdd(EXCEPTION_ILLEGAL_INSTRUCTION,1,NULL);
	myMainWindow->coreDebugger->CustomExceptionAdd(EXCEPTION_INT_DIVIDE_BY_ZERO,1,NULL);

	rbModuleEP->setChecked(true);
	cbLoadSym->setChecked(true);
	cbDebugChild->setChecked(true);
	cbSuspendThread->setChecked(false);
	cbIgEx->setChecked(false);

	cbIG_AVIOL->setChecked(true);
	cbInvPriv->setChecked(true);
	cbDivZero->setChecked(true);

	clsHelperClass::WriteToSettingsFile(myMainWindow->coreDebugger);

	OnLoad();
}

void qtDLGOption::OnSave()
{
	qtDLGNanomite* myMainWindow = qtDLGNanomite::GetInstance();

	if(rbSystemEP->isChecked())
		myMainWindow->coreDebugger->dbgSettings.dwBreakOnEPMode = 1;
	else if(rbModuleEP->isChecked())
		myMainWindow->coreDebugger->dbgSettings.dwBreakOnEPMode = 0;
	else if(rbTlsCallback->isChecked())
		myMainWindow->coreDebugger->dbgSettings.dwBreakOnEPMode = 2;
	else if(rbDirect->isChecked())
		myMainWindow->coreDebugger->dbgSettings.dwBreakOnEPMode = 3;

	if(cbLoadSym->isChecked())
		myMainWindow->coreDebugger->dbgSettings.bAutoLoadSymbols = true;
	if(cbDebugChild->isChecked())
		myMainWindow->coreDebugger->dbgSettings.bDebugChilds = true;
	if(cbSuspendThread->isChecked())
		myMainWindow->coreDebugger->dbgSettings.dwSuspendType = 0x1;
	if(cbIgEx->isChecked())
		myMainWindow->coreDebugger->dbgSettings.dwDefaultExceptionMode = 0x1;

	myMainWindow->coreDebugger->CustomExceptionRemoveAll();
	if(cbIG_AVIOL->isChecked())
		myMainWindow->coreDebugger->CustomExceptionAdd(EXCEPTION_ACCESS_VIOLATION,1,NULL);
	if(cbDivZero->isChecked())
		myMainWindow->coreDebugger->CustomExceptionAdd(EXCEPTION_PRIV_INSTRUCTION,1,NULL);
	if(cbInvPriv->isChecked())
	{
		myMainWindow->coreDebugger->CustomExceptionAdd(EXCEPTION_ILLEGAL_INSTRUCTION,1,NULL);
		myMainWindow->coreDebugger->CustomExceptionAdd(EXCEPTION_INT_DIVIDE_BY_ZERO,1,NULL);
	}

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
				MessageBox(NULL,QString().sprintf("The exception : %08X does already exists!",tblCustomExceptions->item(i,0)->text().toULong()).toStdWString().c_str(),
					L"Nanomite - Option",MB_OK);
		}
	}

	clsHelperClass::WriteToSettingsFile(myMainWindow->coreDebugger);
	MessageBox(NULL,L"Your settings have been saved!",L"Nanomite - Option",MB_OK);
}

void qtDLGOption::OnLoad()
{
	qtDLGNanomite* myMainWindow = qtDLGNanomite::GetInstance();
	clsHelperClass::ReadFromSettingsFile(myMainWindow->coreDebugger);

	switch(myMainWindow->coreDebugger->dbgSettings.dwBreakOnEPMode)
	{
	case 0:
		rbModuleEP->setChecked(true);
		break;
	case 1:
		rbSystemEP->setChecked(true);
		break;
	case 2:
		rbTlsCallback->setChecked(true);
		break;
	case 3:
		rbDirect->setChecked(true);
		break;
	}

	if(myMainWindow->coreDebugger->dbgSettings.bAutoLoadSymbols)
		cbLoadSym->setChecked(true);
	if(myMainWindow->coreDebugger->dbgSettings.bDebugChilds);
		cbDebugChild->setChecked(true);
	if(myMainWindow->coreDebugger->dbgSettings.dwSuspendType)
		cbSuspendThread->setChecked(true);
	if(myMainWindow->coreDebugger->dbgSettings.dwDefaultExceptionMode)
		cbIgEx->setChecked(true);

	for(size_t i = 0;i < myMainWindow->coreDebugger->ExceptionHandler.size();i++)
	{
		if(myMainWindow->coreDebugger->ExceptionHandler[i].dwExceptionType == EXCEPTION_ACCESS_VIOLATION)
			cbIG_AVIOL->setChecked(true);
		else if(myMainWindow->coreDebugger->ExceptionHandler[i].dwExceptionType == EXCEPTION_PRIV_INSTRUCTION)
			cbInvPriv->setChecked(true);
		else if(myMainWindow->coreDebugger->ExceptionHandler[i].dwExceptionType == EXCEPTION_ILLEGAL_INSTRUCTION)
			cbInvPriv->setChecked(true);
		else if(myMainWindow->coreDebugger->ExceptionHandler[i].dwExceptionType == EXCEPTION_INT_DIVIDE_BY_ZERO)
			cbDivZero->setChecked(true);
		else
		{
			tblCustomExceptions->insertRow(tblCustomExceptions->rowCount());

			tblCustomExceptions->setItem(tblCustomExceptions->rowCount() - 1,0,
				new QTableWidgetItem(QString().sprintf("%08X",myMainWindow->coreDebugger->ExceptionHandler[i].dwExceptionType)));

			tblCustomExceptions->setItem(tblCustomExceptions->rowCount() - 1,1,
				new QTableWidgetItem(QString().sprintf("%d",myMainWindow->coreDebugger->ExceptionHandler[i].dwAction)));
		}
	}
}