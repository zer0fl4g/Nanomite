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
#ifndef QTDLGATTACH_H
#define QTDLGATTACH_H

#include "ui_qtDLGAttach.h"

/**
* @file qtDLGAttach.h
* @brief Displaying the attaching dialog
*/
class qtDLGAttach : public QDialog, public Ui_qtDLGAttachClass
{
	Q_OBJECT

public:
	/**
	* @brief Responsible for initializing and showing the GUI of the attaching dialog
	* @param parent Takes the a QWidget pointer of the calling QObject
	* @param flags A value of the Qt::WFlags enum which defines how the Dialog is shown
	*
	* @return no
	*/
	qtDLGAttach(QWidget *parent = 0, Qt::WFlags flags = 0);
	/**
	* @brief Does not need to do something at the current stage
	*
	* @return no
	*/
	~qtDLGAttach();

signals:
	/**
	* @brief A Qt signal which is send when a process was selected by the user
	*
	* @return no
	*/
	void StartAttachedDebug(int iPID,QString sFile);

private slots:
	/**
	* @brief A qt slot which is triggered when the user double clicks a process from the QTableWidget
	* @param iRow The selected row
	* @param iColumn The selected column
	*
	* @return no
	*/
	void OnProcessDoubleClick(int iRow,int iColumn);
	/**
	* @brief A qt slot which is triggered when the user reloads the list, or it fills the list on 
	* dialog opening
	*
	* @return no
	*/
	void FillProcessList();
};

#endif
