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
#ifndef QTDLGSTRINGVIEW_H
#define QTDLGSTRINGVIEW_H

#include "ui_qtDLGStringView.h"

#include "qtDLGNanomite.h"

#include "clsStringViewWorker.h"

class qtDLGStringView : public QWidget, public Ui_qtDLGStringViewClass
{
	Q_OBJECT

public:
	qtDLGStringView(QWidget *parent = 0, Qt::WFlags flags = 0, qint32 processID = 0);
	~qtDLGStringView();

private:
	size_t				m_processID,
						m_forEntry,
						m_forEnd;
	int					m_selectedRow;

	qtDLGNanomite		*m_pMainWindow;

	clsStringViewWorker	*m_pStringProcessor;

	void PrintStringToList(int processID,QString stringToPrint, int stringOffset);

private slots:
	void InsertDataFrom(int position);
	void DataProcessing();
	void OnCustomContextMenuRequested(QPoint qPoint);
	void MenuCallback(QAction* pAction);
	void DisplayStrings();

protected:
	void wheelEvent(QWheelEvent * event);
	void resizeEvent(QResizeEvent *event);
};
#endif
