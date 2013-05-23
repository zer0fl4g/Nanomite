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
#include "qtDLGRegisters.h"
#include "qtDLGNanomite.h"
#include "qtDLGRegEdit.h"

#include "clsMemManager.h"
#include "clsAPIImport.h"

#include <QClipboard>
#include <QMenu>

qtDLGRegisters::qtDLGRegisters(QWidget *parent)
	: QDockWidget(parent)
{
	setupUi(this);

	// List Register
	tblRegView->horizontalHeader()->resizeSection(0,75);
	tblRegView->horizontalHeader()->resizeSection(1,100);

	connect(tblRegView,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnContextMenu(QPoint)));
	connect(tblRegView,SIGNAL(itemDoubleClicked(QTableWidgetItem *)),this,SLOT(OnChangeRequest(QTableWidgetItem *)));
}

qtDLGRegisters::~qtDLGRegisters()
{

}

void qtDLGRegisters::OnContextMenu(QPoint qPoint)
{
	QMenu menu;

	_iSelectedRow = tblRegView->indexAt(qPoint).row();
	if(_iSelectedRow < 0) return;


	menu.addAction(new QAction("Send to StackView",this));
	menu.addAction(new QAction("Send to Disassembler",this));
	QMenu *submenu = menu.addMenu("Copy to Clipboard");
	submenu->addAction(new QAction("Line",this));
	submenu->addAction(new QAction("Value",this));

	menu.addMenu(submenu);
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));

	menu.exec(QCursor::pos());
}

void qtDLGRegisters::MenuCallback(QAction* pAction)
{
	qtDLGNanomite *pMainWindow = qtDLGNanomite::GetInstance();

	if(!pMainWindow->coreDebugger->GetDebuggingState()) return;

	if(QString().compare(pAction->text(),"Send to Disassembler") == 0)
	{
		if(!pMainWindow->coreDisAs->InsertNewDisassembly(pMainWindow->coreDebugger->GetCurrentProcessHandle(),tblRegView->item(_iSelectedRow,1)->text().toULongLong(0,16)))
			pMainWindow->OnDisplayDisassembly(tblRegView->item(_iSelectedRow,1)->text().toULongLong(0,16));
	}
	else if(QString().compare(pAction->text(),"Send to StackView") == 0)
	{
#ifdef _AMD64_
		BOOL bIsWOW64 = false;
		if(clsAPIImport::pIsWow64Process)
			clsAPIImport::pIsWow64Process(pMainWindow->coreDebugger->GetCurrentProcessHandle(),&bIsWOW64);

		if(bIsWOW64)
			pMainWindow->stackView->LoadStackView(tblRegView->item(_iSelectedRow,1)->text().toULongLong(0,16),4);
		else
			pMainWindow->stackView->LoadStackView(tblRegView->item(_iSelectedRow,1)->text().toULongLong(0,16),8);
#else
		pMainWindow->stackView->LoadStackView(tblRegView->item(_iSelectedRow,1)->text().toULongLong(0,16),4);
#endif
	}
	else if(QString().compare(pAction->text(),"Line") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(QString("%1:%2").arg(tblRegView->item(_iSelectedRow,0)->text()).arg(tblRegView->item(_iSelectedRow,1)->text()));
	}
	else if(QString().compare(pAction->text(),"Value") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblRegView->item(_iSelectedRow,1)->text());
	}
}

void qtDLGRegisters::OnChangeRequest(QTableWidgetItem *pItem)
{
	qtDLGNanomite *pMainWindow = qtDLGNanomite::GetInstance();

	if(!pMainWindow->coreDebugger->GetDebuggingState())
		return;

	qtDLGRegEdit *newRegEditWindow;
#ifdef _AMD64_
	BOOL bIsWOW64 = false;
	if(clsAPIImport::pIsWow64Process)
		clsAPIImport::pIsWow64Process(pMainWindow->coreDebugger->GetCurrentProcessHandle(),&bIsWOW64);

	if(bIsWOW64)
		newRegEditWindow = new qtDLGRegEdit(this,Qt::Window,&pMainWindow->coreDebugger->wowProcessContext,false);
	else
		newRegEditWindow = new qtDLGRegEdit(this,Qt::Window,&pMainWindow->coreDebugger->ProcessContext,true);			
#else
	newRegEditWindow = new qtDLGRegEdit(this,Qt::Window,&pMainWindow->coreDebugger->ProcessContext,false);			
#endif	

	connect(newRegEditWindow,SIGNAL(OnUpdateRegView()),pMainWindow,SLOT(LoadRegView()));
	newRegEditWindow->exec();
}

