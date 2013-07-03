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
#include "qtDLGHexView.h"
#include "qtDLGNanomite.h"

#include "clsMemManager.h"

#include <Windows.h>

qtDLGHexView::qtDLGHexView(QWidget *parent, Qt::WFlags flags,unsigned long dwPID, unsigned long long StartOffset,unsigned long long Size)
	: QWidget(parent, flags)
{
	setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose,true);
	this->setLayout(horizontalLayout);
	this->setWindowTitle(QString("[ Nanomite ] - Show Memory - PID - %1 - From: %2 - To: %3").arg(dwPID,8,16,QChar('0')).arg(StartOffset,8,16,QChar('0')).arg(StartOffset + Size,8,16,QChar('0')));

	tblHexView->horizontalHeader()->resizeSection(0,75);
	tblHexView->horizontalHeader()->resizeSection(1,135);
	tblHexView->horizontalHeader()->resizeSection(2,375);
	tblHexView->horizontalHeader()->resizeSection(3,230);
	tblHexView->horizontalHeader()->setFixedHeight(21);

	qtDLGNanomite *MyMainWindow = qtDLGNanomite::GetInstance();

	tblHexView->setRowCount(0);

	HANDLE hProcess = NULL;
	for(size_t i = 0;i < MyMainWindow->coreDebugger->PIDs.size();i++)
	{
		if(dwPID == MyMainWindow->coreDebugger->PIDs[i].dwPID)
			hProcess = MyMainWindow->coreDebugger->PIDs[i].hProc;
	}

	m_pHexDataWorker = new clsHexViewWorker(dwPID,hProcess,StartOffset,Size);
	connect(m_pHexDataWorker,SIGNAL(finished()),this,SLOT(DisplayData()),Qt::QueuedConnection);
	connect(memoryScroll,SIGNAL(valueChanged(int)),this,SLOT(InsertDataFrom(int)));
}

qtDLGHexView::~qtDLGHexView()
{
	delete m_pHexDataWorker;
}

void qtDLGHexView::DisplayData()
{
	memoryScroll->setValue(0);
	memoryScroll->setMaximum(m_pHexDataWorker->dataList.count() - (tblHexView->verticalHeader()->height() / 11) + 1);

	InsertDataFrom(0);
}

void qtDLGHexView::InsertDataFrom(int position)
{
	tblHexView->setRowCount(0);
	int numberOfLines = 0,
		possibleRowCount = (tblHexView->verticalHeader()->height() / 11) - 1,
		count = 0;
	QMap<DWORD64,HexData>::const_iterator i = m_pHexDataWorker->dataList.constBegin();

	if(position != 0)
	{
		while(count < memoryScroll->value()) // && count <= (memoryScroll->value() - ((tblHexView->verticalHeader()->height() + 4) / 11)))
		{
			count++;++i;
		}
	}
	else
	{
		i = m_pHexDataWorker->dataList.begin();
		memoryScroll->setValue(0);
	}

	while(numberOfLines <= possibleRowCount)
	{
		if(i == m_pHexDataWorker->dataList.constEnd())
			break;
		else
		{
			tblHexView->insertRow(tblHexView->rowCount());

			tblHexView->setItem(tblHexView->rowCount() - 1,0,
				new QTableWidgetItem(QString("%1").arg(i->PID,8,16,QChar('0'))));

			tblHexView->setItem(tblHexView->rowCount() - 1,1,
				new QTableWidgetItem(QString("%1").arg(i->hexOffset,16,16,QChar('0'))));

			tblHexView->setItem(tblHexView->rowCount() - 1,2,
				new QTableWidgetItem(i->hexString));

			tblHexView->setItem(tblHexView->rowCount() - 1,3,
				new QTableWidgetItem(i->asciiData));

			++i;numberOfLines++;
		}
	}	
}

void qtDLGHexView::resizeEvent(QResizeEvent *event)
{
	InsertDataFrom(memoryScroll->value());
}

void qtDLGHexView::wheelEvent(QWheelEvent *event)
{
	QWheelEvent *pWheel = (QWheelEvent*)event;

	if(pWheel->delta() > 0)
	{
		memoryScroll->setValue(memoryScroll->value() - 1);
		InsertDataFrom(-1);
	}
	else
	{
		memoryScroll->setValue(memoryScroll->value() + 1);
		InsertDataFrom(1);
	}
}