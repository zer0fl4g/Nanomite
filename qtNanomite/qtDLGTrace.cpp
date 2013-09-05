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
#include "qtDLGTrace.h"

#include "clsHelperClass.h"
#include "clsMemManager.h"

#include <QWheelEvent>

using namespace std;

qtDLGTrace *qtDLGTrace::pThis = NULL;

qtDLGTrace::qtDLGTrace(QWidget *parent, Qt::WFlags flags)
	: QWidget(parent, flags),
	m_stepsDoneInSecond(0),
	m_prevStepsDone(0)
{
	setupUi(this);
	this->setLayout(horizontalLayout);
	this->setStyleSheet(clsHelperClass::LoadStyleSheet());
	pThis = this;
	
	m_statusBarTimer = new QTimer(this);
	m_statusBarTimer->setInterval(1000);
	m_statusBarTimer->stop();

	m_maxRows = (tblTraceLog->verticalHeader()->height() / 12) - 1;

	tblTraceLog->horizontalHeader()->resizeSection(0,80); //PID
	tblTraceLog->horizontalHeader()->resizeSection(1,80); //TID
	tblTraceLog->horizontalHeader()->resizeSection(2,135); //OFFSET
	tblTraceLog->horizontalHeader()->resizeSection(3,300); //Symbol.
	tblTraceLog->horizontalHeader()->setFixedHeight(21);

	connect(m_statusBarTimer,SIGNAL(timeout()),this,SLOT(OnUpdateStatusBar()));
	connect(tblTraceLog,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnCustomContextMenu(QPoint)));
	connect(tblTraceLog,SIGNAL(itemDoubleClicked(QTableWidgetItem *)),this,SLOT(OnDoubleClickFunction(QTableWidgetItem *)));
	connect(scrollTrace,SIGNAL(valueChanged(int)),this,SLOT(OnShow(int)));
	connect(new QShortcut(Qt::Key_Escape,this),SIGNAL(activated()),this,SLOT(close()));
}

qtDLGTrace::~qtDLGTrace()
{
	pThis = NULL;
	delete m_statusBarTimer;
}

void qtDLGTrace::addTraceData(DWORD64 dwOffset,DWORD PID,DWORD TID)
{
	if(pThis == NULL) return;

	TraceInfoRow newTraceRow;
	newTraceRow.PID = PID;
	newTraceRow.TID = TID;
	newTraceRow.debugSymbols = QString("");
	//newTraceRow.cpuReg = QString("");
	newTraceRow.dwOffset = dwOffset;

	pThis->m_traceData.append(newTraceRow);
}

void qtDLGTrace::clearTraceData()
{
	if(pThis == NULL) return;

	pThis->m_traceData.clear();
	pThis->scrollTrace->setValue(0);
	pThis->scrollTrace->setMaximum(0);
}

void qtDLGTrace::showEvent(QShowEvent * event)
{
	if(m_traceData.count() <= 0) return;

	m_maxRows = (tblTraceLog->verticalHeader()->height() / 12) - 1;

	scrollTrace->setValue(0);

	OnShow(0);
}

void qtDLGTrace::OnShow(int delta)
{
	if(delta < 0) return;

	if((tblTraceLog->rowCount() - 1) != m_maxRows)
	{
		tblTraceLog->setRowCount(0);

		int count = 0;
		while(count <= m_maxRows)
		{
			tblTraceLog->insertRow(0);
			count++;
		}
	}
	scrollTrace->setMaximum(m_traceData.count() - m_maxRows - 1);

	int printLine = NULL;
	std::wstring FuncName,ModName;
	TraceInfoRow currentTraceData;
	while(printLine <= m_maxRows)
	{
		if(delta >= m_traceData.count())
			return;
		else
			currentTraceData = m_traceData.at(delta);

		tblTraceLog->setItem(printLine,0,
			new QTableWidgetItem(QString("%1").arg(currentTraceData.PID,8,16,QChar('0'))));
		
		tblTraceLog->setItem(printLine,1,
			new QTableWidgetItem(QString("%1").arg(currentTraceData.TID,8,16,QChar('0'))));
	
		tblTraceLog->setItem(printLine,2,
			new QTableWidgetItem(QString("%1").arg(currentTraceData.dwOffset,16,16,QChar('0'))));

		if(currentTraceData.debugSymbols.length() <= 0)
		{
			clsHelperClass::LoadSymbolForAddr(FuncName,ModName,currentTraceData.dwOffset,qtDLGNanomite::GetInstance()->coreDebugger->GetProcessHandleByPID(currentTraceData.PID));
			if(ModName.length() > 0 && FuncName.length() > 0)
				currentTraceData.debugSymbols.append(QString::fromStdWString(ModName)).append(".").append(QString::fromStdWString(FuncName));
		}

		tblTraceLog->setItem(printLine,3,
			new QTableWidgetItem(currentTraceData.debugSymbols));

		delta++;printLine++;
	}
}

void qtDLGTrace::wheelEvent(QWheelEvent *event)
{
	QWheelEvent *pWheel = (QWheelEvent*)event;

	if(pWheel->delta() > 0)
	{
		scrollTrace->setValue(scrollTrace->value() - 1);
	}
	else
	{
		scrollTrace->setValue(scrollTrace->value() + 1);
	}

	OnShow(scrollTrace->value());
}

void qtDLGTrace::resizeEvent(QResizeEvent *event)
{
	m_maxRows = (tblTraceLog->verticalHeader()->height() / 12) - 1;
	
	OnShow(scrollTrace->value());
}

void qtDLGTrace::OnCustomContextMenu(QPoint qPoint)
{
	QMenu menu;

	m_iSelectedRow = tblTraceLog->indexAt(qPoint).row();
	if(m_iSelectedRow < 0) return;

	menu.addAction(new QAction("Send to Disassembler",this));
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));

	menu.exec(QCursor::pos());
}

void qtDLGTrace::MenuCallback(QAction* pAction)
{
	if(QString().compare(pAction->text(),"Send to Disassembler") == 0)
		emit OnDisplayDisassembly(tblTraceLog->item(m_iSelectedRow,2)->text().toULongLong(0,16));
}

void qtDLGTrace::OnUpdateStatusBar()
{
	quint64 temp = m_traceData.size();
	m_stepsDoneInSecond =  (temp - m_prevStepsDone);
	m_prevStepsDone = temp;

	emit OnUpdateStatusBar(4,m_stepsDoneInSecond);
}

void qtDLGTrace::enableStatusBarTimer()
{
	pThis->m_statusBarTimer->start();
}

void qtDLGTrace::disableStatusBarTimer()
{
	pThis->m_statusBarTimer->stop();
	pThis->m_prevStepsDone = 0;
}

void qtDLGTrace::OnDoubleClickFunction(QTableWidgetItem *pItem)
{
	emit OnDisplayDisassembly(tblTraceLog->item(pItem->row(),2)->text().toULongLong(0,16));
}