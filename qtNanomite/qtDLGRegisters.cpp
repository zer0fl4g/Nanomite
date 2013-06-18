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
			emit OnDisplayDisassembly(tblRegView->item(_iSelectedRow,1)->text().toULongLong(0,16));
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

		PrintValueInTable("EAX",QString("%1").arg(coreDebugger->wowProcessContext.Eax,16,16,QChar('0')));
		PrintValueInTable("EBX",QString("%1").arg(coreDebugger->wowProcessContext.Ebx,16,16,QChar('0')));
		PrintValueInTable("ECX",QString("%1").arg(coreDebugger->wowProcessContext.Ecx,16,16,QChar('0')));
		PrintValueInTable("EDX",QString("%1").arg(coreDebugger->wowProcessContext.Edx,16,16,QChar('0')));
		PrintValueInTable("ESP",QString("%1").arg(coreDebugger->wowProcessContext.Esp,16,16,QChar('0')));
		PrintValueInTable("EBP",QString("%1").arg(coreDebugger->wowProcessContext.Ebp,16,16,QChar('0')));
		PrintValueInTable("ESI",QString("%1").arg(coreDebugger->wowProcessContext.Esi,16,16,QChar('0')));
		PrintValueInTable("EDI",QString("%1").arg(coreDebugger->wowProcessContext.Edi,16,16,QChar('0')));
		PrintValueInTable("EIP",QString("%1").arg(coreDebugger->wowProcessContext.Eip,16,16,QChar('0')));
		PrintValueInTable("SegCs",QString("%1").arg(coreDebugger->wowProcessContext.SegCs,16,16,QChar('0')));
		PrintValueInTable("SegDs",QString("%1").arg(coreDebugger->wowProcessContext.SegDs,16,16,QChar('0')));
		PrintValueInTable("SegEs",QString("%1").arg(coreDebugger->wowProcessContext.SegEs,16,16,QChar('0')));
		PrintValueInTable("SegFs",QString("%1").arg(coreDebugger->wowProcessContext.SegFs,16,16,QChar('0')));
		PrintValueInTable("SegGs",QString("%1").arg(coreDebugger->wowProcessContext.SegGs,16,16,QChar('0')));
		PrintValueInTable("SegSs",QString("%1").arg(coreDebugger->wowProcessContext.SegSs,16,16,QChar('0')));

		//for(int i = 0; i < 8; i++)
		//{
		//	// MMX
		//	DWORD64* pMMX = (DWORD64*)&coreDebugger->wowProcessContext.FloatSave.RegisterArea[i];
		//	PrintValueInTable(QString("MMX%1").arg(i),QString("%1").arg(*pMMX,16,16,QChar('0')));
		//}

		// EFlags
		PrintValueInTable("EFlags",QString("%1").arg(coreDebugger->wowProcessContext.EFlags,16,16,QChar('0')));
	}
	else
	{
		dwEFlags = coreDebugger->ProcessContext.EFlags;
		
		PrintValueInTable("RAX",QString("%1").arg(coreDebugger->ProcessContext.Rax,16,16,QChar('0')));
		PrintValueInTable("RBX",QString("%1").arg(coreDebugger->ProcessContext.Rbx,16,16,QChar('0')));
		PrintValueInTable("RCX",QString("%1").arg(coreDebugger->ProcessContext.Rcx,16,16,QChar('0')));
		PrintValueInTable("RDX",QString("%1").arg(coreDebugger->ProcessContext.Rdx,16,16,QChar('0')));
		PrintValueInTable("RSP",QString("%1").arg(coreDebugger->ProcessContext.Rsp,16,16,QChar('0')));
		PrintValueInTable("RBP",QString("%1").arg(coreDebugger->ProcessContext.Rbp,16,16,QChar('0')));
		PrintValueInTable("RSI",QString("%1").arg(coreDebugger->ProcessContext.Rsi,16,16,QChar('0')));
		PrintValueInTable("RDI",QString("%1").arg(coreDebugger->ProcessContext.Rdi,16,16,QChar('0')));
		PrintValueInTable("RIP",QString("%1").arg(coreDebugger->ProcessContext.Rip,16,16,QChar('0')));
		PrintValueInTable("R8",QString("%1").arg(coreDebugger->ProcessContext.R8,16,16,QChar('0')));
		PrintValueInTable("R9",QString("%1").arg(coreDebugger->ProcessContext.R9,16,16,QChar('0')));
		PrintValueInTable("R10",QString("%1").arg(coreDebugger->ProcessContext.R10,16,16,QChar('0')));
		PrintValueInTable("R11",QString("%1").arg(coreDebugger->ProcessContext.R11,16,16,QChar('0')));
		PrintValueInTable("R12",QString("%1").arg(coreDebugger->ProcessContext.R12,16,16,QChar('0')));
		PrintValueInTable("R13",QString("%1").arg(coreDebugger->ProcessContext.R13,16,16,QChar('0')));
		PrintValueInTable("R14",QString("%1").arg(coreDebugger->ProcessContext.R14,16,16,QChar('0')));
		PrintValueInTable("R15",QString("%1").arg(coreDebugger->ProcessContext.R15,16,16,QChar('0')));		
		PrintValueInTable("SegCs",QString("%1").arg(coreDebugger->ProcessContext.SegCs,16,16,QChar('0')));
		PrintValueInTable("SegDs",QString("%1").arg(coreDebugger->ProcessContext.SegDs,16,16,QChar('0')));
		PrintValueInTable("SegEs",QString("%1").arg(coreDebugger->ProcessContext.SegEs,16,16,QChar('0')));
		PrintValueInTable("SegFs",QString("%1").arg(coreDebugger->ProcessContext.SegFs,16,16,QChar('0')));
		PrintValueInTable("SegGs",QString("%1").arg(coreDebugger->ProcessContext.SegGs,16,16,QChar('0')));
		PrintValueInTable("SegSs",QString("%1").arg(coreDebugger->ProcessContext.SegSs,16,16,QChar('0')));

		//for(int i = 0; i < 8; i++)
		//{
		//	// MMX
		//	DWORD64 MMX_LOW = coreDebugger->ProcessContext.FltSave.FloatRegisters[i].Low;
		//	DWORD64 MMX_HIGH = coreDebugger->ProcessContext.FltSave.FloatRegisters[i].High;

		//	PrintValueInTable(QString("MMX%1").arg(i),QString("%1 %2").arg(MMX_LOW,16,16,QChar('0')).arg(MMX_HIGH,16,16,QChar('0')));
		//}

		// EFlags
		PrintValueInTable("EFlags",QString("%1").arg(coreDebugger->ProcessContext.EFlags,16,16,QChar('0')));
	}
