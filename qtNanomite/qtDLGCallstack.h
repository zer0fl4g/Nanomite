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
#ifndef QTDLGCALLSTACK_H
#define QTDLGCALLSTACK_H

#include <string>

#include <QDockWidget>

#include "ui_qtDLGCallstack.h"

class qtDLGCallstack : public QDockWidget, public Ui_qtDLGCallstack
{
	Q_OBJECT

public:
	qtDLGCallstack(QWidget *parent = 0);
	~qtDLGCallstack();

private:
	int _iSelectedRow;

public slots:
	void OnDisplaySource(QTableWidgetItem *pItem);
	void OnContextMenu(QPoint);
	void MenuCallback(QAction* pAction);
	int OnCallStack(quint64 dwStackAddr,quint64 dwReturnTo,std::wstring sReturnToFunc,std::wstring sModuleName,quint64 dwEIP,std::wstring sFuncName,std::wstring sFuncModule,std::wstring sSourceFilePath,int iSourceLineNum);

signals:
	void DisplaySource(QString,int);

};

#endif // QTDLGCALLSTACK_H
