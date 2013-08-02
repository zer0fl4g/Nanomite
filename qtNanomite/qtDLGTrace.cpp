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

	tblTraceLog->horizontalHeader()->resizeSection(0,80); //PID
	tblTraceLog->horizontalHeader()->resizeSection(1,80); //TID
	tblTraceLog->horizontalHeader()->resizeSection(2,135); //OFFSET
	tblTraceLog->horizontalHeader()->resizeSection(3,300); //Symbol.
	//tblTraceLog->horizontalHeader()->resizeSection(4,300); //REG
	tblTraceLog->horizontalHeader()->setFixedHeight(21);

	connect(m_statusBarTimer,SIGNAL(timeout()),this,SLOT(OnUpdateStatusBar()));
	connect(tblTraceLog,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnCustomContextMenu(QPoint)));
	connect(scrollTrace,SIGNAL(valueChanged(int)),this,SLOT(OnShow(int)));
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
	OnShow(0);
}

void qtDLGTrace::OnShow(int delta)
{
	tblTraceLog->setRowCount(0);
	int iLines = NULL,
		count = NULL,
		iPossibleRowCount = (tblTraceLog->verticalHeader()->height() / 12) - 1;
	QList<TraceInfoRow>::iterator i = m_traceData.begin();

	scrollTrace->setMaximum(m_traceData.count() + 2 - iPossibleRowCount);

	if(delta != 0 && scrollTrace->value() >= 0)
	{
		while(count < scrollTrace->value())// && count <= (scrollTrace->value() - ((tblTraceLog->verticalHeader()->height() + 4) / 12)))
		{
			count++;++i;
		}
	}
	else
	{
		i = m_traceData.begin();
		scrollTrace->setValue(0);
	}

	while(iLines <= iPossibleRowCount)
	{
		if(i == m_traceData.end())
			return;

		tblTraceLog->insertRow(tblTraceLog->rowCount());
		
		tblTraceLog->setItem(tblTraceLog->rowCount() - 1,0,
			new QTableWidgetItem(QString("%1").arg(i->PID,8,16,QChar('0'))));
		
		tblTraceLog->setItem(tblTraceLog->rowCount() - 1,1,
			new QTableWidgetItem(QString("%1").arg(i->TID,8,16,QChar('0'))));
	
		tblTraceLog->setItem(tblTraceLog->rowCount() - 1,2,
			new QTableWidgetItem(QString("%1").arg(i->dwOffset,16,16,QChar('0'))));

		if(i->debugSymbols.length() <= 0)
		{
			std::wstring FuncName,ModName;
			clsHelperClass::LoadSymbolForAddr(FuncName,ModName,i->dwOffset,qtDLGNanomite::GetInstance()->coreDebugger->GetProcessHandleByPID(i->PID));

			QString funcName = QString().fromStdWString(FuncName);
			QString modName = QString().fromStdWString(ModName);

			if(modName.length() > 0 && funcName.length() > 0)
				i->debugSymbols.append(modName).append(".").append(funcName);
		}

		tblTraceLog->setItem(tblTraceLog->rowCount() - 1,3,
			new QTableWidgetItem(i->debugSymbols));

		//tblTraceLog->setItem(tblTraceLog->rowCount() - 1,4,
		//	new QTableWidgetItem(i.value().cpuReg));

		++i;iLines++;
	}
}

void qtDLGTrace::wheelEvent(QWheelEvent *event)
{
	QWheelEvent *pWheel = (QWheelEvent*)event;

	if(pWheel->delta() > 0)
	{
		scrollTrace->setValue(scrollTrace->value() - 1);
		OnShow(-1);
	}
	else
	{
		scrollTrace->setValue(scrollTrace->value() + 1);
		OnShow(1);
	}
}

void qtDLGTrace::resizeEvent(QResizeEvent *event)
{
	OnShow(0);
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