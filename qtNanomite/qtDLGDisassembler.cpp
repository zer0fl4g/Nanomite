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

#include "clsAPIImport.h"
#include "clsHelperClass.h"
#include "clsSymbolAndSyntax.h"

#include <string>

using namespace std;

qtDLGDisassembler::qtDLGDisassembler(QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);
	this->setLayout(horizontalLayout);

	dlgSourceViewer = new qtDLGSourceViewer(this,Qt::Window);
	qtNanomiteDisAsColor = qtDLGNanomite::GetInstance()->qtNanomiteDisAsColor;
	coreDisAs = qtDLGNanomite::GetInstance()->coreDisAs;
	coreDebugger = qtDLGNanomite::GetInstance()->coreDebugger;
	PEManager = qtDLGNanomite::GetInstance()->PEManager;

	connect(new QShortcut(QKeySequence("F2"),this),SIGNAL(activated()),this,SLOT(OnF2BreakPointPlace()));
	connect(new QShortcut(QKeySequence::InsertParagraphSeparator,this),SIGNAL(activated()),this,SLOT(OnDisAsReturnPressed()));
	connect(new QShortcut(QKeySequence("Backspace"),this),SIGNAL(activated()),this,SLOT(OnDisAsReturn()));
	connect(tblDisAs,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnCustomDisassemblerContextMenu(QPoint)));
	connect(scrollDisAs,SIGNAL(valueChanged(int)),this,SLOT(OnDisAsScroll(int)));
	
	// eventFilter for mouse scroll
	tblDisAs->installEventFilter(this);
    tblDisAs->viewport()->installEventFilter(this);
	
	// List DisAs
	tblDisAs->horizontalHeader()->resizeSection(0,135);
	tblDisAs->horizontalHeader()->resizeSection(1,250);
	tblDisAs->horizontalHeader()->resizeSection(2,310);
	tblDisAs->horizontalHeader()->resizeSection(3,310);
	tblDisAs->horizontalHeader()->setFixedHeight(21);
}

qtDLGDisassembler::~qtDLGDisassembler()
{
	tblDisAs->setRowCount(0);
	_OffsetWalkHistory.clear();

	delete dlgSourceViewer;
}

