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

	cbBreakOn->setEnabled(false);
	cbSize->setEnabled(true);

	// List BP Manager
	tblBPs->horizontalHeader()->resizeSection(0,75);
	tblBPs->horizontalHeader()->resizeSection(1,135);
	tblBPs->horizontalHeader()->resizeSection(2,135);
	tblBPs->horizontalHeader()->resizeSection(3,50);
	tblBPs->horizontalHeader()->setFixedHeight(21);

	connect(pbClose,SIGNAL(clicked()),this,SLOT(OnClose()));
	connect(pbAddUpdate,SIGNAL(clicked()),this,SLOT(OnAddUpdate()));
	connect(tblBPs,SIGNAL(cellClicked(int,int)),this,SLOT(OnSelectedBPChanged(int,int)));
	connect(tblBPs,SIGNAL(itemDoubleClicked(QTableWidgetItem *)),this,SLOT(OnSendToDisassembler(QTableWidgetItem *)));
	connect(cbType,SIGNAL(currentIndexChanged(const QString &)),this,SLOT(OnBPTypeSelectionChanged(const QString &)));
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
	if(newBP.dwHandle == BP_KEEP)
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
			TempString = "Software BP";
			break;
		case 1:
			TempString = "Memory BP";
			break;
		case 2:
			TempString = "Hardware BP";
			break;
		}
		tblBPs->setItem(tblBPs->rowCount() - 1,2,new QTableWidgetItem(TempString));

		tblBPs->setItem(tblBPs->rowCount() - 1,3,new QTableWidgetItem(QString("%1").arg(newBP.dwSize,2,16,QChar('0'))));

		switch(newBP.dwTypeFlag)
		{
		case BP_EXEC:
			TempString = "Execute";
			break;
		case BP_READ:
			TempString = "Read";
			break;
		case BP_WRITE:
			TempString = "Write";
			break;
		case BP_ACCESS:
			TempString = "Access";
			break;
		}
		tblBPs->setItem(tblBPs->rowCount() - 1,4,new QTableWidgetItem(TempString));
	}
	else if(newBP.dwHandle == BP_OFFSETUPDATE)
	{ // BP got new Offset
		for(int i = 0; i < tblBPs->rowCount(); i++)
		{
			if(tblBPs->item(i,1)->text().toULongLong(0,16) == newBP.dwOldOffset)
			{
				tblBPs->removeRow(i);
			}
		}
		newBP.dwHandle = BP_KEEP;

		OnUpdate(newBP,breakpointType);
	}
}

