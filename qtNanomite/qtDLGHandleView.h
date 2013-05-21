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
#ifndef QTDLGHANDLEVIEW_H
#define QTDLGHANDLEVIEW_H

#include "ui_qtDLGHandleView.h"

#include <Windows.h>

class qtDLGHandleView : public QWidget, public Ui_qtDLGHandleViewClass
{
	Q_OBJECT

public:
	qtDLGHandleView(QWidget *parent = 0, Qt::WFlags flags = 0,qint32 iPID = 0);
	~qtDLGHandleView();

private:
	size_t	_iPID,
			_iForEntry,
			_iForEnd;
	int		_iSelectedRow;

	void InsertDataIntoTable(DWORD dwPID,DWORD dwHandle,PTCHAR ptType,PTCHAR ptName);

private slots:
	void OnDisplayHandles();
	void OnCustomContextMenuRequested(QPoint qPoint);
	void MenuCallback(QAction* pAction);
};

#endif
