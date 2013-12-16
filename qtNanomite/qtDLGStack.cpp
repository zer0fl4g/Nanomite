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
#include "clsMemoryProtector.h"

qtDLGStack::qtDLGStack(QWidget *parent)
	: QDockWidget(parent),
	m_pCoreDebugger(qtDLGNanomite::GetInstance()->coreDebugger)
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
	QString sFuncName,sModName;
	HANDLE	hProcess		= m_pCoreDebugger->GetCurrentProcessHandle();
	int		maxRows			= (tblStack->verticalHeader()->height() / 11) - 1;
	DWORD	dwSize			= (maxRows + 1) * stackAlign;
	int		lineCount		= NULL;
	LPBYTE	bBuffer			= (LPBYTE)clsMemManager::CAlloc(dwSize);
	PTCHAR	sTemp			= (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));
	quint64	dwStartOffset	= stackBaseOffset - stackAlign;
	bool	worked			= false;

	if(hProcess == INVALID_HANDLE_VALUE)
	{
		clsMemManager::CFree(bBuffer);
		clsMemManager::CFree(sTemp);
		return;
	}

	clsMemoryProtector tempMemProtect(hProcess, PAGE_EXECUTE_READWRITE, dwSize, dwStartOffset, &worked);

	if(!ReadProcessMemory(hProcess,(LPVOID)dwStartOffset,(LPVOID)bBuffer,dwSize,NULL))
	{
		clsMemManager::CFree(bBuffer);
		clsMemManager::CFree(sTemp);
		return;
	}

	if((tblStack->rowCount() - 1) != maxRows)
	{
		tblStack->setRowCount(0);
		while(lineCount <= maxRows)
		{
			tblStack->insertRow(0);
			lineCount++;
		}
		lineCount = 0;
	}

	while(lineCount <= maxRows)
	{
		// Current Offset
		tblStack->setItem(lineCount, 0, new QTableWidgetItem(QString("%1").arg((dwStartOffset + lineCount * stackAlign),16,16,QChar('0'))));

		// Value
		memset(sTemp,0,MAX_PATH * sizeof(TCHAR));
		for(int id = stackAlign - 1;id != -1;id--)
			wsprintf(sTemp,L"%s%02X",sTemp,*(bBuffer + (lineCount * stackAlign + id)));
		tblStack->setItem(lineCount, 1, new QTableWidgetItem(QString::fromWCharArray(sTemp)));

		// Comment
		clsHelperClass::LoadSymbolForAddr(sFuncName,sModName,QString::fromWCharArray(sTemp).toULongLong(0,16),hProcess);
		if(sFuncName.length() > 0 && sModName.length() > 0)
			tblStack->setItem(lineCount, 2,
			new QTableWidgetItem(sModName.append(".").append(sFuncName)));
		else
			tblStack->setItem(lineCount, 2, new QTableWidgetItem(""));

		lineCount++;
	}
	
	clsMemManager::CFree(bBuffer);
	clsMemManager::CFree(sTemp);
}

void qtDLGStack::OnStackScroll(int iValue)
{
	if(iValue == 5 || !m_pCoreDebugger->GetDebuggingState()) return;
	
	DWORD stackAlign = NULL;
	quint64 dwOffset = NULL;

#ifdef _AMD64_
	BOOL bIsWOW64 = false;
	if(clsAPIImport::pIsWow64Process)
		clsAPIImport::pIsWow64Process(m_pCoreDebugger->GetCurrentProcessHandle(),&bIsWOW64);

	if(bIsWOW64)
		stackAlign = 4;	
	else
		stackAlign = 8;
#else
	stackAlign = 4;
#endif

	if(tblStack->rowCount() <= 0)
#ifdef _AMD64_
	{
		if(bIsWOW64)
			dwOffset = m_pCoreDebugger->wowProcessContext.Esp;
		else
			dwOffset = m_pCoreDebugger->ProcessContext.Rsp;
	}
#else
		dwOffset = m_pCoreDebugger->ProcessContext.Esp;
#endif
	else
		dwOffset = tblStack->item(0,0)->text().toULongLong(0,16);

	if(iValue < 5)
		LoadStackView(dwOffset,stackAlign);
	else
		LoadStackView(dwOffset + (stackAlign * 2),stackAlign);

	scrollStackView->setValue(5);
}

bool qtDLGStack::eventFilter(QObject *pObject, QEvent *event)
{	
	if(pObject == tblStack)
	{
		if(event->type() == QEvent::Wheel)
		{
			QWheelEvent *pWheel = (QWheelEvent*)event;
		
			OnStackScroll(pWheel->delta() * -1);
			return true;
		}
		else if(event->type() == QEvent::KeyPress)
		{
			QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if(keyEvent->key() == Qt::Key_Up)
            {
				return OnMoveUpOrDown(true);
            }
            else if(keyEvent->key() == Qt::Key_Down)
            {
				return OnMoveUpOrDown(false);
            }
		}
	}

	return false;
}

void qtDLGStack::OnContextMenu(QPoint qPoint)
{
	QMenu menu;

	m_selectedRow = tblStack->indexAt(qPoint).row();
	if(m_selectedRow < 0) return;

	menu.addAction(new QAction("Send to Disassembler",this));
	menu.addSeparator();
	QMenu *submenu = menu.addMenu("Copy to Clipboard");
	submenu->addAction(new QAction("Line",this));
	submenu->addAction(new QAction("Offset",this));
	submenu->addAction(new QAction("Data",this));
	submenu->addAction(new QAction("Comment",this));

	menu.addMenu(submenu);
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));

	menu.exec(QCursor::pos());
}

void qtDLGStack::MenuCallback(QAction* pAction)
{
	if(QString().compare(pAction->text(),"Send to Disassembler") == 0)
	{
		qtDLGNanomite::GetInstance()->DisAsGUI->OnDisplayDisassembly(tblStack->item(m_selectedRow,1)->text().toULongLong(0,16));
	}
	else if(QString().compare(pAction->text(),"Line") == 0)
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
	else if(QString().compare(pAction->text(),"Data") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblStack->item(m_selectedRow,1)->text());
	}
	else if(QString().compare(pAction->text(),"Comment") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblStack->item(m_selectedRow,2)->text());
	}
}

void qtDLGStack::resizeEvent(QResizeEvent *event)
{
	if(!m_pCoreDebugger->GetDebuggingState())
		return;

#ifdef _AMD64_
	BOOL bIsWOW64 = false;
	if(clsAPIImport::pIsWow64Process)
		clsAPIImport::pIsWow64Process(m_pCoreDebugger->GetCurrentProcessHandle(),&bIsWOW64);

	if(bIsWOW64)
	{
		LoadStackView(m_pCoreDebugger->wowProcessContext.Esp,4);
	}
	else
	{
		LoadStackView(m_pCoreDebugger->ProcessContext.Rsp,8);
	}
#else
	LoadStackView(m_pCoreDebugger->ProcessContext.Esp,4);
#endif
}

bool qtDLGStack::OnMoveUpOrDown(bool isUp)
{
	if(tblStack->selectedItems().count() <= 0) return false;

	QString selectedOffset = tblStack->selectedItems()[0]->text();

	if(isUp)
	{
		if(tblStack->item(0,0)->text().compare(selectedOffset) == 0)
		{
			OnStackScroll(4);
		}
		else
		{
			return false;
		}
	}
	else
	{
		if(tblStack->item(tblStack->rowCount() - 1, 0)->text().compare(selectedOffset) == 0)
		{
			OnStackScroll(6);
		}
		else
		{
			return false;
		}
	}

	return true;
}