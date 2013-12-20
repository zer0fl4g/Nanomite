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
#include "qtDLGDisassembler.h"
#include "qtDLGAssembler.h"
#include "qtDLGNanomite.h"
#include "qtDLGGoToDialog.h"
#include "qtDLGBookmark.h"

#include "clsAPIImport.h"
#include "clsHelperClass.h"
#include "clsSymbolAndSyntax.h"
#include "clsBreakpointManager.h"
#include "clsClipboardHelper.h"

qtDLGDisassembler::qtDLGDisassembler(QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);
	this->setLayout(horizontalLayout);

	dlgSourceViewer			= new qtDLGSourceViewer(this,Qt::Window);
	qtNanomiteDisAsColor	= qtDLGNanomite::GetInstance()->disasColor;
	coreDisAs				= qtDLGNanomite::GetInstance()->coreDisAs;
	coreDebugger			= qtDLGNanomite::GetInstance()->coreDebugger;
	PEManager				= qtDLGNanomite::GetInstance()->PEManager;

	connect(new QShortcut(QKeySequence("F2"), this), SIGNAL(activated()), this, SLOT(OnF2BreakPointPlace()));
	connect(new QShortcut(QKeySequence("F4"), this), SIGNAL(activated()), this, SLOT(OnF4ExecuteTo()));
	connect(new QShortcut(QKeySequence::InsertParagraphSeparator, this), SIGNAL(activated()), this, SLOT(OnDisAsReturnPressed()));
	connect(new QShortcut(QKeySequence("Backspace"), this), SIGNAL(activated()),this, SLOT(OnDisAsReturn()));
	connect(new QShortcut(QKeySequence("space"), this), SIGNAL(activated()), this, SLOT(OnEditInstruction()));
	connect(tblDisAs, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(OnCustomDisassemblerContextMenu(QPoint)));
	connect(scrollDisAs, SIGNAL(valueChanged(int)), this, SLOT(OnDisAsScroll(int)));
	
	// eventFilter for mouse scroll
	tblDisAs->installEventFilter(this);
	tblDisAs->viewport()->installEventFilter(this);
	
	// List DisAs
	tblDisAs->horizontalHeader()->resizeSection(0,135);
	tblDisAs->horizontalHeader()->resizeSection(1,250);
	tblDisAs->horizontalHeader()->resizeSection(2,310);
	tblDisAs->horizontalHeader()->resizeSection(3,310);
	tblDisAs->horizontalHeader()->setFixedHeight(21);

	m_maxRows = ((tblDisAs->verticalHeader()->height()) / 11) - 1;

	buildContextMenu();
}

qtDLGDisassembler::~qtDLGDisassembler()
{
	tblDisAs->setRowCount(0);
	m_offsetWalkHistory.clear();

	delete dlgSourceViewer;
}

void qtDLGDisassembler::OnDisAsScroll(int iValue)
{
	if(iValue == 5 || tblDisAs->rowCount() < 10) return;
	else if(iValue < 5)
		OnDisplayDisassembly(tblDisAs->item(0,0)->text().toULongLong(0,16));
	else
	{
		if(tblDisAs->rowCount() < m_maxRows)
		{
			scrollDisAs->setValue(5);
			return;
		}

		OnDisplayDisassembly(tblDisAs->item(10,0)->text().toULongLong(0,16));
	}

	scrollDisAs->setValue(5);
}