void qtDLGRegisters::LoadRegView(clsDebugger *coreDebugger)
{
	tblRegView->setRowCount(0);
	DWORD dwEFlags = NULL;

#ifdef _AMD64_
	BOOL bIsWOW64 = false;

	if(clsAPIImport::pIsWow64Process)
		clsAPIImport::pIsWow64Process(coreDebugger->GetCurrentProcessHandle(),&bIsWOW64);

	if(bIsWOW64)
	{
		dwEFlags = coreDebugger->wowProcessContext.EFlags;
		// EAX
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("EAX"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->wowProcessContext.Eax,16,16,QChar('0'))));
		// EBX
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("EBX"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->wowProcessContext.Ebx,16,16,QChar('0'))));
		// ECX
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("ECX"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->wowProcessContext.Ecx,16,16,QChar('0'))));
		// EDX
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("EDX"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->wowProcessContext.Edx,16,16,QChar('0'))));
		// ESP
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("ESP"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->wowProcessContext.Esp,16,16,QChar('0'))));
		// EBP
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("EBP"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->wowProcessContext.Ebp,16,16,QChar('0'))));
		// ESI
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("ESI"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->wowProcessContext.Esi,16,16,QChar('0'))));
		// EIP
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("EIP"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->wowProcessContext.Eip,16,16,QChar('0'))));
		// EFlags
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("EFlags"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->wowProcessContext.EFlags,16,16,QChar('0'))));
	}
	else
	{
		dwEFlags = coreDebugger->ProcessContext.EFlags;
		// RAX
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("RAX"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.Rax,16,16,QChar('0'))));
		// RBX
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("RBX"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.Rbx,16,16,QChar('0'))));
		// RCX
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("RCX"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.Rcx,16,16,QChar('0'))));
		// RDX
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("RDX"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.Rdx,16,16,QChar('0'))));
		// RSP
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("RSP"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.Rsp,16,16,QChar('0'))));
		// RBP
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("RBP"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.Rbp,16,16,QChar('0'))));
		// RSI
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("RSI"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.Rsi,16,16,QChar('0'))));
		// RIP
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("RIP"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.Rip,16,16,QChar('0'))));
		// R8
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("R8"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.R8,16,16,QChar('0'))));
		// R9
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("R9"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.R9,16,16,QChar('0'))));
		// R10
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("R10"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.R10,16,16,QChar('0'))));
		// R11
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("R11"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.R11,16,16,QChar('0'))));
		// R12
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("R12"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.R12,16,16,QChar('0'))));
		// R13
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("R13"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.R13,16,16,QChar('0'))));
		// R14
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("R14"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.R14,16,16,QChar('0'))));
		// R15
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("R15"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.R15,16,16,QChar('0'))));
		// EFlags
		tblRegView->insertRow(tblRegView->rowCount());
		tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("EFlags"));
		tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.EFlags,8,16,QChar('0'))));
	}
#else
	dwEFlags = coreDebugger->ProcessContext.EFlags;
	// EAX
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("EAX"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.Eax,16,16,QChar('0'))));
	// EBX
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("EBX"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.Ebx,16,16,QChar('0'))));
	// ECX
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("ECX"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.Ecx,16,16,QChar('0'))));
	// EDX
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("EDX"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.Edx,16,16,QChar('0'))));
	// ESP
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("ESP"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.Esp,16,16,QChar('0'))));
	// EBP
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("EBP"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.Ebp,16,16,QChar('0'))));
	// ESI
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("ESI"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.Esi,16,16,QChar('0'))));
	// EIP
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("EIP"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.Eip,16,16,QChar('0'))));
	// EFlags
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("EFlags"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(coreDebugger->ProcessContext.EFlags,16,16,QChar('0'))));
#endif

	BOOL bCF = false, // Carry Flag
		bPF = false, // Parity Flag
		bAF = false, // Auxiliarty carry flag
		bZF = false, // Zero Flag
		bSF = false, // Sign Flag
		bTF = false, // Trap Flag
		bIF = false, // Interrupt Flag
		bDF = false, // Direction Flag
		bOF = false; // Overflow Flag

	bOF = (dwEFlags & 0x800) ? true : false;
	bDF = (dwEFlags & 0x400) ? true : false;
	bTF = (dwEFlags & 0x100) ? true : false;
	bSF = (dwEFlags & 0x80) ? true : false;
	bZF = (dwEFlags & 0x40) ? true : false;
	bAF = (dwEFlags & 0x10) ? true : false;
	bPF = (dwEFlags & 0x4) ? true : false;
	bCF = (dwEFlags & 0x1) ? true : false;

	// OF
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("OF"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(bOF)));
	// DF
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("DF"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(bDF)));
	// TF
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("TF"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(bTF)));
	// SF
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("SF"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(bSF)));
	// ZF
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("ZF"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(bZF)));
	// AF
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("AF"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(bAF)));
	// PF
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("PF"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(bPF)));
	// CF
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem("CF"));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(bCF)));
}