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
#ifndef QTDLGFUNCTIONS_H
#define QTDLGFUNCTIONS_H

#include "ui_qtDLGFunctions.h"

#include "clsFunctionsViewWorker.h"

#include <Windows.h>
#include <QtCore>

/**
* @file qtDLGFunctions.h
* @brief Displaying the function view widget
*/
class qtDLGFunctions : public QWidget, public Ui_qtDLGFunctionsClass
{
	Q_OBJECT

public:
	/**
	* @brief Responsible for initializing and showing the GUI of the function view widget
	* @param parent Takes the a QWidget pointer of the calling QObject
	* @param flags A value of the Qt::WFlags enum which defines how the Dialog is shown
	* @param processID The process id from which the functions will be shown
	*
	* @return no
	*/
	qtDLGFunctions(QWidget *parent = 0, Qt::WFlags flags = 0,qint32 processID = 0);
	/**
	* @brief Deleting the worker data
	*
	* @return no
	*/
	~qtDLGFunctions();

signals:
	/**
	* @brief A Qt signal which is send when the user wants to display a function in 
	* the gui
	* @param Offset The offset of the function which will be shown
	*
	* @return no
	*/
	void ShowInDisAs(quint64 Offset);

private:
	int m_processID,	/* contains the process id from which the functions will be extracted */
		m_selectedRow;	/* contains the selected row if the user opens the context menu */

	clsFunctionsViewWorker *m_pFunctionWorker; /* a pointer to the data background worker */

private slots:
	/**
	* @brief A Qt slot which is called when the user clicks opens,scrolls,resizes and will
	* insert the data into the table
	* @param position The position from where the data will be inserted
	*
	* @return no
	*/
	void InsertDataFrom(int position);
	/**
	* @brief A Qt slot which is called when the data needs to be inserted into the table
	*
	* @return no
	*/
	void DisplayFunctionLists();
	/**
	* @brief A Qt slot which is called when the user opens the context menu
	* @param qPoint The point where the user opened the context menu
	*
	* @return no
	*/
	void OnCustomContextMenu(QPoint qPoint);
	/**
	* @brief A Qt slot which is called when the user clicks on a element in the context menu
	* @param pAction The action which was selected from the context menu
	*
	* @return no
	*/
	void MenuCallback(QAction *pAction);
	/**
	* @brief A Qt slot which is called when the user double clicks a function and sends it
	* to the disassembler
	* @param pSelectedRow The row on which the user double clicked
	*
	* @return no
	*/
	void OnSendToDisassembler(QTableWidgetItem *pSelectedRow);
	/**
	* @brief A Qt slot which is called when the user pressed return
	*
	* @return no
	*/
	void OnReturnPressed();

protected:
	/**
	* @brief A override of the wheel event to enable scrolling in the table widget
	* @param event A pointer to the event data
	*
	* @return no
	*/
	void wheelEvent(QWheelEvent *event);
	/**
	* @brief A override of the resize event to enable refilling in the table widget
	* @param event A pointer to the event data
	*
	* @return no
	*/
	void resizeEvent(QResizeEvent *event);
};

#endif