void qtDLGBreakPointManager::OnAddUpdate()
{
	int iUpdateLine = -1;
	quint64 dwOffset = NULL;

	if(leOffset->text().contains("::"))
	{
		QStringList SplitAPIList = leOffset->text().split("::");

		if(SplitAPIList.count() >= 2)
		{
			dwOffset = clsHelperClass::CalcOffsetForModule((PTCHAR)SplitAPIList[0].toLower().toStdWString().c_str(),NULL,lePID->text().toULong(0,16));
			dwOffset = clsHelperClass::RemoteGetProcAddr(SplitAPIList[1],dwOffset,lePID->text().toULong(0,16));

			if(dwOffset <= 0)
			{
				QMessageBox::critical(this,"Nanomite","Please use a correct API Name!",QMessageBox::Ok,QMessageBox::Ok);
				return;
			}

			leOffset->setText(QString("%1").arg(dwOffset,16,16,QChar('0')));
		}
	}
	else
	{
		dwOffset = leOffset->text().toULongLong(0,16);

		if(dwOffset <= 0)
		{
			QMessageBox::critical(this,"Nanomite","This offset seems to be invalid!",QMessageBox::Ok,QMessageBox::Ok);
			return;
		}	
	}

	for(int i = 0; i < tblBPs->rowCount(); i++)
	{
		if(QString().compare(tblBPs->item(i,1)->text(),leOffset->text()) == 0)
		{
			iUpdateLine = i;		
			break;
		}
	}

	if(iUpdateLine != -1)
	{
		DWORD dwType = 0;
		if(QString().compare(tblBPs->item(iUpdateLine,2)->text(),"Software BP") == 0)
			dwType = SOFTWARE_BP;
		else if(QString().compare(tblBPs->item(iUpdateLine,2)->text(),"Hardware BP") == 0)
			dwType = HARDWARE_BP;
		else if(QString().compare(tblBPs->item(iUpdateLine,2)->text(),"Memory BP") == 0)
			dwType = MEMORY_BP;

		clsBreakpointManager::BreakpointDelete(tblBPs->item(iUpdateLine,1)->text().toULongLong(0,16),dwType);
		tblBPs->removeRow(iUpdateLine);
	}

	DWORD dwType = 0,
		dwBreakOn = 0;

	if(cbType->currentText().compare("Software BP") == 0)
		dwType = SOFTWARE_BP;
	else if(cbType->currentText().compare("Hardware BP") == 0)
		dwType = HARDWARE_BP;
	else if(cbType->currentText().compare("Memory BP") == 0)
		dwType = MEMORY_BP;

	if(cbBreakOn->currentText().compare("Execute") == 0)
		dwBreakOn = BP_EXEC;
	else if(cbBreakOn->currentText().compare("Read") == 0)
		dwBreakOn = BP_READ;
	else if(cbBreakOn->currentText().compare("Write") == 0)
		dwBreakOn = BP_WRITE;
	else if(cbBreakOn->currentText().compare("Access") == 0)
		dwBreakOn = BP_ACCESS;

	if(lePID->text().toInt() == -1)
		clsBreakpointManager::BreakpointInsert(dwType, dwBreakOn, lePID->text().toInt(), dwOffset, cbSize->currentText().toInt(), BP_KEEP);
	else
		clsBreakpointManager::BreakpointInsert(dwType, dwBreakOn, lePID->text().toInt(0,16), dwOffset, cbSize->currentText().toInt(), BP_KEEP);
}

void qtDLGBreakPointManager::OnSelectedBPChanged(int iRow,int iCol)
{
	lePID->setText(tblBPs->item(iRow,0)->text());
	leOffset->setText(tblBPs->item(iRow,1)->text());

	DWORD	selectedBreakType	= NULL,
			selectedBPSize		= tblBPs->item(iRow,3)->text().toInt();

	if(QString().compare(tblBPs->item(iRow,4)->text(),"Execute") == 0)
		selectedBreakType = BP_EXEC;
	else if(QString().compare(tblBPs->item(iRow,4)->text(),"Read") == 0)
		selectedBreakType = BP_READ;
	else if(QString().compare(tblBPs->item(iRow,4)->text(),"Write") == 0)
		selectedBreakType = BP_WRITE;
	else if(QString().compare(tblBPs->item(iRow,4)->text(),"Access") == 0)
		selectedBreakType = BP_ACCESS;

	if(QString().compare(tblBPs->item(iRow,2)->text(),"Software BP") == 0)
	{
		cbBreakOn->clear();
		cbBreakOn->addItem("Execute");
		cbBreakOn->setEnabled(false);

		cbType->setCurrentIndex(0);

		cbSize->clear();
		cbSize->addItem("1");
		cbSize->addItem("2");
		cbSize->addItem("4");
		cbSize->setEnabled(true);

		switch(selectedBPSize)
		{
		case 1: cbSize->setCurrentIndex(0); break;
		case 2: cbSize->setCurrentIndex(1); break;
		case 4: cbSize->setCurrentIndex(2); break;
		}
	}
	else if(QString().compare(tblBPs->item(iRow,2)->text(),"Hardware BP") == 0)
	{
		cbBreakOn->clear();
		cbBreakOn->addItem("Execute");
		cbBreakOn->addItem("Read");
		cbBreakOn->addItem("Write");
		cbBreakOn->setEnabled(true);

		cbType->setCurrentIndex(1);

		switch(selectedBreakType)
		{
		case BP_EXEC: cbBreakOn->setCurrentIndex(0); break;
		case BP_READ: cbBreakOn->setCurrentIndex(1); break;
		case BP_WRITE: cbBreakOn->setCurrentIndex(2); break;
		}

		cbSize->clear();
		cbSize->addItem("1");
		cbSize->addItem("2");
		cbSize->addItem("4");
		cbSize->setEnabled(true);

		switch(selectedBPSize)
		{
		case 1: cbSize->setCurrentIndex(0); break;
		case 2: cbSize->setCurrentIndex(1); break;
		case 4: cbSize->setCurrentIndex(2); break;
		}
	}
	else if(QString().compare(tblBPs->item(iRow,2)->text(),"Memory BP") == 0)
	{
		cbBreakOn->clear();
		cbBreakOn->addItem("Access");
		cbBreakOn->addItem("Execute");
		cbBreakOn->addItem("Write");
		cbBreakOn->setEnabled(true);

		cbType->setCurrentIndex(2);

		cbSize->clear();
		cbSize->addItem("0");
		cbSize->setEnabled(false);

		switch(selectedBreakType)
		{
		case BP_ACCESS: cbBreakOn->setCurrentIndex(0); break;
		case BP_EXEC: cbBreakOn->setCurrentIndex(1); break;
		case BP_WRITE: cbBreakOn->setCurrentIndex(2); break;
		}	
	}
}

