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
	isDetailView(false)
{
	this->setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose,true);
	this->setLayout(horizontalLayout);
	tblFunctions->setRowCount(0);

	connect(tblFunctions,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnCustomContextMenu(QPoint)));
	connect(tblFunctions,SIGNAL(itemDoubleClicked(QTableWidgetItem *)),this,SLOT(OnSendToDisassembler(QTableWidgetItem *)));

	// Init List
	tblFunctions->horizontalHeader()->resizeSection(0,75);
	tblFunctions->horizontalHeader()->resizeSection(1,200);
	tblFunctions->horizontalHeader()->resizeSection(2,135);
	tblFunctions->horizontalHeader()->setFixedHeight(21);

	qtDLGNanomite *myMainWindow = qtDLGNanomite::GetInstance();

	size_t	iForEntry = 0,
			iForEnd = myMainWindow->coreDebugger->PIDs.size();

	for(size_t i = 0; i < myMainWindow->coreDebugger->PIDs.size(); i++)
	{
		if(myMainWindow->coreDebugger->PIDs[i].dwPID == m_processID)
			iForEntry = i; iForEnd = i + 1;
	}

	QList<FunctionProcessingData> dataForProcessing;
	for(size_t i = iForEntry; i < iForEnd;i++)
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
	functionScroll->setValue(0);
	functionScroll->setMaximum(m_pFunctionWorker->functionList.count() - (tblFunctions->verticalHeader()->height() / 11) + 1);

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
	tblFunctions->setRowCount(0);
	int numberOfLines = 0,
		possibleRowCount = (tblFunctions->verticalHeader()->height() / 11) - 1,
		count = 0;
	QList<FunctionData>::ConstIterator i = m_pFunctionWorker->functionList.constBegin();

	if(position != 0)
	{
		while(count < functionScroll->value())
		{
			count++;++i;
		}
	}
	else
	{
		i = m_pFunctionWorker->functionList.begin();
		functionScroll->setValue(0);
	}

	while(numberOfLines <= possibleRowCount)
	{
		if(i == m_pFunctionWorker->functionList.constEnd())
			break;
		else
		{
			tblFunctions->insertRow(tblFunctions->rowCount());
			// PID
			tblFunctions->setItem(tblFunctions->rowCount() - 1,0,
				new QTableWidgetItem(QString("%1").arg(i->processID,8,16,QChar('0'))));

			// Func Name
			tblFunctions->setItem(tblFunctions->rowCount() - 1,1,
				new QTableWidgetItem(i->functionSymbol));

			// Func Offset
			tblFunctions->setItem(tblFunctions->rowCount() - 1,2,
				new QTableWidgetItem(QString("%1").arg(i->FunctionOffset,16,16,QChar('0'))));

			// Func Size
			tblFunctions->setItem(tblFunctions->rowCount() - 1,3,
				new QTableWidgetItem(QString("%1").arg(i->FunctionSize,6,10,QChar('0'))));

			++i;numberOfLines++;
		}
	}
}

void qtDLGFunctions::resizeEvent(QResizeEvent *event)
{
	InsertDataFrom(functionScroll->value());
}

void qtDLGFunctions::wheelEvent(QWheelEvent *event)
{
	QWheelEvent *pWheel = (QWheelEvent*)event;

	if(pWheel->delta() > 0)
	{
		functionScroll->setValue(functionScroll->value() - 1);
		InsertDataFrom(-1);
	}
	else
	{
		functionScroll->setValue(functionScroll->value() + 1);
		InsertDataFrom(1);
	}
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