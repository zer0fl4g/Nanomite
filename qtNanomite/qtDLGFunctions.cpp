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
#include "qtDLGFunctions.h"
#include "qtDLGNanomite.h"

#include "clsMemManager.h"

using namespace std;

qtDLGFunctions::qtDLGFunctions(qint32 processID, QWidget *parent, Qt::WFlags flags)
	: QWidget(parent, flags),
	m_processID(processID),
	isDetailView(false),
	m_isFinished(false)
{
	this->setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose,true);
	this->setLayout(horizontalLayout);
	tblFunctions->setRowCount(0);
	m_maxRows = (tblFunctions->verticalHeader()->height() / 11) - 1;

	connect(tblFunctions,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnCustomContextMenu(QPoint)));
	connect(tblFunctions,SIGNAL(itemDoubleClicked(QTableWidgetItem *)),this,SLOT(OnSendToDisassembler(QTableWidgetItem *)));

	// Init List
	tblFunctions->horizontalHeader()->resizeSection(0,75);
	tblFunctions->horizontalHeader()->resizeSection(1,200);
	tblFunctions->horizontalHeader()->resizeSection(2,135);
	tblFunctions->horizontalHeader()->setFixedHeight(21);

	qtDLGNanomite *myMainWindow = qtDLGNanomite::GetInstance();

	int	iForEntry = 0,
		iForEnd = myMainWindow->coreDebugger->PIDs.size();

	for(int i = 0; i < myMainWindow->coreDebugger->PIDs.size(); i++)
	{
		if(myMainWindow->coreDebugger->PIDs[i].dwPID == m_processID)
		{
			iForEntry = i;
			iForEnd = i + 1;
			break;
		}
	}

	QList<FunctionProcessingData> dataForProcessing;
	for(int i = iForEntry; i < iForEnd;i++)
	{
		FunctionProcessingData newData;
		newData.currentModule = (PTCHAR)myMainWindow->coreDebugger->PIDs[i].sFileName;
		newData.processHandle = myMainWindow->coreDebugger->PIDs[i].hProc;
		dataForProcessing.append(newData);
	}
	
	m_pFunctionWorker = new clsFunctionsViewWorker(dataForProcessing);
	connect(m_pFunctionWorker,SIGNAL(finished()),this,SLOT(DisplayFunctionLists()),Qt::QueuedConnection);
	connect(functionScroll,SIGNAL(valueChanged(int)),this,SLOT(InsertDataFrom(int)));
	connect(new QShortcut(Qt::Key_Escape,this),SIGNAL(activated()),this,SLOT(close()));
	connect(new QShortcut(QKeySequence::InsertParagraphSeparator,this),SIGNAL(activated()),this,SLOT(OnReturnPressed()));
}

qtDLGFunctions::qtDLGFunctions(qint32 processID, QString modulePath, QWidget *parent, Qt::WFlags flags)
	: QWidget(parent, flags),
	m_processID(processID),
	isDetailView(true)
{
	this->setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose,true);
	this->setLayout(horizontalLayout);
	tblFunctions->setRowCount(0);
	m_maxRows = (tblFunctions->verticalHeader()->height() / 11) - 1;

	connect(tblFunctions,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnCustomContextMenu(QPoint)));
	connect(tblFunctions,SIGNAL(itemDoubleClicked(QTableWidgetItem *)),this,SLOT(OnSendToDisassembler(QTableWidgetItem *)));

	// Init List
	tblFunctions->horizontalHeader()->resizeSection(0,75);
	tblFunctions->horizontalHeader()->resizeSection(1,200);
	tblFunctions->horizontalHeader()->resizeSection(2,135);
	tblFunctions->horizontalHeader()->setFixedHeight(21);
	
	m_modulePath = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));
	ZeroMemory(m_modulePath,MAX_PATH * sizeof(TCHAR));
	modulePath.toWCharArray(m_modulePath);

	QList<FunctionProcessingData> dataForProcessing;
	FunctionProcessingData newData;
	newData.currentModule = m_modulePath;
	newData.processHandle = clsDebugger::GetProcessHandleByPID(processID);
	dataForProcessing.append(newData);

	
	m_pFunctionWorker = new clsFunctionsViewWorker(dataForProcessing);
	connect(m_pFunctionWorker,SIGNAL(finished()),this,SLOT(DisplayFunctionLists()),Qt::QueuedConnection);
	connect(functionScroll,SIGNAL(valueChanged(int)),this,SLOT(InsertDataFrom(int)));
	connect(new QShortcut(Qt::Key_Escape,this),SIGNAL(activated()),this,SLOT(close()));
	connect(new QShortcut(QKeySequence::InsertParagraphSeparator,this),SIGNAL(activated()),this,SLOT(OnReturnPressed()));
}