void qtDLGBreakPointManager::OnBPRemove()
{
	DWORD dwType = 0;

	for(int i = 0; i < tblBPs->rowCount(); i++)
	{
		if(tblBPs->item(i,0)->isSelected())
		{
			if(QString().compare(tblBPs->item(i,2)->text(),"Software BP") == 0)
				dwType = SOFTWARE_BP;
			else if(QString().compare(tblBPs->item(i,2)->text(),"Hardware BP") == 0)
				dwType = HARDWARE_BP;
			else if(QString().compare(tblBPs->item(i,2)->text(),"Memory BP") == 0)
				dwType = MEMORY_BP;

			clsBreakpointManager::BreakpointDelete(tblBPs->item(i,1)->text().toULongLong(0,16),dwType);
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

void qtDLGBreakPointManager::OnSendToDisassembler(QTableWidgetItem *pItem)
{
	emit OnDisplayDisassembly(tblBPs->item(pItem->row(),1)->text().toULongLong(0,16));
}

void qtDLGBreakPointManager::OnBPTypeSelectionChanged(const QString &selectedItemText)
{
	if(selectedItemText.compare("Software BP") == 0)
	{
		cbBreakOn->clear();
		cbBreakOn->addItem("Execute");
		cbBreakOn->setEnabled(false);

		cbSize->clear();
		cbSize->addItem("1");
		cbSize->addItem("2");
		cbSize->addItem("4");
		cbSize->setEnabled(true);
	}
	else if(selectedItemText.compare("Hardware BP") == 0)
	{
		cbBreakOn->clear();
		cbBreakOn->addItem("Execute");
		cbBreakOn->addItem("Read");
		cbBreakOn->addItem("Write");
		cbBreakOn->setEnabled(true);

		cbSize->clear();
		cbSize->addItem("1");
		cbSize->addItem("2");
		cbSize->addItem("4");
		cbSize->setEnabled(true);
	}
	else if(selectedItemText.compare("Memory BP") == 0)
	{
		cbBreakOn->clear();
		cbBreakOn->addItem("Access");
		cbBreakOn->addItem("Execute");
		cbBreakOn->addItem("Write");
		cbBreakOn->setEnabled(true);
		
		cbSize->clear();
		cbSize->addItem("0");
		cbSize->setEnabled(false);
	}
}