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
#ifndef QTDLGEXCEPTIONEDIT_H
#define QTDLGEXCEPTIONEDIT_H

#include "ui_qtDLGExceptionEdit.h"

#include <Windows.h>

/**
* @file qtDLGExceptionEdit.h
* @brief Displaying the exception editor
*/
class qtDLGExceptionEdit : public QDialog, public Ui_qtDLGExceptionEditClass
{
	Q_OBJECT

public:
	/**
	* @brief Responsible for initializing and showing the GUI of the exception editor
	* @param parent Takes the a QWidget pointer of the calling QObject
	* @param flags A value of the Qt::WFlags enum which defines how the Dialog is shown
	* @param exceptionCode the exception code the user wants to modify
	* @param handleException The original value how the exception would be handled
	*
	* @return no
	*/
	qtDLGExceptionEdit(QWidget *parent = 0, Qt::WFlags flags = 0, DWORD exceptionCode = 0, int handleException = -1);
	/**
	* @brief Does not need to do something at the current stage
	*
	* @return no
	*/
	~qtDLGExceptionEdit();

signals:
	/**
	* @brief A Qt signal which sende the new value for the exception to the option dialog
	* @param exceptionCode The exception code
	* @param handleCode The value for the debugger to know how to handle this exception
	*
	* @return no
	*/
	void OnInsertNewException(DWORD exceptionCode, int handleCode);

private:
	/**
	* @brief Load a default list with different exception codes, names to be used in a 
	* QCompleter for the edit field
	*
	* @return QStringList A list containing exception codes and names
	*/
	QStringList LoadExceptionList();

private slots:
	/**
	* @brief A Qt slot which is called when the user clicked on the save button
	*
	* @return no
	*/
	void OnSave();
	/**
	* @brief A Qt slot which is called when the user clicked on the cancel button
	*
	* @return no
	*/
	void OnCancel();
};

#endif
