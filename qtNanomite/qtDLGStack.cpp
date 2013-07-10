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
#include "qtDLGStack.h"
#include "qtDLGNanomite.h"

#include "clsMemManager.h"
#include "clsAPIImport.h"
#include "clsHelperClass.h"

#include <string>

using namespace std;

qtDLGStack::qtDLGStack(QWidget *parent)
	: QDockWidget(parent)
{
	setupUi(this);

	// eventFilter for mouse scroll
	tblStack->installEventFilter(this);
    tblStack->viewport()->installEventFilter(this);

	// List StackView
	tblStack->horizontalHeader()->resizeSection(0,135);
	tblStack->horizontalHeader()->resizeSection(1,135);
	tblStack->horizontalHeader()->resizeSection(2,300);
	tblStack->horizontalHeader()->setFixedHeight(21);

	connect(scrollStackView,SIGNAL(valueChanged(int)),this,SLOT(OnStackScroll(int)));
	connect(tblStack,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnContextMenu(QPoint)));
}

qtDLGStack::~qtDLGStack()
{

}

void qtDLGStack::LoadStackView(quint64 stackBaseOffset, DWORD stackAlign)
{
	clsDebugger *coreDebugger = qtDLGNanomite::GetInstance()->coreDebugger;

	if(!coreDebugger->GetDebuggingState())
		return;

	bool bCheckVar = false;
	SIZE_T dwBytesRead = NULL;
	wstring sFuncName,sModName;
	LPBYTE bBuffer;
	PTCHAR sTemp;
	HANDLE hProcess = coreDebugger->GetCurrentProcessHandle();
	DWORD dwOldProtect = NULL,
		dwNewProtect = PAGE_READWRITE,
		dwRowCount = (tblStack->verticalHeader()->height() / 11),
		dwSize = dwRowCount * stackAlign;
	quint64	dwStartOffset = stackBaseOffset - stackAlign * (dwRowCount / 2),
		dwEndOffset = stackBaseOffset + stackAlign * (dwRowCount / 2);

	if(hProcess == INVALID_HANDLE_VALUE)
		return;

	if(!VirtualProtectEx(hProcess,(LPVOID)dwStartOffset,dwSize,dwNewProtect,&dwOldProtect))
		return;

	bBuffer = (LPBYTE)clsMemManager::CAlloc(dwSize);
	if(bBuffer == NULL)
		return;

	if(!ReadProcessMemory(hProcess,(LPVOID)dwStartOffset,(LPVOID)bBuffer,dwSize,&dwBytesRead))
	{
		clsMemManager::CFree(bBuffer);
		return;
	}

	sTemp = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));

	tblStack->setRowCount(0);
	for(size_t i = 0; i < dwRowCount; i++)
	{
		tblStack->insertRow(tblStack->rowCount());
		int itemIndex = tblStack->rowCount();

		// Current Offset
		wsprintf(sTemp,L"%016I64X",(dwStartOffset + i * stackAlign));
		tblStack->setItem(itemIndex - 1,0,new QTableWidgetItem(QString::fromWCharArray(sTemp)));

		// Value
		memset(sTemp,0,MAX_PATH * sizeof(TCHAR));
#ifdef _AMD64_
		BOOL bIsWOW64 = false;
		IsWow64Process(coreDebugger->GetCurrentProcessHandle(),&bIsWOW64);

		if(bIsWOW64)
			for(int id = 3;id != -1;id--)
				wsprintf(sTemp,L"%s%02X",sTemp,*(bBuffer + (i * stackAlign + id)));
		else
			for(int id = 7;id != -1;id--)
				wsprintf(sTemp,L"%s%02X",sTemp,*(bBuffer + (i * stackAlign + id)));

#else
		for(int id = 3;id != -1;id--)
			wsprintf(sTemp,L"%s%02X",sTemp,*(bBuffer + (i * stackAlign + id)));
#endif
		tblStack->setItem(itemIndex - 1,1,new QTableWidgetItem(QString::fromWCharArray(sTemp)));

		// Comment
		clsHelperClass::LoadSymbolForAddr(sFuncName,sModName,QString::fromWCharArray(sTemp).toULongLong(0,16),
			coreDebugger->GetCurrentProcessHandle());
		if(sFuncName.length() > 0 && sModName.length() > 0)
			tblStack->setItem(itemIndex - 1,2,
			new QTableWidgetItem(QString::fromStdWString(sModName).append(".").append(QString::fromStdWString(sFuncName))));
		else if(sFuncName.length() > 0)
			tblStack->setItem(itemIndex - 1,2,new QTableWidgetItem(QString::fromStdWString(sFuncName)));	
		else
			tblStack->setItem(itemIndex- 1,2,new QTableWidgetItem(""));
	}
	
	bCheckVar = VirtualProtectEx(hProcess,(LPVOID)dwStartOffset,dwSize,dwOldProtect,NULL);
	clsMemManager::CFree(bBuffer);
	clsMemManager::CFree(sTemp);
}

