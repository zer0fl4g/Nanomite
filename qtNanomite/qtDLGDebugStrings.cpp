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
#include "qtDLGDebugStrings.h"

#include "clsMemManager.h"

#include <QClipboard>
#include <QShortcut>
#include <QMenu>

qtDLGDebugStrings::qtDLGDebugStrings(QWidget *parent, Qt::WFlags flags)
	: QWidget(parent, flags)
{
	setupUi(this);
	
	// List Debug Strings
	tblDebugStrings->horizontalHeader()->resizeSection(0,75);
	tblDebugStrings->horizontalHeader()->setFixedHeight(21);

	setLayout(verticalLayout);

	connect(tblDebugStrings,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnCustomContextMenuRequested(QPoint)));
	connect(new QShortcut(Qt::Key_Escape,this),SIGNAL(activated()),this,SLOT(close()));
}

qtDLGDebugStrings::~qtDLGDebugStrings()
{

}

void qtDLGDebugStrings::OnCustomContextMenuRequested(QPoint qPoint)
{
	QMenu menu;

	m_selectedRow = tblDebugStrings->indexAt(qPoint).row();
	if(m_selectedRow < 0) return;

	QMenu *submenu = menu.addMenu("Copy to Clipboard");
	submenu->addAction(new QAction("Line",this));
	submenu->addAction(new QAction("Debug String",this));

	menu.addMenu(submenu);
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));

	menu.exec(QCursor::pos());
}

void qtDLGDebugStrings::MenuCallback(QAction* pAction)
{
	if(QString().compare(pAction->text(),"Line") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(QString("%1:%2")
			.arg(tblDebugStrings->item(m_selectedRow,0)->text())
			.arg(tblDebugStrings->item(m_selectedRow,1)->text()));
	}
	else if(QString().compare(pAction->text(),"Debug String") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblDebugStrings->item(m_selectedRow,1)->text());
	}
}

void qtDLGDebugStrings::OnDbgString(QString debugString, DWORD processID)
{
	tblDebugStrings->insertRow(tblDebugStrings->rowCount());

	tblDebugStrings->setItem(tblDebugStrings->rowCount() - 1,0,
		new QTableWidgetItem(QString().sprintf("%08X",processID)));

	tblDebugStrings->setItem(tblDebugStrings->rowCount() - 1,1,
		new QTableWidgetItem(debugString));

	tblDebugStrings->scrollToBottom();
}