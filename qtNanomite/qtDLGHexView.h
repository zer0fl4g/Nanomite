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
#ifndef QTDLGHEXVIEW_H
#define QTDLGHEXVIEW_H

#include "ui_qtDLGHexView.h"

#include "clsHexViewWorker.h"

#include <QTCore>

/**
* @file qtDLGHexView.h
* @brief Displaying the exception editor
*/
class qtDLGHexView : public QWidget, public Ui_qtDLGHexViewClass
{
	Q_OBJECT

public:
	/**
	* @brief Responsible for initializing and showing the GUI of the hexview widget
	* @param parent Takes the a QWidget pointer of the calling QObject
	* @param flags A value of the Qt::WFlags enum which defines how the Dialog is shown
	* @param dwPID The process id from which the memory will be
	* @param StartOffset The offset from the begining of the display part
	* @param Size The size of memory which will be displayed
	*
	* @return no
	*/
	qtDLGHexView(QWidget *parent, Qt::WFlags flags,unsigned long dwPID, unsigned long long StartOffset,unsigned long long Size);
	/**
	* @brief Cleans up the data
	*
	* @return no
	*/
	~qtDLGHexView();

private:
	clsHexViewWorker *m_pHexDataWorker; /* holds a pointer to the worker thread */

	int m_maxRows; /* contains the maximal amount of rows which can be displayed */

	bool m_isFinished; /* contains the state of the worker thread */

private slots:
	/**
	* @brief A Qt slot which is called when the worked thread has finished
	*
	* @return no
	*/
	void DisplayData();
	/**
	* @brief A Qt slot which is called when the user scrolls, resizes
	* @param position The position from where the data will be displayed
	*
	* @return no
	*/
	void InsertDataFrom(int position);

protected:
	/**
	* @brief A override of the wheelevent to enable scrolling in the table widget
	* @param event A pointer to the event data
	*
	* @return no
	*/
	void wheelEvent(QWheelEvent *event);
	/**
	* @brief A override of the resizeevent to enable refilling in the table widget
	* @param event A pointer to the event data
	*
	* @return no
	*/
	void resizeEvent(QResizeEvent *event);
};

#endif
