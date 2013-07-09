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
#ifndef QTDLGEXCEPTIONASK_H
#define QTDLGEXCEPTIONASK_H

#include "ui_qtDLGExceptionAsk.h"

#include <Windows.h>

/**
* @file qtDLGExceptionAsk.h
* @brief Displaying the Exception Assistant
*/
class qtDLGExceptionAsk : public QDialog, Ui_qtDLGExceptionAskClass
{
	Q_OBJECT

public:
	/**
	* @brief Responsible for initializing and showing the GUI of the Exception Assistant
	* @param exceptionCode The exception code
	* @param parent Takes the a QWidget pointer of the calling QObject
	* @param flags A value of the Qt::WFlags enum which defines how the Dialog is shown
	*
	* @return no
	*/
	qtDLGExceptionAsk(DWORD exceptionCode, QWidget *parent = 0, Qt::WFlags flags = 0);
	/**
	* @brief Does not need to do something at the current stage
	*
	* @return no
	*/
	~qtDLGExceptionAsk();

signals:
	/**
	* @brief A Qt signal which contains the instruction how to handle the occured exception
	* @param handleException The value for the debugger to know how to handle this exception
	*
	* @return no
	*/
	void ContinueException(int handleException);

private:
	int m_retValue; /* stores a value depending on what the user decided */

private slots:
	/**
	* @brief A Qt slot which is called when the user selected the exception to be ignored
	*
	* @return no
	*/
	void ExceptionIgnore();
	/**
	* @brief A Qt slot which is called when the user selected the exception to be send to the application
	*
	* @return no
	*/
	void ExceptionSendToApp();
	/**
	* @brief A Qt slot which is called when the user selected to break on the exception
	*
	* @return no
	*/
	void ExceptionBreak();

protected:
	/**
	* @brief A override of the closeEvent which is called when the user has done his input and 
	* the window closes itself
	*
	* @return no
	*/
	void closeEvent(QCloseEvent *event);
};

#endif
