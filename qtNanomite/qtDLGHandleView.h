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

/**
* @file qtDLGHandleView.h
* @brief Displaying the handleview widget
*/
class qtDLGHandleView : public QWidget, public Ui_qtDLGHandleViewClass
{
	Q_OBJECT

public:
	/**
	* @brief Responsible for initializing and showing the GUI of the hexview widget
	* @param parent Takes the a QWidget pointer of the calling QObject
	* @param flags A value of the Qt::WFlags enum which defines how the Dialog is shown
	* @param processID The process id from which the handles will be shown
	*
	* @return no
	*/
	qtDLGHandleView(QWidget *parent = 0, Qt::WFlags flags = 0, qint32 processID = 0);
	/**
	* @brief Does not need to do something at the current stage
	*
	* @return no
	*/
	~qtDLGHandleView();

private:
	size_t	m_processID,			/* contains the process id from which the handle will be shown */
			m_processCountEntry,	/* loop for process ids to show if we have more than one */
			m_processCountEnd;		/* loop for process ids to show if we have more than one */
	int		m_selectedRow;			/* contains the row on which the user opend the context menu */
	
	/**
	* @brief A Qt slot which is called when the user scrolls, resizes
	* @param dwPID The process id of the handle
	* @param dwHandle The handle id
	* @param ptType The type of handle
	* @param ptName The name of the handle
	*
	* @return no
	*/
	void InsertDataIntoTable(DWORD dwPID, DWORD dwHandle, PTCHAR ptType, PTCHAR ptName);

private slots:
	/**
	* @brief A Qt slot which is called when the data needs to be displayed
	*
	* @return no
	*/
	void OnDisplayHandles();
	/**
	* @brief A Qt slot which is called when the user right clicks to open the context menu
	* @param qPoint The point where the user opened the context menu
	*
	* @return no
	*/
	void OnCustomContextMenuRequested(QPoint qPoint);
	/**
	* @brief A Qt slot which is called when the user clicks on a element in the context menu
	* @param pAction The action which was selected from the context menu
	*
	* @return no
	*/
	void MenuCallback(QAction* pAction);
};

#endif