void qtDLGDisassembler::OnDisplayDisassembly(quint64 dwEIP)
{
	if(coreDisAs->SectionDisAs.count() > 0 && dwEIP != 0)
	{
		m_lastEIP = dwEIP;
		int lineCount = 0;

		QMap<quint64,DisAsDataRow>::const_iterator disassemblyDataCurrent = coreDisAs->SectionDisAs.constFind(dwEIP);
		QMap<quint64,DisAsDataRow>::const_iterator disassemblyDataEnd = coreDisAs->SectionDisAs.constEnd(); --disassemblyDataEnd;

		if(disassemblyDataCurrent == coreDisAs->SectionDisAs.constEnd())
		{
			coreDisAs->InsertNewDisassembly(coreDebugger->GetCurrentProcessHandle(),dwEIP);
			return;
		}
		else if(disassemblyDataCurrent == coreDisAs->SectionDisAs.constBegin())
		{
			if(coreDisAs->InsertNewDisassembly(coreDebugger->GetCurrentProcessHandle(),disassemblyDataCurrent.value().Offset))
				return;
		}
		else
		{
			for(int iBack = 0; iBack < 5; iBack++)
			{
				if(disassemblyDataCurrent == coreDisAs->SectionDisAs.constBegin())
					break;
				--disassemblyDataCurrent;
			}
		}

		if(m_maxRows == -1)
		{
			m_maxRows = (tblDisAs->verticalHeader()->height() / 11) - 1;
		}

		tblDisAs->setUpdatesEnabled(false);
		if((tblDisAs->rowCount() - 1) != m_maxRows)
		{
			tblDisAs->setRowCount(0);
			while(lineCount <= m_maxRows)
			{
				tblDisAs->insertRow(0);
				lineCount++;
			}
			lineCount = 0;
		}

		quint64 itemStyle;
		int processID = clsDebugger::GetCurrentPID();
		QString bookmarkComment;
		bool IsAlreadyEIPSet = false;

		while(lineCount <= m_maxRows)
		{
			itemStyle = disassemblyDataCurrent.value().itemStyle;

			tblDisAs->setItem(lineCount, 0,new QTableWidgetItem(QString("%1").arg(disassemblyDataCurrent.value().Offset,16,16,QChar('0'))));

			if(clsBreakpointManager::IsOffsetAnBP(disassemblyDataCurrent.value().Offset))
				tblDisAs->item(lineCount,0)->setForeground(QColor(qtNanomiteDisAsColor->colorBP));

			if(!IsAlreadyEIPSet && clsDebugger::IsOffsetEIP(disassemblyDataCurrent.value().Offset))
			{
				tblDisAs->item(lineCount,0)->setBackground(QColor("Magenta"));
				IsAlreadyEIPSet = true;
			}

			tblDisAs->setItem(lineCount, 1,new QTableWidgetItem(disassemblyDataCurrent.value().OpCodes));

			tblDisAs->setItem(lineCount, 2,new QTableWidgetItem(disassemblyDataCurrent.value().ASM));

			if(itemStyle & COLOR_CALLS)
				tblDisAs->item(lineCount,2)->setForeground(QColor(qtNanomiteDisAsColor->colorCall));
			else if(itemStyle & COLOR_JUMP)
				tblDisAs->item(lineCount,2)->setForeground(QColor(qtNanomiteDisAsColor->colorJump));
			else if(itemStyle & COLOR_MOVE)
				tblDisAs->item(lineCount,2)->setForeground(QColor(qtNanomiteDisAsColor->colorMove));
			else if(itemStyle & COLOR_STACK)
				tblDisAs->item(lineCount,2)->setForeground(QColor(qtNanomiteDisAsColor->colorStack));
			else if(itemStyle & COLOR_MATH)
				tblDisAs->item(lineCount,2)->setForeground(QColor(qtNanomiteDisAsColor->colorMath));

			
			bookmarkComment = qtDLGBookmark::BookmarkGetComment(processID, disassemblyDataCurrent.value().Offset);
			if(bookmarkComment.length() <= 0)
				tblDisAs->setItem(lineCount, 3, new QTableWidgetItem(disassemblyDataCurrent.value().Comment));
			else
				tblDisAs->setItem(lineCount, 3, new QTableWidgetItem(bookmarkComment));


			++lineCount;++disassemblyDataCurrent;
			if(disassemblyDataCurrent == disassemblyDataEnd)
			{
				if(tblDisAs->rowCount() < 5 && tblDisAs->rowCount() > 0)
					coreDisAs->InsertNewDisassembly(coreDebugger->GetCurrentProcessHandle(),tblDisAs->item(tblDisAs->rowCount() - 1,0)->text().toULongLong(0,16),true);
				else
					coreDisAs->InsertNewDisassembly(coreDebugger->GetCurrentProcessHandle(),tblDisAs->item(5,0)->text().toULongLong(0,16),true);
				
				tblDisAs->setUpdatesEnabled(true);
				return;
			}
		}

		// Update Window Title
		QString ModName,FuncName;
		clsHelperClass::LoadSymbolForAddr(FuncName,ModName,dwEIP,coreDebugger->GetCurrentProcessHandle());
		if(ModName.length() > 0 && FuncName.length() > 0)
			qtDLGNanomite::GetInstance()->setWindowTitle(QString("[Nanomite v 0.1 - PID: %1 - TID: %2]- %3.%4").arg(processID,6,16,QChar('0')).arg(clsDebugger::GetCurrentTID(),6,16,QChar('0')).arg(ModName).arg(FuncName));
		else if(ModName.length() > 0 && FuncName.length() <= 0)
			qtDLGNanomite::GetInstance()->setWindowTitle(QString("[Nanomite v 0.1 - PID: %1 - TID: %2]- %3.%4").arg(processID,6,16,QChar('0')).arg(clsDebugger::GetCurrentTID(),6,16,QChar('0')).arg(ModName).arg(dwEIP,16,16,QChar('0')));

		tblDisAs->setUpdatesEnabled(true);
	}
	else
	{
		coreDisAs->InsertNewDisassembly(coreDebugger->GetCurrentProcessHandle(),dwEIP);
	}
}

