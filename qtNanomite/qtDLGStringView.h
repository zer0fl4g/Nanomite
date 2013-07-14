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
#ifndef QTDLGSTRINGVIEW_H
#define QTDLGSTRINGVIEW_H

#include "ui_qtDLGStringView.h"

#include "qtDLGNanomite.h"

#include "clsStringViewWorker.h"

/**
* @file qtDLGStringView.h
* @brief Displaying stringdata from debugged file
*/
class qtDLGStringView : public QWidget, public Ui_qtDLGStringViewClass
{
	Q_OBJECT

public:
	/**
	* @brief Responsible for initializing and showing the GUI of the string view.
	* @param parent Takes the a QWidget pointer of the calling QObject
	* @param flags A value of the Qt::WFlags enum which defines how the Dialog is shown
	* @param processID ID from the process to read string data 
	*
	* @return no
	*/
	qtDLGStringView(QWidget *parent = 0, Qt::WFlags flags = 0, qint32 processID = 0);
	/**
	* @brief cleanup used data.
	*
	* @return no
	*/
	~qtDLGStringView();

private:
	size_t				m_processID,
						m_forEntry,
						m_forEnd;
	int					m_selectedRow;

	qtDLGNanomite		*m_pMainWindow;

	clsStringViewWorker	*m_pStringProcessor;

	void PrintStringToList(int processID,QString stringToPrint, int stringOffset);

private slots:
	/**
	* @brief Qt slot which is triggered when Scrollbar has changed value
	*
	* @return no
	*/
	void InsertDataFrom(int position);
	/**
	* @brief Qt slot which is triggered when the mainwindow from "qtDLGStringView"
	* is created or when the Shortcut "F5" is used.
	*
	* A QMap will be created where the Process ID (dwPID) and the Filename (sFileName) 
	* from the process/es will be saved.
	*
	* "StringViewWorker" triggers "DisplayStrings()" when finished processing data. 
	*
	* @return no
	*/
	void DataProcessing();
	/**
	* @brief Qt slot which is triggered when a right click on the StringView Window 
	* is performed to displayand manage the StringView context menu. 
	* @param qPoint takes a pointer to the selected tablerow.
	*
	* @return no
	*/
	void OnCustomContextMenuRequested(QPoint qPoint);
	/**
	* @brief Qt slot which is triggered when right click on a table row.
	* A contextmenu will open to copy over selected data from row to clipboard.
	* @param qAction takes a pointer to selected table column (Line, Offset, String).
	*
	* @return no
	*/
	void MenuCallback(QAction* pAction);
	/**
	* @brief Qt slot which is triggered when the StringView is opened. It fits the
	* size of the Scrollbar to set maximum Value of table rows. Then it will fill
	* the rows with content by triggering InsertDataFrom().
	*
	* @return no
	*/
	void DisplayStrings();

protected:
	/**
	* @brief Qt slot which is triggered when scrolling in StringView.
	* It calculates how much rows where moved up or down and deletes/appends
	* data to the StringView.
	* @param *event is the mousewheel event triggerd when mousewheel is used.
	*
	* @return no
	*/
	void wheelEvent(QWheelEvent * event);
	/**
	* @brief When the StringView window is resized it will detect new size of
	* the window and insert data to displayed rows.
	* @param *event triggerd by resizing the StringView window.
	*
	* @return no
	*/
	void resizeEvent(QResizeEvent *event);
};
#endif
