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
#ifndef QTDLGREGEDIT_H
#define QTDLGREGEDIT_H

#include "ui_qtDLGRegEdit86.h"
#include "ui_qtDLGRegEdit64.h"

#include <Windows.h>

/**
* @file qtDLGRegEdit.h
* @brief Displaying the regedit dialog
*/
class qtDLGRegEdit : public QDialog
{
	Q_OBJECT

public:
	/**
	* @brief Responsible for initializing and showing the GUI of the regedit dialog
	* @param parent Takes the a QWidget pointer of the calling QObject
	* @param flags A value of the Qt::WFlags enum which defines how the Dialog is shown
	* @param pProcessContext The process context which will be displayed
	* @param is64Bit Is the process x64 or x86
	*
	* @return no
	*/
	qtDLGRegEdit(QWidget *parent = 0, Qt::WFlags flags = 0,LPVOID pProcessContext = NULL,bool is64Bit = false);
	/**
	* @brief Does not need to do something at the current stage
	*
	* @return no
	*/
	~qtDLGRegEdit();

signals:
	/**
	* @brief Qt signal which is send when the user manipulated the process context to
	* reload the regview in the debugger
	*
	* @return no
	*/
	void OnUpdateRegView();

private:
	bool					m_is64Bit;			/* contains true if the process is x64 */
	LPVOID					m_pProcessContext;	/* the process context which will be displayed */

	Ui::qtDLGRegEdit86Class ui86;				/* the gui stuff for x86 context */
	Ui::qtDLGRegEdit64Class ui64;				/* the gui stuff for x64 context */

	/**
	* @brief Depending on the is64bit constructor the GUI will be filled with the register
	* values of the process context
	*
	* @return no
	*/
	void FillGUI();

private slots:
	/**
	* @brief A Qt slot which is called when the user clicked the close button
	*
	* @return no
	*/
	void OnExit();
	/**
	* @brief A Qt slot which is called when the user clicked the save button
	* it will save the data and close the dialog then
	*
	* @return no
	*/
	void OnSaveAndExit();
};

#endif