#else
	dwEFlags = coreDebugger->ProcessContext.EFlags;

	PrintValueInTable("EAX",QString("%1").arg(coreDebugger->ProcessContext.Eax,8,16,QChar('0')));
	PrintValueInTable("EBX",QString("%1").arg(coreDebugger->ProcessContext.Ebx,8,16,QChar('0')));
	PrintValueInTable("ECX",QString("%1").arg(coreDebugger->ProcessContext.Ecx,8,16,QChar('0')));
	PrintValueInTable("EDX",QString("%1").arg(coreDebugger->ProcessContext.Edx,8,16,QChar('0')));
	PrintValueInTable("ESP",QString("%1").arg(coreDebugger->ProcessContext.Esp,8,16,QChar('0')));
	PrintValueInTable("EBP",QString("%1").arg(coreDebugger->ProcessContext.Ebp,8,16,QChar('0')));
	PrintValueInTable("ESI",QString("%1").arg(coreDebugger->ProcessContext.Esi,8,16,QChar('0')));
	PrintValueInTable("EDI",QString("%1").arg(coreDebugger->ProcessContext.Edi,8,16,QChar('0')));
	PrintValueInTable("EIP",QString("%1").arg(coreDebugger->ProcessContext.Eip,8,16,QChar('0')));
	PrintValueInTable("SegCs",QString("%1").arg(coreDebugger->ProcessContext.SegCs,8,16,QChar('0')));
	PrintValueInTable("SegDs",QString("%1").arg(coreDebugger->ProcessContext.SegDs,8,16,QChar('0')));
	PrintValueInTable("SegEs",QString("%1").arg(coreDebugger->ProcessContext.SegEs,8,16,QChar('0')));
	PrintValueInTable("SegFs",QString("%1").arg(coreDebugger->ProcessContext.SegFs,8,16,QChar('0')));
	PrintValueInTable("SegGs",QString("%1").arg(coreDebugger->ProcessContext.SegGs,8,16,QChar('0')));
	PrintValueInTable("SegSs",QString("%1").arg(coreDebugger->ProcessContext.SegSs,8,16,QChar('0')));

	//for(int i = 0; i < 8; i++)
	//{
		// MMX
		//DWORD64* pMMX = (DWORD64*)&coreDebugger->ProcessContext.FloatSave.RegisterArea[i];
		//PrintValueInTable(QString("MMX%1").arg(i),QString("%1").arg(*pMMX,16,16,QChar('0')));
	//}

	// EFlags
	PrintValueInTable("EFlags",QString("%1").arg(coreDebugger->ProcessContext.EFlags,8,16,QChar('0')));
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

	PrintValueInTable("OF",QString("%1").arg(bOF));
	PrintValueInTable("DF",QString("%1").arg(bDF));
	PrintValueInTable("TF",QString("%1").arg(bTF));
	PrintValueInTable("SF",QString("%1").arg(bSF));
	PrintValueInTable("ZF",QString("%1").arg(bZF));
	PrintValueInTable("AF",QString("%1").arg(bAF));
	PrintValueInTable("PF",QString("%1").arg(bPF));
	PrintValueInTable("CF",QString("%1").arg(bCF));
}

void qtDLGRegisters::PrintValueInTable(QString regName, QString regValue)
{
	tblRegView->insertRow(tblRegView->rowCount());
	tblRegView->setItem(tblRegView->rowCount() - 1,0,new QTableWidgetItem(regName));
	tblRegView->setItem(tblRegView->rowCount() - 1,1,new QTableWidgetItem(regValue));
}