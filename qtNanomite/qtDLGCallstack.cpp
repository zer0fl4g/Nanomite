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

#include <QClipboard>
#include <QMenu>

using namespace std;

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

	connect(tblCallstack,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnContextMenu(QPoint)));
	connect(tblCallstack,SIGNAL(itemDoubleClicked(QTableWidgetItem *)),this,SLOT(OnDisplaySource(QTableWidgetItem *)));
}

qtDLGCallstack::~qtDLGCallstack()
{

}

void qtDLGCallstack::OnContextMenu(QPoint qPoint)
{
	QMenu menu;

	_iSelectedRow = tblCallstack->indexAt(qPoint).row();
	if(_iSelectedRow < 0) return;

	menu.addAction(new QAction("Send to Disassembler",this));
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
		if(!pMainWindow->coreDisAs->InsertNewDisassembly(pMainWindow->coreDebugger->GetCurrentProcessHandle(),tblCallstack->item(_iSelectedRow,1)->text().toULongLong(0,16)))
				pMainWindow->OnDisplayDisassembly(tblCallstack->item(_iSelectedRow,1)->text().toULongLong(0,16));	
	}	
	else if(QString().compare(pAction->text(),"Line") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(QString("%1:%2:%3:%4:%5:%6:%7")
			.arg(tblCallstack->item(_iSelectedRow,0)->text())
			.arg(tblCallstack->item(_iSelectedRow,1)->text())
			.arg(tblCallstack->item(_iSelectedRow,2)->text())
			.arg(tblCallstack->item(_iSelectedRow,3)->text())
			.arg(tblCallstack->item(_iSelectedRow,4)->text())
			.arg(tblCallstack->item(_iSelectedRow,5)->text())
			.arg(tblCallstack->item(_iSelectedRow,6)->text()));
	}
	else if(QString().compare(pAction->text(),"Stack Offset") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblCallstack->item(_iSelectedRow,0)->text());
	}
	else if(QString().compare(pAction->text(),"current Function Offset") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblCallstack->item(_iSelectedRow,1)->text());
	}
	else if(QString().compare(pAction->text(),"current Module.Function") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblCallstack->item(_iSelectedRow,2)->text());
	}
	else if(QString().compare(pAction->text(),"return Function Offset") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblCallstack->item(_iSelectedRow,3)->text());
	}
	else if(QString().compare(pAction->text(),"return Module.Function") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblCallstack->item(_iSelectedRow,4)->text());
	}
	else if(QString().compare(pAction->text(),"Source Line") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblCallstack->item(_iSelectedRow,5)->text());
	}
	else if(QString().compare(pAction->text(),"Source File") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblCallstack->item(_iSelectedRow,6)->text());
	}
}

void qtDLGCallstack::OnDisplaySource(QTableWidgetItem *pItem)
{
	QString sourcePath = tblCallstack->item(pItem->row(),6)->text();
	int LineNumber = tblCallstack->item(pItem->row(),5)->text().toInt();

	if(LineNumber <= 0 && sourcePath.isEmpty()) return;

	emit DisplaySource(sourcePath,LineNumber);

	qtDLGNanomite::GetInstance()->dlgSourceViewer->show();
	return;
}

int qtDLGCallstack::OnCallStack(quint64 dwStackAddr,
						 quint64 dwReturnTo,wstring sReturnToFunc,wstring sModuleName,
						 quint64 dwEIP,wstring sFuncName,wstring sFuncModule,
						 wstring sSourceFilePath,int iSourceLineNum)
{
	tblCallstack->insertRow(tblCallstack->rowCount());

	// Stack Address
	tblCallstack->setItem(tblCallstack->rowCount() - 1,0,
		new QTableWidgetItem(QString("%1").arg(dwStackAddr,16,16,QChar('0'))));

	// Func Addr
	tblCallstack->setItem(tblCallstack->rowCount() - 1,1,
		new QTableWidgetItem(QString("%1").arg(dwEIP,16,16,QChar('0'))));

	// <mod.func>
	if(sFuncModule.length() > 0 && sFuncName.length() > 0)
		tblCallstack->setItem(tblCallstack->rowCount() - 1,2,
			new QTableWidgetItem(QString::fromStdWString(sFuncModule).append(".").append(QString::fromStdWString(sFuncName))));
	else if(sFuncModule.length() > 0 && sFuncName.length() <= 0)
		tblCallstack->setItem(tblCallstack->rowCount() - 1,2,
			new QTableWidgetItem(QString::fromStdWString(sFuncModule).append(".").append(QString("%1").arg(dwEIP,16,16,QChar('0')))));
	else
		tblCallstack->setItem(tblCallstack->rowCount() - 1,24,
			new QTableWidgetItem(""));

	// Return To
	tblCallstack->setItem(tblCallstack->rowCount() - 1,3,
		new QTableWidgetItem(QString("%1").arg(dwReturnTo,16,16,QChar('0'))));

	// Return To <mod.func>
	if(sFuncName.length() > 0 && sModuleName.length() > 0)
		tblCallstack->setItem(tblCallstack->rowCount() - 1,4,
			new QTableWidgetItem(QString::fromStdWString(sModuleName).append(".").append(QString::fromStdWString(sReturnToFunc))));
	else if(sFuncName.length() <= 0 && sModuleName.length() > 0)
		tblCallstack->setItem(tblCallstack->rowCount() - 1,4,
			new QTableWidgetItem(QString::fromStdWString(sModuleName).append(".").append(QString("%1").arg(dwReturnTo,16,16,QChar('0')))));
	else
		tblCallstack->setItem(tblCallstack->rowCount() - 1,4,
			new QTableWidgetItem(""));

	// Source Line
	tblCallstack->setItem(tblCallstack->rowCount() - 1,5,
		new QTableWidgetItem(QString().sprintf("%d",iSourceLineNum)));
	
	// Source File
	tblCallstack->setItem(tblCallstack->rowCount() - 1,6,
		new QTableWidgetItem(QString::fromStdWString(sSourceFilePath)));

	return 0;
}
