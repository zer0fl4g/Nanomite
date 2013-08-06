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
#include "qtDLGStringView.h"

#include "clsMemManager.h"

#include <QMenu>
#include <QClipboard>

using namespace std;

qtDLGStringView::qtDLGStringView(QWidget *parent, Qt::WFlags flags, qint32 processID)
	: QWidget(parent, flags),
	m_processID(processID),
	m_pStringProcessor(NULL)
{
	setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose,true);
	this->setLayout(horizontalLayout);
	tblStringView->setRowCount(0);
	
	// Init List
	tblStringView->horizontalHeader()->resizeSection(0,75);
	tblStringView->horizontalHeader()->resizeSection(1,135);
	tblStringView->horizontalHeader()->setFixedHeight(21);

	// Display
	m_pMainWindow = qtDLGNanomite::GetInstance();

	m_forEntry = 0;
	m_forEnd = m_pMainWindow->coreDebugger->PIDs.size();

	for(size_t i = 0; i < m_pMainWindow->coreDebugger->PIDs.size(); i++)
	{
		if(m_pMainWindow->coreDebugger->PIDs[i].dwPID == m_processID)
			m_forEntry = i; m_forEnd = i + 1;
	}
	
	connect(stringScroll,SIGNAL(valueChanged(int)),this,SLOT(InsertDataFrom(int)));
	connect(tblStringView,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnCustomContextMenuRequested(QPoint)));
	connect(new QShortcut(QKeySequence("F5"),this),SIGNAL(activated()),this,SLOT(DataProcessing()));
	connect(new QShortcut(Qt::Key_Escape,this),SIGNAL(activated()),this,SLOT(close()));

	DataProcessing();
}

qtDLGStringView::~qtDLGStringView()
{
	delete m_pStringProcessor;
}

void qtDLGStringView::DataProcessing()
{
	QMap<int,PTCHAR> dataForProcessing;
	for(size_t i = m_forEntry; i < m_forEnd; i++)
	{
		dataForProcessing.insert(m_pMainWindow->coreDebugger->PIDs[m_forEntry].dwPID,
			m_pMainWindow->coreDebugger->PIDs[m_forEntry].sFileName);
	}

	if(m_pStringProcessor != NULL)
		delete m_pStringProcessor;
	m_pStringProcessor = new clsStringViewWorker(dataForProcessing);
	connect(m_pStringProcessor,SIGNAL(finished()),this,SLOT(DisplayStrings()),Qt::QueuedConnection);
}

void qtDLGStringView::DisplayStrings()
{
	stringScroll->setValue(0);
	stringScroll->setMaximum(m_pStringProcessor->stringList.count() - (tblStringView->verticalHeader()->height() / 15) + 1);

	InsertDataFrom(0);
}

void qtDLGStringView::InsertDataFrom(int position)
{
	tblStringView->setRowCount(0);
	int numberOfLines = 0,
		possibleRowCount = (tblStringView->verticalHeader()->height() / 15) - 1,
		count = 0;
	QMap<DWORD64,StringData>::const_iterator i = m_pStringProcessor->stringList.constBegin();

	if(position != 0)
	{
		while(count < stringScroll->value()) // && count <= (stringScroll->value() - ((tblStringView->verticalHeader()->height() + 4) / 15)))
		{
			count++;++i;
		}
	}
	else
	{
		i = m_pStringProcessor->stringList.begin();
		stringScroll->setValue(0);
	}

	while(numberOfLines <= possibleRowCount)
	{
		if(i == m_pStringProcessor->stringList.constEnd())
			break;
		else
		{
			tblStringView->insertRow(tblStringView->rowCount());

			// PID
			tblStringView->setItem(tblStringView->rowCount() - 1,0,
				new QTableWidgetItem(QString().sprintf("%08X",i->PID)));

			// Offset
			if(i->StringOffset > 0)
				tblStringView->setItem(tblStringView->rowCount() - 1,1,
				new QTableWidgetItem(QString("%1").arg(i->StringOffset,8,16,QChar('0'))));
			else 
				tblStringView->setItem(tblStringView->rowCount() - 1,1,
				new QTableWidgetItem(""));

			// String
			tblStringView->setItem(tblStringView->rowCount() - 1,2,
				new QTableWidgetItem(i->DataString));

			++i;numberOfLines++;
		}
	}
}

void qtDLGStringView::MenuCallback(QAction* pAction)
{
	if(QString().compare(pAction->text(),"Line") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(QString("%1:%2:%3")
			.arg(tblStringView->item(m_selectedRow,0)->text())
			.arg(tblStringView->item(m_selectedRow,1)->text())
			.arg(tblStringView->item(m_selectedRow,2)->text()));
	}
	else if(QString().compare(pAction->text(),"Offset") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblStringView->item(m_selectedRow,1)->text());
	}
	else if(QString().compare(pAction->text(),"String") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblStringView->item(m_selectedRow,2)->text());
	}
}

void qtDLGStringView::OnCustomContextMenuRequested(QPoint qPoint)
{
	QMenu menu;

	m_selectedRow = tblStringView->indexAt(qPoint).row();
	if(m_selectedRow < 0) return;

	QMenu *submenu = menu.addMenu("Copy to Clipboard");
	submenu->addAction(new QAction("Line",this));
	submenu->addAction(new QAction("Offset",this));
	submenu->addAction(new QAction("String",this));

	menu.addMenu(submenu);
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));

	menu.exec(QCursor::pos());
}

void qtDLGStringView::resizeEvent(QResizeEvent *event)
{
	InsertDataFrom(stringScroll->value());
}

void qtDLGStringView::wheelEvent(QWheelEvent *event)
{
	QWheelEvent *pWheel = (QWheelEvent*)event;

	if(pWheel->delta() > 0)
	{
		stringScroll->setValue(stringScroll->value() - 1);
		InsertDataFrom(-1);
	}
	else
	{
		stringScroll->setValue(stringScroll->value() + 1);
		InsertDataFrom(1);
	}
}