void qtDLGDisassembler::OnDisAsScroll(int iValue)
{
	if(iValue == 5 || tblDisAs->rowCount() <= 10) return;
	else if(iValue < 5)
		OnDisplayDisassembly(tblDisAs->item(0,0)->text().toULongLong(0,16));
	else
	{
		if(tblDisAs->rowCount() <= 25)
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
		bool IsAlreadyEIPSet = false;
		int iLines = 0;

		QMap<QString,DisAsDataRow>::const_iterator i = coreDisAs->SectionDisAs.constFind(QString("%1").arg(dwEIP,16,16,QChar('0')).toUpper());
		QMap<QString,DisAsDataRow>::const_iterator iEnd = coreDisAs->SectionDisAs.constEnd();--iEnd;

		if((QMapData::Node *)i == (QMapData::Node *)coreDisAs->SectionDisAs.constEnd())
		{
			coreDisAs->InsertNewDisassembly(coreDebugger->GetCurrentProcessHandle(),dwEIP);
			return;
		}

		if((QMapData::Node *)i != (QMapData::Node *)coreDisAs->SectionDisAs.constBegin())
		{
			for(int iBack = 0; iBack <= 5; iBack++)
			{
				if(!i.value().Offset.isEmpty() && i.value().Offset.compare(coreDisAs->SectionDisAs.begin().value().Offset) == 0)
				{
					coreDisAs->InsertNewDisassembly(coreDebugger->GetCurrentProcessHandle(),i.value().Offset.toULongLong(0,16));
					return;
				}			
				--i;
			}
		}

		tblDisAs->setRowCount(0);

		quint64 itemStyle;
		while(iLines <= ((tblDisAs->verticalHeader()->height()) / 11) - 1)
		{
			itemStyle = i.value().itemStyle;
			tblDisAs->insertRow(tblDisAs->rowCount());

			tblDisAs->setItem(tblDisAs->rowCount() - 1, 0,new QTableWidgetItem(i.value().Offset));
			if(clsDebugger::IsOffsetAnBP(i.value().Offset.toULongLong(0,16)))
				tblDisAs->item(tblDisAs->rowCount() - 1,0)->setForeground(QColor(qtNanomiteDisAsColor->colorBP));

			if(!IsAlreadyEIPSet && clsDebugger::IsOffsetEIP(i.value().Offset.toULongLong(0,16)))
			{
				tblDisAs->item(tblDisAs->rowCount() - 1,0)->setBackground(QColor("Magenta"));
				IsAlreadyEIPSet = true;
			}

			tblDisAs->setItem(tblDisAs->rowCount() - 1, 1,new QTableWidgetItem(i.value().OpCodes));

			tblDisAs->setItem(tblDisAs->rowCount() - 1, 2,new QTableWidgetItem(i.value().ASM));
			if(itemStyle & COLOR_CALLS)
				tblDisAs->item(tblDisAs->rowCount() - 1,2)->setForeground(QColor(qtNanomiteDisAsColor->colorCall));
			else if(itemStyle & COLOR_JUMP)
				tblDisAs->item(tblDisAs->rowCount() - 1,2)->setForeground(QColor(qtNanomiteDisAsColor->colorJump));
			else if(itemStyle & COLOR_MOVE)
				tblDisAs->item(tblDisAs->rowCount() - 1,2)->setForeground(QColor(qtNanomiteDisAsColor->colorMove));
			else if(itemStyle & COLOR_STACK)
				tblDisAs->item(tblDisAs->rowCount() - 1,2)->setForeground(QColor(qtNanomiteDisAsColor->colorStack));
			else if(itemStyle & COLOR_MATH)
				tblDisAs->item(tblDisAs->rowCount() - 1,2)->setForeground(QColor(qtNanomiteDisAsColor->colorMath));

			tblDisAs->setItem(tblDisAs->rowCount() - 1, 3,new QTableWidgetItem(i.value().Comment));

			if(!i.value().Offset.isEmpty() && i.value().Offset.compare(iEnd.value().Offset) == 0)
			{
				coreDisAs->InsertNewDisassembly(coreDebugger->GetCurrentProcessHandle(),tblDisAs->item(4,0)->text().toULongLong(0,16),true);
				return;
			}

			++iLines;++i;
		}

		// Update Window Title
		wstring ModName,FuncName;
		clsHelperClass::LoadSymbolForAddr(FuncName,ModName,dwEIP,coreDebugger->GetCurrentProcessHandle());
		qtDLGNanomite::GetInstance()->setWindowTitle(QString("[Nanomite v 0.1 - PID: %1 - TID: %2]- %3.%4").arg(coreDebugger->GetCurrentPID(),6,16,QChar('0')).arg(coreDebugger->GetCurrentTID(),6,16,QChar('0')).arg(QString().fromStdWString(ModName)).arg(QString().fromStdWString(FuncName)));
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

	if(tempSelectedString.contains("ptr") || tempSelectedString.contains("ptr"))
		dwSelectedVA = tempSelectedString.split(" ")[3].replace("h","").replace("[","").replace("]","").toULongLong(0,16);
	else
		dwSelectedVA = tempSelectedString.split(" ")[1].replace("h","").toULongLong(0,16);

	if(dwSelectedVA != 0)
	{
		_OffsetWalkHistory.append(currentSelectedItems.value(0)->text().toULongLong(0,16));
		coreDisAs->InsertNewDisassembly(coreDebugger->GetCurrentProcessHandle(),dwSelectedVA);
	}
	return;
}

void qtDLGDisassembler::OnDisAsReturn()
{
	if(_OffsetWalkHistory.isEmpty()) return;
	coreDisAs->InsertNewDisassembly(coreDebugger->GetCurrentProcessHandle(),_OffsetWalkHistory.takeLast());
}

bool qtDLGDisassembler::eventFilter(QObject *pObject, QEvent *event)
{	
	if(event->type() == QEvent::Wheel && pObject == tblDisAs)
	{
		QWheelEvent *pWheel = (QWheelEvent*)event;
		
		OnDisAsScroll(pWheel->delta() * -1);
			return true;
	}
	return false;
}

void qtDLGDisassembler::OnCustomDisassemblerContextMenu(QPoint qPoint)
{
	QMenu menu;

	_iSelectedRow = tblDisAs->indexAt(qPoint).row();
	if(_iSelectedRow < 0) return;


	menu.addAction(new QAction("Goto Offset",this));
	menu.addAction(new QAction("Edit Instruction",this));
	menu.addAction(new QAction("Show Source",this));
	menu.addAction(new QAction("Set R/EIP to this",this));
	QMenu *submenu = menu.addMenu("Copy to Clipboard");
	submenu->addAction(new QAction("Line",this));
	submenu->addAction(new QAction("Offset",this));
	submenu->addAction(new QAction("OpCodes",this));
	submenu->addAction(new QAction("Mnemonic",this));
	submenu->addAction(new QAction("Comment",this));

	menu.addMenu(submenu);
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(CustomDisassemblerMenuCallback(QAction*)));

	menu.exec(QCursor::pos());
}

