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
#include "dbghelp.h"

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

	connect(tblCallstack,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnContextMenu(QPoint)));
	connect(tblCallstack,SIGNAL(itemDoubleClicked(QTableWidgetItem *)),this,SLOT(OnDisplaySource(QTableWidgetItem *)));
}

qtDLGCallstack::~qtDLGCallstack()
{

}

void qtDLGCallstack::OnContextMenu(QPoint qPoint)
{
	QMenu menu;

	m_selectedRow = tblCallstack->indexAt(qPoint).row();
	if(m_selectedRow < 0) return;

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

void qtDLGCallstack::OnCallStack(quint64 stackAddress,
						 quint64 returnOffset, QString returnFunctionName, QString returnModuleName,
						 quint64 currentOffset, QString currentFunctionName, QString currentModuleName,
						 QString sourceFilePath, int sourceLineNumber)
{
	tblCallstack->insertRow(tblCallstack->rowCount());

	// Stack Address
	tblCallstack->setItem(tblCallstack->rowCount() - 1,0,
		new QTableWidgetItem(QString("%1").arg(stackAddress,16,16,QChar('0'))));

	// Func Addr
	tblCallstack->setItem(tblCallstack->rowCount() - 1,1,
		new QTableWidgetItem(QString("%1").arg(currentOffset,16,16,QChar('0'))));

	// <mod.func>
	if(currentModuleName.length() > 0 && currentFunctionName.length() > 0)
	{
		tblCallstack->setItem(tblCallstack->rowCount() - 1,2,
			new QTableWidgetItem(currentModuleName.append(".").append(currentFunctionName)));
	}
	else if(currentModuleName.length() > 0 && currentFunctionName.length() <= 0)
	{
		tblCallstack->setItem(tblCallstack->rowCount() - 1,2,
			new QTableWidgetItem(currentModuleName.append(".").append(QString("%1").arg(currentOffset,16,16,QChar('0')))));
	}
	else
	{
		tblCallstack->setItem(tblCallstack->rowCount() - 1,2,
			new QTableWidgetItem(""));
	}

	// Return To
	tblCallstack->setItem(tblCallstack->rowCount() - 1,3,
		new QTableWidgetItem(QString("%1").arg(returnOffset,16,16,QChar('0'))));

	// Return To <mod.func>
	if(returnFunctionName.length() > 0 && returnModuleName.length() > 0)
	{	
		tblCallstack->setItem(tblCallstack->rowCount() - 1,4,
			new QTableWidgetItem(returnModuleName.append(".").append(returnFunctionName)));
	}
	else if(returnFunctionName.length() <= 0 && returnModuleName.length() > 0)
	{
		tblCallstack->setItem(tblCallstack->rowCount() - 1,4,
			new QTableWidgetItem(returnModuleName.append(".").append(QString("%1").arg(returnOffset,16,16,QChar('0')))));
	}
	else
	{
		tblCallstack->setItem(tblCallstack->rowCount() - 1,4,
			new QTableWidgetItem(""));
	}

	if(sourceLineNumber > 0 && sourceFilePath.length() > 0)
	{
		// Source Line
		tblCallstack->setItem(tblCallstack->rowCount() - 1,5,
			new QTableWidgetItem(QString().sprintf("%d",sourceLineNumber)));

		// Source File
		tblCallstack->setItem(tblCallstack->rowCount() - 1,6,
			new QTableWidgetItem(sourceFilePath));
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

void qtDLGCallstack::ShowCallStack()
{
	clsDebugger *pDebugger = qtDLGNanomite::GetInstance()->coreDebugger;
	
	HANDLE	hProc = pDebugger->GetCurrentProcessHandle(),
			hThread = OpenThread(THREAD_GETSET_CONTEXT,false,pDebugger->GetCurrentTID());
	PSYMBOL_INFOW pSymbol = (PSYMBOL_INFOW)malloc(sizeof(SYMBOL_INFOW) + MAX_PATH * 2);
	DWORD dwMaschineMode = NULL;
	LPVOID pContext;
	STACKFRAME64 stackFr = {0};
	stackFr.AddrPC.Mode = AddrModeFlat;
	stackFr.AddrFrame.Mode = AddrModeFlat;
	stackFr.AddrStack.Mode = AddrModeFlat;

	QString sFuncName,
		sFuncMod,
		sReturnToFunc,
		sReturnToMod;
	quint64 dwStackAddr,
		dwReturnTo,
		dwEIP,
		dwDisplacement;
	IMAGEHLP_LINEW64 imgSource = {0};
	IMAGEHLP_MODULEW64 imgMod = {0};
	BOOL bSuccess;

#ifdef _AMD64_
	BOOL bIsWOW64 = false;

	if(clsAPIImport::pIsWow64Process)
		clsAPIImport::pIsWow64Process(hProc,&bIsWOW64);

	if(bIsWOW64)
	{
		dwMaschineMode = IMAGE_FILE_MACHINE_I386;
		WOW64_CONTEXT cTT;
		cTT = pDebugger->wowProcessContext;
		pContext = &cTT;

		stackFr.AddrPC.Offset = cTT.Eip;
		stackFr.AddrFrame.Offset = cTT.Ebp;
		stackFr.AddrStack.Offset = cTT.Esp;	
	}
	else
	{
		dwMaschineMode = IMAGE_FILE_MACHINE_AMD64;
		CONTEXT cTT;
		cTT = pDebugger->ProcessContext;
		pContext = &cTT;

		stackFr.AddrPC.Offset = cTT.Rip;
		stackFr.AddrFrame.Offset = cTT.Rbp;
		stackFr.AddrStack.Offset = cTT.Rsp;	
	}
#else
	dwMaschineMode = IMAGE_FILE_MACHINE_I386;
	CONTEXT cTT;
	cTT = pDebugger->ProcessContext;
	pContext = &cTT;

	stackFr.AddrPC.Offset = cTT.Eip;
	stackFr.AddrFrame.Offset = cTT.Ebp;
	stackFr.AddrStack.Offset = cTT.Esp;	

#endif

	tblCallstack->setRowCount(0);

	do
	{
		bSuccess = StackWalk64(dwMaschineMode,hProc,hThread,&stackFr,pContext,NULL,SymFunctionTableAccess64,SymGetModuleBase64,0);

		if(!bSuccess)        
			break;

		memset(&imgSource,0,sizeof(IMAGEHLP_LINEW64));
		imgSource.SizeOfStruct = sizeof(IMAGEHLP_LINEW64);

		dwStackAddr = stackFr.AddrStack.Offset;
		dwEIP = stackFr.AddrPC.Offset;
		dwReturnTo = stackFr.AddrReturn.Offset;


		memset(&imgMod,0,sizeof(IMAGEHLP_MODULEW64));
		imgMod.SizeOfStruct = sizeof(IMAGEHLP_MODULEW64);
		memset(pSymbol,0,sizeof(SYMBOL_INFOW) + MAX_PATH * 2);
		pSymbol->SizeOfStruct = sizeof(SYMBOL_INFOW);
		pSymbol->MaxNameLen = MAX_PATH;

		bSuccess = SymGetModuleInfoW64(hProc,dwEIP,&imgMod);
		bSuccess = SymFromAddrW(hProc,dwEIP,&dwDisplacement,pSymbol);
		sFuncName = QString::fromWCharArray(pSymbol->Name);
		sFuncMod = QString::fromWCharArray(imgMod.ModuleName);


		memset(&imgMod,0,sizeof(IMAGEHLP_MODULEW64));
		imgMod.SizeOfStruct = sizeof(IMAGEHLP_MODULEW64);
		memset(pSymbol,0,sizeof(SYMBOL_INFOW) + MAX_PATH * 2);
		pSymbol->SizeOfStruct = sizeof(SYMBOL_INFOW);
		pSymbol->MaxNameLen = MAX_PATH;

		bSuccess = SymGetModuleInfoW64(hProc,dwReturnTo,&imgMod);
		bSuccess = SymFromAddrW(hProc,dwReturnTo,&dwDisplacement,pSymbol);
		sReturnToMod = QString::fromWCharArray(imgMod.ModuleName);
		sReturnToFunc = QString::fromWCharArray(pSymbol->Name);

		bSuccess = SymGetLineFromAddrW64(hProc,dwEIP,(PDWORD)&dwDisplacement,&imgSource);

		if(bSuccess)
			OnCallStack(dwStackAddr,
				dwReturnTo,sReturnToFunc,sReturnToMod,
				dwEIP,sFuncName,sFuncMod,
				QString::fromWCharArray(imgSource.FileName),imgSource.LineNumber);
		else
			OnCallStack(dwStackAddr,
				dwReturnTo,sReturnToFunc,sReturnToMod,
				dwEIP,sFuncName,sFuncMod,
				QString(""),0);

	}while(stackFr.AddrReturn.Offset != 0);

	free(pSymbol);
	CloseHandle(hThread);
}