void qtDLGDisassembler::OnDisAsReturnPressed()
{
	QList<QTableWidgetItem *> currentSelectedItems = tblDisAs->selectedItems();
	if(coreDebugger->GetCurrentProcessHandle() == NULL || currentSelectedItems.count() <= 0) return;

	quint64 dwSelectedVA = NULL;
	QString tempSelectedString = currentSelectedItems.value(2)->text();

	if(tempSelectedString.contains("ptr"))
	{
		QStringList splittedSelectedString = tempSelectedString.split(" ");
		if(splittedSelectedString[1].compare("qword") == 0)
		{
			ReadProcessMemory(coreDebugger->GetCurrentProcessHandle(),
				(LPVOID)splittedSelectedString[3].replace("h","").replace("[","").replace("]","").toULongLong(0,16),
				(LPVOID)&dwSelectedVA, sizeof(DWORD64),NULL);
		}
		else if(splittedSelectedString[1].compare("dword") == 0)
		{
			ReadProcessMemory(coreDebugger->GetCurrentProcessHandle(),
				(LPVOID)splittedSelectedString[3].replace("h","").replace("[","").replace("]","").toULongLong(0,16),
				(LPVOID)&dwSelectedVA, sizeof(DWORD),NULL);
		}
	}
	else
		dwSelectedVA = tempSelectedString.split(" ")[1].replace("h","").toULongLong(0,16);

	if(dwSelectedVA != 0)
	{
		m_offsetWalkHistory.append(currentSelectedItems.value(0)->text().toULongLong(0,16));

		if(!coreDisAs->InsertNewDisassembly(coreDebugger->GetCurrentProcessHandle(),dwSelectedVA))
			OnDisplayDisassembly(dwSelectedVA);
	}
	return;
}

void qtDLGDisassembler::OnDisAsReturn()
{
	if(m_offsetWalkHistory.isEmpty()) return;

	quint64 lastOffset = m_offsetWalkHistory.takeLast();
	if(!coreDisAs->InsertNewDisassembly(coreDebugger->GetCurrentProcessHandle(),lastOffset))
		OnDisplayDisassembly(lastOffset);
}

