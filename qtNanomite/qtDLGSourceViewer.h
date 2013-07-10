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
#ifndef QTDLGSOURCEVIEWER_H
#define QTDLGSOURCEVIEWER_H

#include "ui_qtDLGSourceViewer.h"

/**
* @file qtDLGSourceViewer.h
* @brief Displaying the sourceview widget
*/
class qtDLGSourceViewer : public QWidget, public Ui_qtDLGSourceViewerClass
{
	Q_OBJECT

public:
	bool IsSourceAvailable; /* indicates if a source is avalible and allows the user to open the widget */

	/**
	* @brief Responsible for initializing and showing the GUI of the about dialog
	* @param parent Takes the a QWidget pointer of the calling QObject
	* @param flags A value of the Qt::WFlags enum which defines how the Dialog is shown
	*
	* @return no
	*/
	qtDLGSourceViewer(QWidget *parent = 0, Qt::WFlags flags = 0);
	/**
	* @brief Does not need to do something at the current stage
	*
	* @return no
	*/
	~qtDLGSourceViewer();

public slots:
	/**
	* @brief This Qt slot is called then the debugger needs to display a sourcefile
	* @param sourceFile This is the file which will be displayed
	* @param sourceLine This is the line where the debugger is and which needs to be
	* highlighted
	*
	* @return no
	*/
	void OnDisplaySource(QString sourceFile, int sourceLine);
};

#endif
