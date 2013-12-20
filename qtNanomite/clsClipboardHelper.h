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
#ifndef clsClipboardHelper_H
#define clsClipboardHelper_H

#include <QString>
#include <QTableWidget>
#include <QClipboard>
#include <QMenu>

class clsClipboardHelper
{
public:
	static QString getTableToClipboard(QTableWidget *currentTable, int column = -1);
	//static QMenu* getTableClipboardMenu(QTableWidget *currentTable, QMenu *currentMenu);
};

#endif