bool qtDLGDisassembler::eventFilter(QObject *pObject, QEvent *event)
{	
	if(pObject == tblDisAs)
	{
		if(event->type() == QEvent::Wheel)
		{
			QWheelEvent *pWheel = (QWheelEvent*)event;
		
			OnDisAsScroll(pWheel->delta() * -1);
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
			else if(keyEvent->key() == Qt::Key_PageUp)
			{
				return OnMoveUpOrDown(true,true);
			}
			else if(keyEvent->key() == Qt::Key_PageDown)
			{
				return OnMoveUpOrDown(false,true);
			}
		}
	}

	return false;
}

void qtDLGDisassembler::buildContextMenu()
{
	QMenu *submenu = m_contextMenu.addMenu("Copy to Clipboard");
	submenu->addAction(new QAction("Line", this));
	submenu->addAction(new QAction("Offset", this));
	submenu->addAction(new QAction("OpCodes", this));
	submenu->addAction(new QAction("Mnemonic", this));
	submenu->addAction(new QAction("Comment", this));
	m_contextMenu.addMenu(submenu);
	//menu.addMenu(clsClipboardHelper::getTableClipboardMenu(tblDisAs, &menu));
	m_contextMenu.addSeparator();
	m_contextMenu.addAction(new QAction("Edit Instruction", this));	
	m_contextMenu.addAction(new QAction("Goto Offset / Function", this));
	m_contextMenu.addAction(new QAction("Set R/EIP to this", this));
	m_contextMenu.addAction(new QAction("Show Source", this));	
	m_contextMenu.addAction(new QAction("Toggle SW Breakpoint", this));
	m_contextMenu.addSeparator();
	m_contextMenu.addAction(new QAction("Set Comment", this));
	m_contextMenu.addAction(new QAction("Set Bookmark", this));
	m_contextMenu.addSeparator();
	m_contextMenu.addAction(new QAction("Execute to this", this));
	m_contextMenu.addAction(new QAction("Trace to this", this));
	
	connect(&m_contextMenu, SIGNAL(triggered(QAction*)), this, SLOT(CustomDisassemblerMenuCallback(QAction*)));
}

void qtDLGDisassembler::OnCustomDisassemblerContextMenu(QPoint qPoint)
{
	m_iSelectedRow = tblDisAs->indexAt(qPoint).row();
	if(m_iSelectedRow < 0) return;

	m_contextMenu.exec(QCursor::pos());
}

