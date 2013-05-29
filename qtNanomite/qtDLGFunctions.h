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
#ifndef QTDLGFUNCTIONS_H
#define QTDLGFUNCTIONS_H

#include "ui_qtDLGFunctions.h"

#include "clsFunctionsViewWorker.h"

#include <Windows.h>
#include <QtCore>

class qtDLGFunctions : public QWidget, public Ui_qtDLGFunctionsClass
{
	Q_OBJECT

public:
	qtDLGFunctions(QWidget *parent = 0, Qt::WFlags flags = 0,qint32 iPID = 0);
	~qtDLGFunctions();

signals:
	void ShowInDisAs(quint64 Offset);

private:
	int _iPID,
		_iSelectedRow;

	clsFunctionsViewWorker *m_pFunctionWorker;

	private slots:
		void InsertDataFrom(int position);
		void DisplayFunctionLists();
		void OnCustomContextMenu(QPoint qPoint);
		void MenuCallback(QAction* pAction);

protected:
	void wheelEvent(QWheelEvent * event);
	void resizeEvent(QResizeEvent *event);
};

#endif
