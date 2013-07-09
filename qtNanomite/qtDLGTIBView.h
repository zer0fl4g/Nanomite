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
#ifndef QTDLGTIBVIEW_H
#define QTDLGTIBVIEW_H

#include "ui_qtDLGTIBView.h"

#include <Windows.h>

/**
* @file qtDLGTIBView.h
* @brief Displaying the TEB,TBI widget
*/
class qtDLGTIBView : public QWidget, public Ui_qtDLGTIBViewClass
{
	Q_OBJECT

public:
	/**
	* @brief Responsible for initializing and showing the GUI of the TEB,TBI widget
	* @param processHandle A handle to the process which contains the thread
	* @param threadHandle A handle to the thread which contains the whiched TEB,TBI
	* @param parent Takes the a QWidget pointer of the calling QObject
	* @param flags A value of the Qt::WFlags enum which defines how the Dialog is shown
	*
	* @return no
	*/
	qtDLGTIBView(HANDLE processHandle, HANDLE threadHandle, QWidget *parent = 0, Qt::WFlags flags = 0);
	/**
	* @brief Does not need to do something at the current stage
	*
	* @return no
	*/
	~qtDLGTIBView();

private:
	/**
	* @brief Will insert the given data to the given topelement
	* @param pTopElement A pointer to the topelement which should get the childnode
	* @param valueName The name of value which will be placed in the childnode
	* @param value The value which will be placed in the childnode
	*
	* @return no
	*/
	void InsertDataIntoTable(QTreeWidgetItem *pTopElement, QString valueName, DWORD64 value);
	/**
	* @brief A override of the resizeevent to enable refilling in the table widget
	* @param processHandle A handle to the process which contains the thread
	* @param threadHandle A handle to the thread which contains the whiched TEB,TBI
	*
	* @return no
	*/
	void ShowTIBForThread(HANDLE processHandle, HANDLE threadHandle);
};

#endif