void qtDLGDisassembler::CustomDisassemblerMenuCallback(QAction* pAction)
{
	if(!coreDebugger->GetDebuggingState()) return;
	
	if(pAction->text().compare("Set R/EIP to this") == 0)
	{
#ifdef _AMD64_
		BOOL bIsWOW64 = false;
		if(clsAPIImport::pIsWow64Process)
			clsAPIImport::pIsWow64Process(coreDebugger->GetCurrentProcessHandle(),&bIsWOW64);

		if(bIsWOW64)
			coreDebugger->wowProcessContext.Eip = tblDisAs->item(m_iSelectedRow,0)->text().toULongLong(0,16);
		else
			coreDebugger->ProcessContext.Rip = tblDisAs->item(m_iSelectedRow,0)->text().toULongLong(0,16);
#else
		coreDebugger->ProcessContext.Eip = tblDisAs->item(m_iSelectedRow,0)->text().toULongLong(0,16);
#endif
		emit OnDebuggerBreak();
	}
	else if(pAction->text().compare("Set Bookmark") == 0)
	{
		qtDLGBookmark::BookmarkAdd(coreDebugger->GetCurrentPID(), tblDisAs->item(m_iSelectedRow, 0)->text().toULongLong(0,16));
	}
	else if(pAction->text().compare("Set Comment") == 0)
	{
		QString commentString = QInputDialog::getText(this, "Nanomite", "Comment:");

		if(commentString.length() > 0)
			qtDLGBookmark::BookmarkAdd(coreDebugger->GetCurrentPID(), tblDisAs->item(m_iSelectedRow, 0)->text().toULongLong(0,16), commentString);
	}
	else if(pAction->text().compare("Trace to this") == 0)
	{
		int processID = coreDebugger->GetCurrentPID();
		qtDLGNanomite *pMainWindow = qtDLGNanomite::GetInstance();

		pMainWindow->coreBPManager->BreakpointAdd(SOFTWARE_BP, BP_TRACETO, processID, tblDisAs->item(m_iSelectedRow,0)->text().toULongLong(0,16), 1, BP_TRACETO, NULL);
		qtDLGTrace::clearTraceData();
		pMainWindow->actionDebug_Trace_Stop->setEnabled(true);
		pMainWindow->actionDebug_Trace_Start->setEnabled(false);
		coreDebugger->SetTraceFlagForPID(processID,true);
	}
	else if(pAction->text().compare("Edit Instruction") == 0)
	{
		OnEditInstruction();
	}
	else if(pAction->text().compare("Execute to this") == 0)
	{
		OnF4ExecuteTo();
	}
	else if(pAction->text().compare("Goto Offset / Function") == 0)
	{
		QString searchedOffset;

		qtDLGGoToDialog newGoToDLG(&searchedOffset, m_searchedOffsetList, this, Qt::Window);
		newGoToDLG.exec();

		if(!searchedOffset.isEmpty())
		{
			if(searchedOffset.contains("::"))
			{
				QStringList SplitAPIList = searchedOffset.split("::");

				if(SplitAPIList.count() >= 2)
				{
					quint64 dwOffset = clsHelperClass::CalcOffsetForModule((PTCHAR)SplitAPIList[0].toLower().toStdWString().c_str(),NULL,clsDebugger::GetCurrentPID());
					dwOffset = clsHelperClass::RemoteGetProcAddr(SplitAPIList[1],dwOffset,clsDebugger::GetCurrentPID());

					if(dwOffset > 0)
						searchedOffset = QString("%1").arg(dwOffset,16,16,QChar('0'));
					else
						return;
				}
			}
			else
			{
				if(!m_searchedOffsetList.contains(searchedOffset))
					m_searchedOffsetList.append(searchedOffset);
			}

			m_offsetWalkHistory.append(tblDisAs->item(m_iSelectedRow, 0)->text().toULongLong(0,16));
			if(!coreDisAs->InsertNewDisassembly(coreDebugger->GetCurrentProcessHandle(),searchedOffset.toULongLong(0,16)))
				OnDisplayDisassembly(searchedOffset.toULongLong(0,16));	
		}
	}
	else if(pAction->text().compare("Show Source") == 0)
	{
		QString fileName;
		int lineNumber = NULL;
		clsHelperClass::LoadSourceForAddr(fileName, lineNumber, tblDisAs->item(m_iSelectedRow, 0)->text().toULongLong(0,16), coreDebugger->GetCurrentProcessHandle());

		dlgSourceViewer->OnDisplaySource(fileName, lineNumber);
		if(dlgSourceViewer->IsSourceAvailable)
			dlgSourceViewer->show();
		else
			QMessageBox::information(this,"Nanomite","Sorry, there is no source available!",QMessageBox::Ok,QMessageBox::Ok);
	}
	else if(pAction->text().compare("Line") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(clsClipboardHelper::getTableToClipboard(tblDisAs, -1));
	}
	else if(pAction->text().compare("Offset") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(clsClipboardHelper::getTableToClipboard(tblDisAs, 0));

	}
	else if(pAction->text().compare("OpCodes") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(clsClipboardHelper::getTableToClipboard(tblDisAs, 1));

	}
	else if(pAction->text().compare("Mnemonic") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(clsClipboardHelper::getTableToClipboard(tblDisAs, 2));

	}
	else if(pAction->text().compare("Comment") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(clsClipboardHelper::getTableToClipboard(tblDisAs, 3));

	}
	else if(pAction->text().compare("Toggle SW Breakpoint") == 0)
	{
		OnF2BreakPointPlace();
	}
}

