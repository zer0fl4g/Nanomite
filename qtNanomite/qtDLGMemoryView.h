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
#ifndef QTDLGMEMORY_H
#define QTDLGMEMORY_H

#include "ui_qtDLGMemoryView.h"

#include "qtDLGNanomite.h"

/**
* @file qtDLGHeapView.h
* @brief Displaying the heapview widget
*/
class qtDLGMemoryView : public QWidget, public Ui_qtDLGMemoryViewClass
{
	Q_OBJECT

public:
	/**
	* @brief Responsible for initializing and showing the GUI of the heapview widget
	* @param parent Takes the a QWidget pointer of the calling QObject
	* @param flags A value of the Qt::WFlags enum which defines how the Dialog is shown
	* @param processID The process id from which the heap will be shown
	*
	* @return no
	*/
	qtDLGMemoryView(QWidget *parent = 0, Qt::WFlags flags = 0,qint32 processID = 0);
	/**
	* @brief Does not need to do something at the current stage
	*
	* @return no
	*/
	~qtDLGMemoryView();

private:
	int		m_processID,
			m_processCountEntry,
			m_processCountEnd,
			m_selectedRow;

	qtDLGNanomite *m_pMainWindow;

	/**
	* @brief This is called when the context menu is opened to display the current
	* mem protection
	*
	* @return no
	*/
	DWORD GetPageProtectionFlags();

	/**
	* @brief This sets the passed protection to the memory page
	* @param protectionFlag The proctection which should be placed on the selected
	* memory page
	*
	* @return no
	*/
	void SetPageProctection(DWORD protectionFlag);

private slots:
	/**
	* @brief A Qt slot which is called when the memory data should be displayed
	*
	* @return no
	*/
	void DisplayMemory();
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
	void OnCustomContextMenuRequested(QPoint qPoint);
};

#endif
