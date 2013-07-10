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
#ifndef QTDLGSTACK_H
#define QTDLGSTACK_H

#include <Windows.h>

#include <QDockWidget>

#include "ui_qtDLGStack.h"

/**
* @file qtDLGStack.h
* @brief Displaying the stackview widget
*/
class qtDLGStack : public QDockWidget, public Ui_qtDLGStack
{
	Q_OBJECT

public:
	/**
	* @brief Responsible for initializing and showing the GUI of the stackview widget
	* @param parent Takes the a QWidget pointer of the calling QObject
	*
	* @return no
	*/
	qtDLGStack(QWidget *parent = 0);
	/**
	* @brief Does not need to do something at the current stage
	*
	* @return no
	*/
	~qtDLGStack();

	/**
	* @brief Loads the needed data into the stack table
	* @param stackBaseOffset The base offset from where it begins to grab the data
	* @param stackAlign The align number wich changes for x64 or x86
	*
	* @return no
	*/
	void LoadStackView(quint64 stackBaseOffset, DWORD stackAlign);
	
public slots:
	/**
	* @brief A Qt slot which is called when the user scrolls in the table with
	* the mouse wheel or the scrollbar
	* @param iValue The number of line which have been scrolled up/down
	*
	* @return no
	*/
	void OnStackScroll(int iValue);

private:
	int m_selectedRow; /* contains the row which the user clicked to open the context menu */

private slots:
	/**
	* @brief A Qt slot which is called when the user clicks on an action in the
	* context menu
	* @param pAction The action the user selected in the context menu
	*
	* @return no
	*/
	void MenuCallback(QAction *pAction);
	/**
	* @brief A Qt slot which is called when the user opens the context menu
	* @param qPoint The position where the user opened the context menu
	*
	* @return no
	*/
	void OnContextMenu(QPoint qPoint);

protected:
	/**
	* @brief An override to handle the wheel scroll event
	* @param pOpject The object on which the event happend
	* @param event The event type
	*
	* @return no
	*/
	bool eventFilter(QObject *pOpject,QEvent *event);
};

#endif // QTDLGSTACK_H
