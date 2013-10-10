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
#ifndef QTDLGWINDOWVIEW_H
#define QTDLGWINDOWVIEW_H

#include "ui_qtDLGWindowView.h"

#include "qtDLGNanomite.h"

/**
* @file qtDLGWindowView.h
* @brief Displaying the window view widget
*/
class qtDLGWindowView : public QWidget, public Ui_qtDLGWindowViewClass
{
	Q_OBJECT

public:
	static qtDLGWindowView *pThis; /* static pointer to instance for enum function */

	/**
	* @brief Responsible for initializing and showing the GUI of the window view widget
	* @param parent Takes the a QWidget pointer of the calling QObject
	* @param flags A value of the Qt::WFlags enum which defines how the Dialog is shown
	* @param processID The process id from which the windows will be displayed
	*
	* @return no
	*/
	qtDLGWindowView(QWidget *parent = 0, Qt::WFlags flags = 0,qint32 processID = 0);
	/**
	* @brief Does not need to do something at the current stage
	*
	* @return no
	*/
	~qtDLGWindowView();

private:
	int		m_processID, /* contains the needed process id */
			m_processCountEntry, /* begin of the loop if we need more than one process */
			m_processCountEnd; /* end of the loop if we need more than one process */

	qtDLGNanomite *m_pMainWindow; /* pointer to the main application */

	/**
	* @brief Callback for the "EnumWindows" function
	* @param hWnd The window handle which was enumed
	* @param lParam Any additional data as a pointer which was send from the enum function
	*
	* @return no
	*/
	static bool CALLBACK EnumWindowCallBack(HWND hWnd,LPARAM lParam);

private slots:
	/**
	* @brief Will call the "EnumWindows" API for each process based on user choice
	*
	* @return no
	*/
	void EnumWindow();
};
#endif