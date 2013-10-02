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

#include <Windows.h>

/**
* @file qtDLGDebugStrings.h
* @brief Displaying the debugstring widget
*/
class qtDLGDebugStrings : public QWidget, public Ui_qtDLGDebugStringsClass
{
	Q_OBJECT

public:
	/**
	* @brief Responsible for initializing and showing the GUI of the debugstring widget
	* @param parent Takes the a QWidget pointer of the calling QObject
	* @param flags A value of the Qt::WFlags enum which defines how the Dialog is shown
	*
	* @return no
	*/
	qtDLGDebugStrings(QWidget *parent = 0, Qt::WFlags flags = 0);
	/**
	* @brief Does not need to do something at the current stage
	*
	* @return no
	*/
	~qtDLGDebugStrings();

public slots:
	/**
	* @brief A Qt slot which is called when the clsDebugger instance catched a new debug string
	* @param debugString The new debugstring
	* @param processID The process id from the the debugstring comes
	*
	* @return no
	*/
	void OnDbgString(QString debugString, DWORD processID);

private:
	int m_selectedRow; /* contains the selected row if the user opens a context menu */

private slots:
	/**
	* @brief A Qt slot which is called when the user selected a option in the context menu
	* @param pAction The selected action
	*
	* @return no
	*/
	void MenuCallback(QAction *pAction);
	/**
	* @brief A Qt slot which is called when the user wants to open the context menu
	* @param qPoint The point where the user clicked
	*
	* @return no
	*/
	void OnCustomContextMenuRequested(QPoint qPoint);
};

#endif