void qtDLGDisassembler::CustomDisassemblerMenuCallback(QAction* pAction)
{
	if(!coreDebugger->GetDebuggingState()) return;

	if(QString().compare(pAction->text(),"Set R/EIP to this") == 0)
	{
#ifdef _AMD64_
		BOOL bIsWOW64 = false;
		if(clsAPIImport::pIsWow64Process)
			clsAPIImport::pIsWow64Process(coreDebugger->GetCurrentProcessHandle(),&bIsWOW64);

		if(bIsWOW64)
			coreDebugger->wowProcessContext.Eip = tblDisAs->item(_iSelectedRow,0)->text().toULongLong(0,16);
		else
			coreDebugger->ProcessContext.Rip = tblDisAs->item(_iSelectedRow,0)->text().toULongLong(0,16);
#else
		coreDebugger->ProcessContext.Eip = tblDisAs->item(_iSelectedRow,0)->text().toULongLong(0,16);
#endif
		emit OnDebuggerBreak();
	}
	else if(QString().compare(pAction->text(),"Edit Instruction") == 0)
	{
		qtDLGAssembler *dlgAssembler = new qtDLGAssembler(this,Qt::Window,coreDebugger->GetCurrentProcessHandle(),tblDisAs->item(_iSelectedRow,0)->text().toULongLong(0,16),coreDisAs,PEManager->is64BitFile(L"\\\\",coreDebugger->GetCurrentPID()));
		connect(dlgAssembler,SIGNAL(OnReloadDebugger()),qtDLGNanomite::GetInstance(),SLOT(OnDebuggerBreak()));
		dlgAssembler->show();
	}
	else if(QString().compare(pAction->text(),"Goto Offset") == 0)
	{
		bool bOk = false;
		QString strNewOffset = QInputDialog::getText(this,"Please give a Offset:","VA:",QLineEdit::Normal,NULL,&bOk);

		if(bOk && !strNewOffset.isEmpty())
			if(!coreDisAs->InsertNewDisassembly(coreDebugger->GetCurrentProcessHandle(),strNewOffset.toULongLong(0,16)))
				OnDisplayDisassembly(strNewOffset.toULongLong(0,16));		
	}
	else if(QString().compare(pAction->text(),"Show Source") == 0)
	{
		if(dlgSourceViewer->IsSourceAvailable)
			dlgSourceViewer->show();
		else
			MessageBoxW(NULL,L"Sorry, there is no source available!",L"Nanomite",MB_OK);
	}
	else if(QString().compare(pAction->text(),"Line") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(QString("%1:%2:%3:%4")
			.arg(tblDisAs->item(_iSelectedRow,0)->text())
			.arg(tblDisAs->item(_iSelectedRow,1)->text())
			.arg(tblDisAs->item(_iSelectedRow,2)->text())
			.arg(tblDisAs->item(_iSelectedRow,3)->text()));
	}
	else if(QString().compare(pAction->text(),"Offset") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblDisAs->item(_iSelectedRow,0)->text());
	}
	else if(QString().compare(pAction->text(),"OpCodes") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblDisAs->item(_iSelectedRow,1)->text());
	}
	else if(QString().compare(pAction->text(),"Mnemonic") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblDisAs->item(_iSelectedRow,2)->text());
	}
	else if(QString().compare(pAction->text(),"Comment") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblDisAs->item(_iSelectedRow,3)->text());
	}
}

void qtDLGDisassembler::OnF2BreakPointPlace()
{
	QList<QTableWidgetItem *> currentSelectedItems = tblDisAs->selectedItems();
	if(currentSelectedItems.count() <= 0) return;

	quint64 dwSelectedVA = currentSelectedItems.value(0)->text().toULongLong(0,16);
	if(coreDebugger->AddBreakpointToList(NULL,DR_EXECUTE,-1,dwSelectedVA,NULL,true))
		currentSelectedItems.value(0)->setForeground(QColor(qtNanomiteDisAsColor->colorBP));
	else
	{// exists
		coreDebugger->RemoveBPFromList(dwSelectedVA,NULL);
		currentSelectedItems.value(0)->setForeground(QColor("Black"));
	}	
	return;
}