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
#ifndef QTDLGTIBVIEW_H
#define QTDLGTIBVIEW_H

#include "ui_qtDLGTIBView.h"

#include <Windows.h>

class qtDLGTIBView : public QWidget, public Ui_qtDLGTIBViewClass
{
	Q_OBJECT

public:
	qtDLGTIBView(HANDLE hProc, HANDLE hThread, QWidget *parent = 0, Qt::WFlags flags = 0);
	~qtDLGTIBView();

private:
	int m_SelectedRow;

	void InsertDataIntoTable(QTreeWidgetItem *pTopElement, QString valueName, DWORD64 value);
	void ShowTIBForThread(HANDLE hProc, HANDLE hThread);

//private slots:
//	void MenuCallback(QAction* pAction);
//	void OnCustomContextMenuRequested(QPoint qPoint);
};

#endif
