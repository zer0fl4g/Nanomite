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

#include "clsHelperClass.h"
#include "clsMemManager.h"

#include <fstream>
#include <sstream>

#include <QMenu>
#include <QClipboard>

using namespace std;

qtDLGStringView::qtDLGStringView(QWidget *parent, Qt::WFlags flags,qint32 iPID)
	: QWidget(parent, flags)
{
	setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose,true);
	this->setLayout(verticalLayout);
	
	_iPID = iPID;

	// Init List
	tblStringView->horizontalHeader()->resizeSection(0,75);
	tblStringView->horizontalHeader()->resizeSection(1,135);

	// Display
	myMainWindow = qtDLGNanomite::GetInstance();

	_iForEntry = 0;
	_iForEnd = myMainWindow->coreDebugger->PIDs.size();

	for(size_t i = 0; i < myMainWindow->coreDebugger->PIDs.size(); i++)
	{
		if(myMainWindow->coreDebugger->PIDs[i].dwPID == _iPID)
			_iForEntry = i; _iForEnd = i + 1;
	}
	
	connect(tblStringView,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnCustomContextMenuRequested(QPoint)));
	connect(new QShortcut(QKeySequence("F5"),this),SIGNAL(activated()),this,SLOT(DisplayStrings()));

	DisplayStrings();
}

qtDLGStringView::~qtDLGStringView()
{

}

void qtDLGStringView::DisplayStrings()
{
	tblStringView->setRowCount(0);

	GetAsciiString();
}

void qtDLGStringView::MenuCallback(QAction* pAction)
{
	if(QString().compare(pAction->text(),"Line") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(QString("%1:%2:%3")
			.arg(tblStringView->item(_iSelectedRow,0)->text())
			.arg(tblStringView->item(_iSelectedRow,1)->text())
			.arg(tblStringView->item(_iSelectedRow,2)->text()));
	}
	else if(QString().compare(pAction->text(),"Offset") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblStringView->item(_iSelectedRow,1)->text());
	}
	else if(QString().compare(pAction->text(),"String") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblStringView->item(_iSelectedRow,2)->text());
	}
}

void qtDLGStringView::OnCustomContextMenuRequested(QPoint qPoint)
{
	QMenu menu;

	_iSelectedRow = tblStringView->indexAt(qPoint).row();
	if(_iSelectedRow < 0) return;

	QMenu *submenu = menu.addMenu("Copy to Clipboard");
	submenu->addAction(new QAction("Line",this));
	submenu->addAction(new QAction("Offset",this));
	submenu->addAction(new QAction("String",this));

	menu.addMenu(submenu);
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));

	menu.exec(QCursor::pos());
}

void qtDLGStringView::GetAsciiString()
{
	for(size_t i = _iForEntry; i < _iForEnd;i++)
	{
		bool	bNotEndOfFile = true,
				isNullPadding = false;
		ifstream inputFile;
	
		inputFile.open(myMainWindow->coreDebugger->PIDs[i].sFileName,ifstream::binary);

		if(!inputFile.is_open())
		{
			MessageBox(NULL,myMainWindow->coreDebugger->PIDs[i].sFileName,L"Error opening File!",MB_OKCANCEL);
			bNotEndOfFile = false;
			close();
		}

		while(bNotEndOfFile && inputFile.good())
		{
			QString asciiChar;
			CHAR sT = '\0';
			
			inputFile.get(sT);
			while(inputFile.good())
			{
				if(((int)sT >= 0x41 && (int)sT <= 0x5a)		||
					((int)sT >= 0x61 && (int)sT <= 0x7a)	||
					((int)sT >= 0x30 && (int)sT <= 0x39)	|| 
					((int)sT == 0x20)						||
					((int)sT == 0xA))
					asciiChar.append(sT);
				else
				{
					break;
				}

				inputFile.get(sT);
			}

			if((int)sT == 0 && asciiChar.length() > 3)
			{
				PrintStringToList(myMainWindow->coreDebugger->PIDs[i].dwPID,asciiChar,inputFile.tellg());
			}
		}

		inputFile.close();
	}
}

void qtDLGStringView::GetUnicodeString()
{
	for(size_t i = _iForEntry; i < _iForEnd;i++)
	{
		bool	bNotEndOfFile = true,
				isNullPadding = false;
		wifstream inputFile;
	
		inputFile.open(myMainWindow->coreDebugger->PIDs[i].sFileName,ifstream::binary);

		if(!inputFile.is_open())
		{
			MessageBox(NULL,myMainWindow->coreDebugger->PIDs[i].sFileName,L"Error opening File!",MB_OKCANCEL);
			bNotEndOfFile = false;
			close();
		}

		while(bNotEndOfFile && inputFile.good())
		{
			QString unicodeChar;
			TCHAR sT = '\0';
			
			inputFile.get(sT);
			while(inputFile.good())
			{
				if(((int)sT >= 0x41 && (int)sT <= 0x5a)		||
					((int)sT >= 0x61 && (int)sT <= 0x7a)	||
					((int)sT >= 0x30 && (int)sT <= 0x39)	|| 
					((int)sT == 0x20)						||
					((int)sT == 0xA))
					unicodeChar.append(sT);
				else
				{
					break;
				}

				inputFile.get(sT);
			}

			if((int)sT == 0 && unicodeChar.length() > 3)
			{
				PrintStringToList(myMainWindow->coreDebugger->PIDs[i].dwPID,unicodeChar,inputFile.tellg());
			}
		}

		inputFile.close();
	}
}

void qtDLGStringView::PrintStringToList(int PID, QString StringToPrint, int StringOffset)
{
	tblStringView->insertRow(tblStringView->rowCount());
						
	// PID
	tblStringView->setItem(tblStringView->rowCount() - 1,0,
		new QTableWidgetItem(QString().sprintf("%08X",PID)));

	// Offset
	if(StringOffset > 0)
		tblStringView->setItem(tblStringView->rowCount() - 1,1,
			new QTableWidgetItem(QString("%1").arg(StringOffset,8,16,QChar('0'))));
	else 
		tblStringView->setItem(tblStringView->rowCount() - 1,1,
			new QTableWidgetItem(""));

	// String
	tblStringView->setItem(tblStringView->rowCount() - 1,2,
		new QTableWidgetItem(StringToPrint));
}