void qtDLGDisassembler::OnF2BreakPointPlace()
{
	QList<QTableWidgetItem *> currentSelectedItems = tblDisAs->selectedItems();
	if(currentSelectedItems.count() <= 0) return;

	quint64 dwSelectedVA = currentSelectedItems.value(0)->text().toULongLong(0,16);
	if(clsBreakpointManager::BreakpointInsert(SOFTWARE_BP, BP_EXEC, -1, dwSelectedVA, 1, BP_KEEP))
		currentSelectedItems.value(0)->setForeground(QColor(qtNanomiteDisAsColor->colorBP));
	else
	{// exists
		clsBreakpointManager::BreakpointDelete(dwSelectedVA,SOFTWARE_BP);
		currentSelectedItems.value(0)->setForeground(QColor("Black"));
	}	
	return;
}

void qtDLGDisassembler::OnF4ExecuteTo()
{
	QList<QTableWidgetItem *> currentSelectedItems = tblDisAs->selectedItems();
	if(currentSelectedItems.count() <= 0) return;

	quint64 dwSelectedVA = currentSelectedItems.value(0)->text().toULongLong(0,16);
	if(dwSelectedVA > 0 && clsBreakpointManager::BreakpointInsert(SOFTWARE_BP, BP_EXEC, -1, dwSelectedVA, 1, BP_STEPOVER))
	{
		qtDLGTrace::clearTraceData();
		coreDebugger->ResumeDebugging();
		qtDLGNanomite::GetInstance()->UpdateStateBar(STATE_RUN);
	}
}

void qtDLGDisassembler::resizeEvent(QResizeEvent *event)
{
	m_maxRows = (tblDisAs->verticalHeader()->height() / 11) - 1;

	if(coreDebugger->GetDebuggingState())
		OnDisplayDisassembly(m_lastEIP);
}

bool qtDLGDisassembler::OnMoveUpOrDown(bool isUp, bool isPage)
{
	if(tblDisAs->selectedItems().count() <= 0) return false;

	QString selectedOffset = tblDisAs->selectedItems()[0]->text();

	if(isUp)
	{
		if(tblDisAs->item(0,0)->text().compare(selectedOffset) == 0)
		{
			if(isPage)
			{
				OnDisplayDisassembly(tblDisAs->item(0,0)->text().toULongLong(0,16));
			}
			else
			{
				OnDisplayDisassembly(tblDisAs->item(4,0)->text().toULongLong(0,16));
			}
		}
		else
		{
			return false;
		}
	}
	else
	{
		if(tblDisAs->item(m_maxRows, 0)->text().compare(selectedOffset) == 0)
		{
			if(isPage)
			{
				OnDisplayDisassembly(tblDisAs->item(10,0)->text().toULongLong(0,16));
			}
			else
			{
				OnDisplayDisassembly(tblDisAs->item(6,0)->text().toULongLong(0,16));
			}
		}
		else
		{
			return false;
		}
	}

	return true;
}

void qtDLGDisassembler::OnEditInstruction()
{
	if(tblDisAs->selectedItems().count() <= 0) return;

	QTableWidgetItem *selectedItem = tblDisAs->selectedItems()[0];

	qtDLGAssembler *dlgAssembler = new qtDLGAssembler(	this, 
														Qt::Window,
														coreDebugger->GetCurrentProcessHandle(),
														tblDisAs->item(selectedItem->row(),0)->text().toULongLong(0,16),
														tblDisAs->item(selectedItem->row(),2)->text(),
														coreDisAs,PEManager->is64BitFile("",coreDebugger->GetCurrentPID()));

	dlgAssembler->show();
}