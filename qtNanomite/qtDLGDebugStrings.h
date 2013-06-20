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
#ifndef QTDLGDBGSTR_H
#define QTDLGDBGSTR_H

#include "Ui_qtDLGDebugStrings.h"

#include <string>
#include <Windows.h>

class qtDLGDebugStrings : public QWidget, public Ui_qtDLGDebugStringsClass
{
	Q_OBJECT

public:
	qtDLGDebugStrings(QWidget *parent = 0, Qt::WFlags flags = 0);
	~qtDLGDebugStrings();

	public slots:
		int OnDbgString(std::wstring sMessage,DWORD dwPID);

private:
	int _iSelectedRow;

	private slots:
		void MenuCallback(QAction*);
		void OnCustomContextMenuRequested(QPoint qPoint);
};

#endif
