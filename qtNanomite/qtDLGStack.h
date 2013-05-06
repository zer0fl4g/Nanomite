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
#ifndef QTDLGSTACK_H
#define QTDLGSTACK_H

#include <Windows.h>

#include <QDockWidget>

#include "ui_qtDLGStack.h"

class qtDLGStack : public QDockWidget, public Ui_qtDLGStack
{
	Q_OBJECT

public:
	qtDLGStack(QWidget *parent = 0);
	~qtDLGStack();

	void LoadStackView(quint64 dwESP, DWORD dwStackSize);
	
public slots:
	void OnStackScroll(int iValue);

private:
	int _iSelectedRow;

private slots:
	void MenuCallback(QAction *pAction);
	void OnContextMenu(QPoint qPoint);

protected:
	bool eventFilter(QObject *pOpject,QEvent *event);
};

#endif // QTDLGSTACK_H
