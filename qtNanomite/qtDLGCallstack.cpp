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
#include "qtDLGCallstack.h"
#include "qtDLGNanomite.h"

#include "clsMemManager.h"
#include "clsAPIImport.h"

#include <QClipboard>
#include <QMenu>

qtDLGCallstack::qtDLGCallstack(QWidget *parent)
	: QDockWidget(parent)
{
	setupUi(this);

	// List CallStack
	tblCallstack->horizontalHeader()->resizeSection(0,135);
	tblCallstack->horizontalHeader()->resizeSection(1,135);
	tblCallstack->horizontalHeader()->resizeSection(2,300);
	tblCallstack->horizontalHeader()->resizeSection(3,135);
	tblCallstack->horizontalHeader()->resizeSection(4,300);
	tblCallstack->horizontalHeader()->resizeSection(5,75);
	tblCallstack->horizontalHeader()->resizeSection(6,300);
	tblCallstack->horizontalHeader()->setFixedHeight(21);

	p_ctWorker = new clsCallstackWorker();
	p_ctWorker->setAutoDelete(false);

	connect(p_ctWorker, SIGNAL(OnCallstackFinished(QList<callstackDisplay>)), this, SLOT(OnCallstack(QList<callstackDisplay>)), Qt::QueuedConnection);
	connect(tblCallstack,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnContextMenu(QPoint)));
	connect(tblCallstack,SIGNAL(itemDoubleClicked(QTableWidgetItem *)),this,SLOT(OnDisplaySource(QTableWidgetItem *)));
}

qtDLGCallstack::~qtDLGCallstack()
{
	delete p_ctWorker;
}

void qtDLGCallstack::OnContextMenu(QPoint qPoint)
{
	QMenu menu;

	m_selectedRow = tblCallstack->indexAt(qPoint).row();
	if(m_selectedRow < 0) return;

	menu.addAction(new QAction("Send to Disassembler",this));
	menu.addSeparator();
	QMenu *submenu = menu.addMenu("Copy to Clipboard");
	submenu->addAction(new QAction("Line",this));
	submenu->addAction(new QAction("Stack Offset",this));
	submenu->addAction(new QAction("current Function Offset",this));
	submenu->addAction(new QAction("current Module.Function",this));
	submenu->addAction(new QAction("return Function Offset",this));
	submenu->addAction(new QAction("return Module.Function",this));
	submenu->addAction(new QAction("Source Line",this));
	submenu->addAction(new QAction("Source File",this));

	menu.addMenu(submenu);
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));

	menu.exec(QCursor::pos());
}

void qtDLGCallstack::MenuCallback(QAction* pAction)
{
	qtDLGNanomite *pMainWindow = qtDLGNanomite::GetInstance();

	if(!pMainWindow->coreDebugger->GetDebuggingState()) return;

	if(QString().compare(pAction->text(),"Send to Disassembler") == 0)
	{
		if(!pMainWindow->coreDisAs->InsertNewDisassembly(pMainWindow->coreDebugger->GetCurrentProcessHandle(),tblCallstack->item(m_selectedRow,1)->text().toULongLong(0,16)))
				emit OnDisplayDisassembly(tblCallstack->item(m_selectedRow,1)->text().toULongLong(0,16));	
	}	
	else if(QString().compare(pAction->text(),"Line") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(QString("%1:%2:%3:%4:%5:%6:%7")
			.arg(tblCallstack->item(m_selectedRow,0)->text())
			.arg(tblCallstack->item(m_selectedRow,1)->text())
			.arg(tblCallstack->item(m_selectedRow,2)->text())
			.arg(tblCallstack->item(m_selectedRow,3)->text())
			.arg(tblCallstack->item(m_selectedRow,4)->text())
			.arg(tblCallstack->item(m_selectedRow,5)->text())
			.arg(tblCallstack->item(m_selectedRow,6)->text()));
	}
	else if(QString().compare(pAction->text(),"Stack Offset") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblCallstack->item(m_selectedRow,0)->text());
	}
	else if(QString().compare(pAction->text(),"current Function Offset") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblCallstack->item(m_selectedRow,1)->text());
	}
	else if(QString().compare(pAction->text(),"current Module.Function") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblCallstack->item(m_selectedRow,2)->text());
	}
	else if(QString().compare(pAction->text(),"return Function Offset") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblCallstack->item(m_selectedRow,3)->text());
	}
	else if(QString().compare(pAction->text(),"return Module.Function") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblCallstack->item(m_selectedRow,4)->text());
	}
	else if(QString().compare(pAction->text(),"Source Line") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblCallstack->item(m_selectedRow,5)->text());
	}
	else if(QString().compare(pAction->text(),"Source File") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblCallstack->item(m_selectedRow,6)->text());
	}
}

void qtDLGCallstack::OnDisplaySource(QTableWidgetItem *pItem)
{
	QString sourcePath = tblCallstack->item(pItem->row(),6)->text();
	int lineNumber = tblCallstack->item(pItem->row(),5)->text().toInt();

	if(lineNumber <= 0 || sourcePath.isEmpty()) return;

	qtDLGSourceViewer *pSourceView = qtDLGNanomite::GetInstance()->DisAsGUI->dlgSourceViewer;
	pSourceView->OnDisplaySource(sourcePath, lineNumber);
	
	if(pSourceView->IsSourceAvailable)
	{
		pSourceView->show();
	}
	else
	{
		QMessageBox::critical(this, "Nanomite", "Unable to open source file!", QMessageBox::Ok, QMessageBox::Ok);
	}

	return;
}

