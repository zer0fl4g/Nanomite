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
#ifndef QTDLGPEBVIEW_H
#define QTDLGPEBVIEW_H

#include "ui_qtDLGPEBView.h"

#include <Windows.h>

/**
* @file qtDLGPEBView.h
* @brief Displaying the PEB,PBI widget
*/
class qtDLGPEBView : public QWidget, public Ui_qtDLGPEBViewClass
{
	Q_OBJECT

public:
	/**
	* @brief Responsible for initializing and showing the GUI of the PEB,PBI widget
	* @param processHandle A handle to the process which contains the thread
	* @param threadHandle A handle to the thread which contains the whiched TEB,TBI
	* @param parent Takes the a QWidget pointer of the calling QObject
	* @param flags A value of the Qt::WFlags enum which defines how the Dialog is shown
	*
	* @return no
	*/
	qtDLGPEBView(HANDLE hProc, QWidget *parent = 0, Qt::WFlags flags = 0);
	/**
	* @brief Does not need to do something at the current stage
	*
	* @return no
	*/
	~qtDLGPEBView();

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
	* @param processHandle A handle to the process which contains the whiched TEB,TBI
	*
	* @return no
	*/
	void ShowPEBForProcess(HANDLE hProc);
};

#endif
