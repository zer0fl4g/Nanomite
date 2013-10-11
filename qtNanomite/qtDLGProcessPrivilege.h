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
#ifndef QTDLGPROCESSPRIVILEGE_H
#define QTDLGPROCESSPRIVILEGE_H

#include "ui_qtDLGProcessPrivilege.h"

#include "qtDLGNanomite.h"

/**
* @file qtDLGProcessPrivilege.h
* @brief Displaying the process privilege view widget
*/
class qtDLGProcessPrivilege : public QWidget, public Ui_qtDLGProcessPrivilegeClass
{
	Q_OBJECT

public:
	/**
	* @brief Responsible for initializing and showing the GUI of the process privilege view widget
	* @param parent Takes the a QWidget pointer of the calling QObject
	* @param flags A value of the Qt::WFlags enum which defines how the Dialog is shown
	* @param processID The process id of which the privileges will be displayed
	*
	* @return no
	*/
	qtDLGProcessPrivilege(QWidget *parent = 0, Qt::WFlags flags = 0, qint32 processID = -1);
	/**
	* @brief Does not need to do something at the current stage
	*
	* @return no
	*/
	~qtDLGProcessPrivilege();

private:
	int			m_processID,
				m_selectedRow,
				m_processLoopEntry,
				m_processLoopEnd;

	qtDLGNanomite *m_pMainWindow;

private slots:
	/**
	* @brief A Qt slot which is called when the privileges data should be displayed
	*
	* @return no
	*/
	void DisplayPrivileges();
	/**
	* @brief A Qt slot which is called when the user clicks on an action in the
	* context menu
	* @param pAction The action the user selected in the context menu
	*
	* @return no
	*/
	void MenuCallback(QAction *pAction);
	/**
	* @brief A Qt slot which is called when the user opens the context menu
	* @param qPoint The position where the user opened the context menu
	*
	* @return no
	*/
	void OnCustomContextMenuRequested(QPoint qPoint);
};

#endif