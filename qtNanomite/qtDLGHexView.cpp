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
	: QWidget(parent, flags),
	m_isFinished(false)
{
	setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose,true);
	this->setLayout(horizontalLayout);
	this->setWindowTitle(QString("[ Nanomite ] - Show Memory - PID - %1 - From: %2 - To: %3").arg(dwPID, 8, 16, QChar('0')).arg(StartOffset, 8, 16, QChar('0')).arg(StartOffset + Size, 8, 16, QChar('0')));
	
	m_maxRows = (tblHexView->verticalHeader()->height() / 11);
	
	tblHexView->horizontalHeader()->resizeSection(0,135);
	tblHexView->horizontalHeader()->resizeSection(1,375);
	tblHexView->horizontalHeader()->resizeSection(2,230);
	tblHexView->horizontalHeader()->setFixedHeight(21);

	qtDLGNanomite *MyMainWindow = qtDLGNanomite::GetInstance();

	tblHexView->setRowCount(0);

	HANDLE hProcess = NULL;
	for(int i = 0;i < MyMainWindow->coreDebugger->PIDs.size();i++)
	{
		if(dwPID == MyMainWindow->coreDebugger->PIDs[i].dwPID)
		{
			hProcess = MyMainWindow->coreDebugger->PIDs[i].hProc;
			break;
		}
	}

	m_pHexDataWorker = new clsHexViewWorker(hProcess, StartOffset, Size);
	connect(m_pHexDataWorker, SIGNAL(finished()), this, SLOT(DisplayData()), Qt::QueuedConnection);
	connect(memoryScroll, SIGNAL(valueChanged(int)), this, SLOT(InsertDataFrom(int)));
	connect(new QShortcut(Qt::Key_Escape, this), SIGNAL(activated()), this, SLOT(close()));
}

qtDLGHexView::~qtDLGHexView()
{
	delete m_pHexDataWorker;
}

void qtDLGHexView::DisplayData()
{
	m_maxRows = (tblHexView->verticalHeader()->height() / 11);

	m_isFinished = true;

	memoryScroll->setValue(0);
	memoryScroll->setMaximum(m_pHexDataWorker->dataList.count() - m_maxRows - 1);

	InsertDataFrom(0);
}

void qtDLGHexView::InsertDataFrom(int position)
{
	if(position < 0 || !m_isFinished) return; 

	if((tblHexView->rowCount() - 1) != m_maxRows)
	{
		tblHexView->setRowCount(0);
		
		int count = 0;
		while(tblHexView->rowCount() <= m_maxRows)
		{
			count++;
			tblHexView->insertRow(0);
		}
	}	
	
	HexData currentHexData;
	int numberOfLines = 0;
	while(numberOfLines <= m_maxRows)
	{
		if(position >= m_pHexDataWorker->dataList.count())
			break;
		else
		{			
			currentHexData = m_pHexDataWorker->dataList.at(position);

			tblHexView->setItem(numberOfLines, 0,
				new QTableWidgetItem(QString("%1").arg(currentHexData.hexOffset,16,16,QChar('0'))));

			tblHexView->setItem(numberOfLines, 1,
				new QTableWidgetItem(currentHexData.hexString));

			tblHexView->setItem(numberOfLines, 2,
				new QTableWidgetItem(currentHexData.asciiData));

			position++;numberOfLines++;
		}
	}	
}

void qtDLGHexView::resizeEvent(QResizeEvent *event)
{
	m_maxRows = (tblHexView->verticalHeader()->height() / 11);
	memoryScroll->setMaximum(m_pHexDataWorker->dataList.count() - m_maxRows - 1);

	InsertDataFrom(memoryScroll->value());
}

void qtDLGHexView::wheelEvent(QWheelEvent *event)
{
	QWheelEvent *pWheel = (QWheelEvent*)event;

	if(pWheel->delta() > 0)
	{
		memoryScroll->setValue(memoryScroll->value() - 1);
	}
	else
	{
		memoryScroll->setValue(memoryScroll->value() + 1);
	}

	InsertDataFrom(memoryScroll->value());
}