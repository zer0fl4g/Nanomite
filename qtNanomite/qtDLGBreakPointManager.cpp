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
#include "qtDLGBreakPointManager.h"
#include "qtDLGNanomite.h"

#include "clsHelperClass.h"
#include "clsMemManager.h"

using namespace std;

qtDLGBreakPointManager *qtDLGBreakPointManager::pThis = NULL;

qtDLGBreakPointManager::qtDLGBreakPointManager(QWidget *parent, Qt::WFlags flags)
	: QWidget(parent, flags)
{
	setupUi(this);
	this->setFixedSize(this->width(),this->height());

	pThis = this;

	// List BP Manager
	tblBPs->horizontalHeader()->resizeSection(0,75);
	tblBPs->horizontalHeader()->resizeSection(1,135);
	tblBPs->horizontalHeader()->resizeSection(2,135);
	tblBPs->horizontalHeader()->resizeSection(3,50);
	tblBPs->horizontalHeader()->setFixedHeight(21);

	connect(pbClose,SIGNAL(clicked()),this,SLOT(OnClose()));
	connect(pbAddUpdate,SIGNAL(clicked()),this,SLOT(OnAddUpdate()));
	connect(tblBPs,SIGNAL(cellClicked(int,int)),this,SLOT(OnSelectedBPChanged(int,int)));
	connect(new QShortcut(QKeySequence(QKeySequence::Delete),this),SIGNAL(activated()),this,SLOT(OnBPRemove()));
	connect(new QShortcut(Qt::Key_Escape,this),SIGNAL(activated()),this,SLOT(close()));

	m_pAPICompleter = new QCompleter(m_completerList, this);
}

qtDLGBreakPointManager::~qtDLGBreakPointManager()
{

}

void qtDLGBreakPointManager::OnClose()
{
	close();
}

void qtDLGBreakPointManager::OnUpdate(BPStruct newBP,int breakpointType)
{
	if(newBP.dwHandle == 0x1)
	{
		tblBPs->insertRow(tblBPs->rowCount());

		if(newBP.dwPID == -1)
			tblBPs->setItem(tblBPs->rowCount() - 1,0,new QTableWidgetItem(QString("%1").arg(newBP.dwPID)));
		else
			tblBPs->setItem(tblBPs->rowCount() - 1,0,new QTableWidgetItem(QString("%1").arg(newBP.dwPID,0,16)));
		tblBPs->setItem(tblBPs->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(newBP.dwOffset,16,16,QChar('0'))));

		QString TempString;
		switch(breakpointType)
		{
		case 0:
			TempString = "Software BP - int3";
			break;
		case 1:
			TempString = "Memory BP - Page Guard";
			break;
		case 2:
			TempString = "Hardware BP - Dr[0-3]";
			break;
		}
		tblBPs->setItem(tblBPs->rowCount() - 1,2,new QTableWidgetItem(TempString));

		tblBPs->setItem(tblBPs->rowCount() - 1,3,new QTableWidgetItem(QString("%1").arg(newBP.dwSize,2,16,QChar('0'))));

		switch(newBP.dwTypeFlag)
		{
		case DR_EXECUTE:
			TempString = "Execute";
			break;
		case DR_READ:
			TempString = "Read";
			break;
		case DR_WRITE:
			TempString = "Write";
			break;
		}
		tblBPs->setItem(tblBPs->rowCount() - 1,4,new QTableWidgetItem(TempString));
	}
	else if(newBP.dwHandle == 0x3)
	{ // BP got new Offset
		for(int i = 0; i < tblBPs->rowCount(); i++)
		{
			if(tblBPs->item(i,1)->text().toULongLong(0,16) == newBP.dwOldOffset)
			{
				tblBPs->removeRow(i);
			}
		}
		newBP.dwHandle = 0x1;

		OnUpdate(newBP,breakpointType);
	}
}