void qtDLGStack::OnStackScroll(int iValue)
{
	if(iValue == 5) return;

	clsDebugger *coreDebugger = qtDLGNanomite::GetInstance()->coreDebugger;

	DWORD stackAlign = NULL;
	quint64 dwOffset = NULL;
	QString strTemp;

#ifdef _AMD64_
	BOOL bIsWOW64 = false;
	if(clsAPIImport::pIsWow64Process)
		clsAPIImport::pIsWow64Process(coreDebugger->GetCurrentProcessHandle(),&bIsWOW64);

	if(bIsWOW64) stackAlign = 4;	
	else stackAlign = 8;
#else
	stackAlign = 4;
#endif

	if(tblStack->rowCount() > 0)
		strTemp = tblStack->item(0,0)->text();
	if(tblStack->rowCount() <= 0 || QString().compare(strTemp,"0") == 0)
#ifdef _AMD64_
		dwOffset = coreDebugger->ProcessContext.Rsp;
#else
		dwOffset = coreDebugger->ProcessContext.Esp;
#endif
	else
		dwOffset = strTemp.toULongLong(0,16);

	if(iValue < 5)
		LoadStackView(dwOffset,stackAlign);
	else
	{
		if((tblStack->verticalHeader()->height() / 14) % 2)
			LoadStackView(dwOffset + (stackAlign * (tblStack->verticalHeader()->height() / 14)),stackAlign);
		else
			LoadStackView(dwOffset + (stackAlign * ((tblStack->verticalHeader()->height() / 14) + 1)),stackAlign);
	}

	scrollStackView->setValue(5);
}

bool qtDLGStack::eventFilter(QObject *pObject, QEvent *event)
{	
	if(event->type() == QEvent::Wheel && pObject == tblStack)
	{
		QWheelEvent *pWheel = (QWheelEvent*)event;

		OnStackScroll(pWheel->delta() * -1);
		return true;
	}
	return false;
}

void qtDLGStack::OnContextMenu(QPoint qPoint)
{
	QMenu menu;

	m_selectedRow = tblStack->indexAt(qPoint).row();
	if(m_selectedRow < 0) return;

	menu.addAction(new QAction("Send to Disassembler",this));
	QMenu *submenu = menu.addMenu("Copy to Clipboard");
	submenu->addAction(new QAction("Line",this));
	submenu->addAction(new QAction("Offset",this));
	submenu->addAction(new QAction("OpCodes",this));
	submenu->addAction(new QAction("Mnemonic",this));
	submenu->addAction(new QAction("Comment",this));

	menu.addMenu(submenu);
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));

	menu.exec(QCursor::pos());
}

void qtDLGStack::MenuCallback(QAction* pAction)
{
	if(QString().compare(pAction->text(),"Line") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(QString("%1:%2:%3:%4")
			.arg(tblStack->item(m_selectedRow,0)->text())
			.arg(tblStack->item(m_selectedRow,1)->text())
			.arg(tblStack->item(m_selectedRow,2)->text())
			.arg(tblStack->item(m_selectedRow,3)->text()));
	}
	else if(QString().compare(pAction->text(),"Offset") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblStack->item(m_selectedRow,0)->text());
	}
	else if(QString().compare(pAction->text(),"OpCodes") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblStack->item(m_selectedRow,1)->text());
	}
	else if(QString().compare(pAction->text(),"Mnemonics") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblStack->item(m_selectedRow,2)->text());
	}
	else if(QString().compare(pAction->text(),"Comment") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblStack->item(m_selectedRow,3)->text());
	}
}