qtDLGFunctions::~qtDLGFunctions()
{
	delete m_pFunctionWorker;

	if(isDetailView)
		clsMemManager::CFree(m_modulePath);
}

void qtDLGFunctions::DisplayFunctionLists()
{
	if(m_pFunctionWorker->functionList.count() <= 0) return;

	m_isFinished = true;

	functionScroll->setValue(0);
	functionScroll->setMaximum(m_pFunctionWorker->functionList.count() - (tblFunctions->verticalHeader()->height() / 11));
	
	m_maxRows = (tblFunctions->verticalHeader()->height() / 11) - 1;

	InsertDataFrom(0);
}

void qtDLGFunctions::OnCustomContextMenu(QPoint qPoint)
{
	QMenu menu;

	m_selectedRow = tblFunctions->indexAt(qPoint).row();
	if(m_selectedRow < 0) return;

	menu.addAction(new QAction("Send to Disassembler",this));
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));

	menu.exec(QCursor::pos());
}

void qtDLGFunctions::MenuCallback(QAction* pAction)
{
	if(QString().compare(pAction->text(),"Send to Disassembler") == 0)
	{
		emit ShowInDisAs(tblFunctions->item(m_selectedRow,2)->text().toULongLong(0,16));
	}
}

void qtDLGFunctions::InsertDataFrom(int position)
{
	if(position < 0 || !m_isFinished) return;

	if((tblFunctions->rowCount() - 1) != m_maxRows)
	{
		int count = 0;
		tblFunctions->setRowCount(0);
		while(count <= m_maxRows)
		{
			tblFunctions->insertRow(0);
			count++;
		}
	}

	int numberOfLines = 0;
	FunctionData currentFunctionData;
	while(numberOfLines <= m_maxRows)
	{
		if(position >= m_pFunctionWorker->functionList.count())
			break;
		else
		{
			currentFunctionData = m_pFunctionWorker->functionList.at(position);

			// PID
			tblFunctions->setItem(numberOfLines,0,
				new QTableWidgetItem(QString("%1").arg(currentFunctionData.processID,8,16,QChar('0'))));

			// Func Name
			tblFunctions->setItem(numberOfLines,1,
				new QTableWidgetItem(currentFunctionData.functionSymbol));

			// Func Offset
			tblFunctions->setItem(numberOfLines,2,
				new QTableWidgetItem(QString("%1").arg(currentFunctionData.FunctionOffset,16,16,QChar('0'))));

			// Func Size
			tblFunctions->setItem(numberOfLines,3,
				new QTableWidgetItem(QString("%1").arg(currentFunctionData.FunctionSize,6,10,QChar('0'))));

			position++;numberOfLines++;
		}
	}
}

void qtDLGFunctions::resizeEvent(QResizeEvent *event)
{
	m_maxRows = (tblFunctions->verticalHeader()->height() / 11) - 1;
	functionScroll->setMaximum(m_pFunctionWorker->functionList.count() - (tblFunctions->verticalHeader()->height() / 11));

	InsertDataFrom(functionScroll->value());
}

void qtDLGFunctions::wheelEvent(QWheelEvent *event)
{
	QWheelEvent *pWheel = (QWheelEvent*)event;

	if(pWheel->delta() > 0)
	{
		functionScroll->setValue(functionScroll->value() - 1);		
	}
	else
	{
		functionScroll->setValue(functionScroll->value() + 1);
	}

	InsertDataFrom(functionScroll->value());
}

void qtDLGFunctions::OnSendToDisassembler(QTableWidgetItem *pSelectedRow)
{
	emit ShowInDisAs(tblFunctions->item(pSelectedRow->row(),2)->text().toULongLong(0,16));
}

void qtDLGFunctions::OnReturnPressed()
{
	if(tblFunctions->selectedItems().count() <= 0) return;

	OnSendToDisassembler(tblFunctions->selectedItems()[0]);
}