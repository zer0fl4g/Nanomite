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
#include "clsClipboardHelper.h"
#include "clsMemManager.h"

QString clsClipboardHelper::getTableToClipboard(QTableWidget *currentTable, int column)
{
	int selectedRowCount	= currentTable->selectedRanges().front().rowCount(),
		firstSelectedRow	= currentTable->selectedRanges().front().topRow(),
		dumpColumnIndex		= column;

	if(selectedRowCount < 1 || firstSelectedRow < 0 || dumpColumnIndex < -1 || dumpColumnIndex >= currentTable->columnCount()) return "";

	QString headerData		= "",
			selectedData	= "";
	
	if(column == -1)
	{
		dumpColumnIndex = currentTable->columnCount();
		column = 0;
	}
	else
	{
		dumpColumnIndex += 1;
	}

	for(int i = column; i < dumpColumnIndex; i++)
	{
		if((i + 1) != dumpColumnIndex)
		{
			headerData.append(QString("%1:").arg(currentTable->horizontalHeaderItem(i)->text()));
		}
		else
		{
			headerData.append(currentTable->horizontalHeaderItem(i)->text());
		}
	}
	
	for(int i = firstSelectedRow; i < (firstSelectedRow + selectedRowCount); i++)
	{
		selectedData.append("\n");

		for(int a = column; a < dumpColumnIndex; a++)
		{
			selectedData.append(currentTable->item(i, a)->text());
			
			if((a + 1) != dumpColumnIndex)
				selectedData.append(":");
		}
	}

	return headerData.append(selectedData);
}

//QMenu* clsClipboardHelper::getTableClipboardMenu(QTableWidget *currentTable, QMenu *currentMenu)
//{	
//	QMenu *clipboardMenu = currentMenu->addMenu("Copy to Clipboard");
//
//	for(int i = 0; i < currentTable->columnCount(); i++)
//	{
//		clipboardMenu->addAction(new QAction(currentTable->horizontalHeaderItem(i)->text(), currentTable));
//	}	
//
//	return clipboardMenu;
//}