void qtDLGBreakPointManager::OnAddUpdate()
{
	int iUpdateLine = -1;
	qtDLGNanomite *myMainWindow = qtDLGNanomite::GetInstance();

	if(leOffset->text().contains("::"))
	{
		QStringList SplitAPIList = leOffset->text().split("::");

		if(SplitAPIList.count() >= 2)
		{
			quint64 dwOffset = clsHelperClass::CalcOffsetForModule((PTCHAR)SplitAPIList[0].toLower().toStdWString().c_str(),NULL,lePID->text().toULong(0,16));
			dwOffset = clsHelperClass::RemoteGetProcAddr(SplitAPIList[0],SplitAPIList[1],dwOffset,lePID->text().toULong(0,16));

			if(dwOffset <= 0)
			{
				MessageBoxW(NULL,L"Please use a correct API Name!",L"Nanomite - Breakpoint Manager",MB_OK);
				return;
			}
			else
				leOffset->setText(QString("%1").arg(dwOffset,16,16,QChar('0')));
		}
	}

	for(int i = 0; i < tblBPs->rowCount(); i++)
		if(QString().compare(tblBPs->item(i,1)->text(),leOffset->text()) == 0)
			iUpdateLine = i;		

	if(iUpdateLine != -1)
	{
		DWORD dwType = 0;
		if(QString().compare(tblBPs->item(iUpdateLine,2)->text(),"Software BP - int3") == 0)
			dwType = 0;
		else if(QString().compare(tblBPs->item(iUpdateLine,2)->text(),"Hardware BP - Dr[0-3]") == 0)
			dwType = 2;
		else if(QString().compare(tblBPs->item(iUpdateLine,2)->text(),"Memory BP - Page Guard") == 0)
			dwType = 1;

		myMainWindow->coreDebugger->RemoveBPFromList(tblBPs->item(iUpdateLine,1)->text().toULongLong(0,16),dwType);
		tblBPs->removeRow(iUpdateLine);
	}

	DWORD dwType = 0,
		dwBreakOn = 0;

	if(cbType->currentText().compare("Software BP - int3") == 0)
		dwType = 0;
	else if(cbType->currentText().compare("Hardware BP - Dr[0-3]") == 0)
		dwType = 2;
	else if(cbType->currentText().compare("Memory BP - Page Guard") == 0)
		dwType = 1;

	if(cbBreakOn->currentText().compare("Execute") == 0)
		dwBreakOn = DR_EXECUTE;
	else if(cbBreakOn->currentText().compare("Read") == 0)
		dwBreakOn = DR_READ;
	else if(cbBreakOn->currentText().compare("Write") == 0)
		dwBreakOn = DR_WRITE;

	if(lePID->text().toInt() == -1)
		myMainWindow->coreDebugger->AddBreakpointToList(dwType,dwBreakOn,lePID->text().toInt(),leOffset->text().toULongLong(0,16),0,true);
	else
		myMainWindow->coreDebugger->AddBreakpointToList(dwType,dwBreakOn,lePID->text().toInt(0,16),leOffset->text().toULongLong(0,16),0,true);
}

void qtDLGBreakPointManager::OnSelectedBPChanged(int iRow,int iCol)
{
	lePID->setText(tblBPs->item(iRow,0)->text());
	leOffset->setText(tblBPs->item(iRow,1)->text());
	leSize->setText(tblBPs->item(iRow,3)->text());

	if(QString().compare(tblBPs->item(iRow,2)->text(),"Software BP - int3") == 0)
		cbType->setCurrentIndex(0);
	else if(QString().compare(tblBPs->item(iRow,2)->text(),"Hardware BP - Dr[0-3]") == 0)
		cbType->setCurrentIndex(1);
	else if(QString().compare(tblBPs->item(iRow,2)->text(),"Memory BP - Page Guard") == 0)
		cbType->setCurrentIndex(2);

	if(QString().compare(tblBPs->item(iRow,4)->text(),"Execute") == 0)
		cbBreakOn->setCurrentIndex(0);
	else if(QString().compare(tblBPs->item(iRow,4)->text(),"Read") == 0)
		cbBreakOn->setCurrentIndex(2);
	else if(QString().compare(tblBPs->item(iRow,4)->text(),"Write") == 0)
		cbBreakOn->setCurrentIndex(1);
}

void qtDLGBreakPointManager::OnBPRemove()
{
	qtDLGNanomite *myMainWindow = qtDLGNanomite::GetInstance();
	DWORD dwType = 0;

	for(int i = 0; i < tblBPs->rowCount(); i++)
	{
		if(tblBPs->item(i,0)->isSelected())
		{
			if(QString().compare(tblBPs->item(i,2)->text(),"Software BP - int3") == 0)
				dwType = 0;
			else if(QString().compare(tblBPs->item(i,2)->text(),"Hardware BP - Dr[0-3]") == 0)
				dwType = 2;
			else if(QString().compare(tblBPs->item(i,2)->text(),"Memory BP - Page Guard") == 0)
				dwType = 1;

			myMainWindow->coreDebugger->RemoveBPFromList(tblBPs->item(i,1)->text().toULongLong(0,16),dwType);
			tblBPs->removeRow(i);
			i = 0;
		}
	}
}

void qtDLGBreakPointManager::UpdateCompleter(wstring FilePath,int processID)
{
	QList<APIData> newImports = clsPEManager::getImportsFromFile(FilePath);

	for(int i = 0; i < newImports.size(); i++)
	{
		m_completerList.append(newImports.value(i).APIName);
	}

	delete m_pAPICompleter;
	m_pAPICompleter = new QCompleter(m_completerList, this);

	m_pAPICompleter->setCaseSensitivity(Qt::CaseInsensitive);
	leOffset->setCompleter(m_pAPICompleter);
}

void qtDLGBreakPointManager::DeleteCompleterContent()
{
	m_completerList.clear();
	delete m_pAPICompleter;
	m_pAPICompleter = new QCompleter(m_completerList, this);
}

void qtDLGBreakPointManager::OnDelete(quint64 breakpointOffset)
{
	for(int i = 0; i < tblBPs->rowCount(); i++)
	{
		if(tblBPs->item(i,1)->text().toULongLong(0,16) == breakpointOffset)
		{
			tblBPs->removeRow(i);
			i = 0;
		}
	}
}

QStringList qtDLGBreakPointManager::ReturnCompleterList()
{
	return pThis->m_completerList;
}