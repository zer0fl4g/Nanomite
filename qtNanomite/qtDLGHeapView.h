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
#ifndef QTDLGHEAPVIEW_H
#define QTDLGHEAPVIEW_H

#include "ui_qtDLGHeapView.h"

#include "qtDLGNanomite.h"

#include "clsAPIImport.h"

class qtDLGHeapView : public QWidget, public Ui_qtDLGHeapViewClass
{
	Q_OBJECT

public:
	qtDLGHeapView(QWidget *parent = 0, Qt::WFlags flags = 0,int iPID = 0);
	~qtDLGHeapView();

private:
	int _iPID,
		_iSelectedRow,
		_iForEntry,
		_iForEnd;

	qtDLGNanomite *myMainWindow;

	private slots:
		void DisplayHeap();
		void MenuCallback(QAction*);
		void OnSelectionChanged();
		void OnCustomContextMenuRequested(QPoint qPoint);
};

#endif