void qtDLGCallstack::OnCallstack(QList<callstackDisplay> callstackDisplayData)
{
	tblCallstack->setRowCount(0);

	for(int i = 0; i < callstackDisplayData.count(); i++)
	{
		tblCallstack->insertRow(tblCallstack->rowCount());

		// Stack Address
		tblCallstack->setItem(tblCallstack->rowCount() - 1,0,
			new QTableWidgetItem(QString("%1").arg(callstackDisplayData.at(i).stackAddress, 16, 16, QChar('0'))));

		// Func Addr
		tblCallstack->setItem(tblCallstack->rowCount() - 1,1,
			new QTableWidgetItem(QString("%1").arg(callstackDisplayData.at(i).currentOffset, 16, 16, QChar('0'))));

		// <mod.func>
		if(callstackDisplayData.at(i).currentModuleName.length() > 0 && callstackDisplayData.at(i).currentFunctionName.length() > 0)
		{
			tblCallstack->setItem(tblCallstack->rowCount() - 1,2,
				new QTableWidgetItem(QString("%1.%2").arg(callstackDisplayData.at(i).currentModuleName).arg(callstackDisplayData.at(i).currentFunctionName)));
		}
		else if(callstackDisplayData.at(i).currentFunctionName.length() <= 0 && callstackDisplayData.at(i).currentModuleName.length() > 0)
		{
			tblCallstack->setItem(tblCallstack->rowCount() - 1,2,
				new QTableWidgetItem(QString("%1.%2").arg(callstackDisplayData.at(i).currentModuleName).arg(QString("%1").arg(callstackDisplayData.at(i).currentOffset,16,16,QChar('0')))));
		}
		else
		{
			tblCallstack->setItem(tblCallstack->rowCount() - 1,2,
				new QTableWidgetItem(""));
		}

		// Return To
		tblCallstack->setItem(tblCallstack->rowCount() - 1,3,
			new QTableWidgetItem(QString("%1").arg(callstackDisplayData.at(i).returnOffset,16,16,QChar('0'))));

		// Return To <mod.func>
		if(callstackDisplayData.at(i).returnFunctionName.length() > 0 && callstackDisplayData.at(i).returnModuleName.length() > 0)
		{	
			tblCallstack->setItem(tblCallstack->rowCount() - 1,4,
				new QTableWidgetItem(QString("%1.%2").arg(callstackDisplayData.at(i).returnModuleName).arg(callstackDisplayData.at(i).returnFunctionName)));
		}
		else if(callstackDisplayData.at(i).returnFunctionName.length() <= 0 && callstackDisplayData.at(i).returnModuleName.length() > 0)
		{
			tblCallstack->setItem(tblCallstack->rowCount() - 1,2,
				new QTableWidgetItem(QString("%1.%2").arg(callstackDisplayData.at(i).returnModuleName).arg(QString("%1").arg(callstackDisplayData.at(i).returnOffset, 16, 16, QChar('0')))));
		}
		else
		{
			tblCallstack->setItem(tblCallstack->rowCount() - 1,4,
				new QTableWidgetItem(""));
		}

		if(callstackDisplayData.at(i).sourceLineNumber > 0 && callstackDisplayData.at(i).sourceFilePath.length() > 0)
		{
			// Source Line
			tblCallstack->setItem(tblCallstack->rowCount() - 1,5,
				new QTableWidgetItem(QString().sprintf("%d",callstackDisplayData.at(i).sourceLineNumber)));

			// Source File
			tblCallstack->setItem(tblCallstack->rowCount() - 1,6,
				new QTableWidgetItem(callstackDisplayData.at(i).sourceFilePath));
		}
		else
		{
			// Source Line
			tblCallstack->setItem(tblCallstack->rowCount() - 1,5,
				new QTableWidgetItem(""));

			// Source File
			tblCallstack->setItem(tblCallstack->rowCount() - 1,6,
				new QTableWidgetItem(""));
		}
	}
}

void qtDLGCallstack::ShowCallStack()
{
	clsDebugger *pDebugger = qtDLGNanomite::GetInstance()->coreDebugger;
	HANDLE	processHandle = pDebugger->GetCurrentProcessHandle();
	callstackData newCallstack = { 0 };

	newCallstack.processHandle = processHandle;
	newCallstack.threadID = pDebugger->GetCurrentTID();

#ifdef _AMD64_
	BOOL bIsWOW64 = false;

	if(clsAPIImport::pIsWow64Process)
		clsAPIImport::pIsWow64Process(processHandle, &bIsWOW64);

	if(bIsWOW64)
	{
		newCallstack.isWOW64 = true;
		newCallstack.wowProcessContext = pDebugger->wowProcessContext;
	}
	else
	{
		newCallstack.isWOW64 = false;
		newCallstack.processContext = pDebugger->ProcessContext;
	}
#else
	newCallstack.isWOW64 = false;
	newCallstack.processContext = pDebugger->ProcessContext;
#endif

	
	p_ctWorker->setCallstackData(newCallstack);

	QThreadPool::globalInstance()->start(p_ctWorker);
}