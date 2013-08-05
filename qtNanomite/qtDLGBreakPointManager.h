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
#ifndef QTDLGBPMANAGER_H
#define QTDLGBPMANAGER_H

#include "clsDebugger\clsDebugger.h"

#include "ui_qtDLGBreakPointManager.h"

/**
* @file qtDLGBreakPointManager.h
* @brief Displaying the breakpoint manager
*/
class qtDLGBreakPointManager : public QWidget, public Ui_qtDLGBreakPointManagerClass
{
	Q_OBJECT

public:
	/**
	* @brief Responsible for initializing and showing the GUI of the breakpoint manager
	* @param parent Takes the a QWidget pointer of the calling QObject
	* @param flags A value of the Qt::WFlags enum which defines how the Dialog is shown
	*
	* @return no
	*/
	qtDLGBreakPointManager(QWidget *parent = 0, Qt::WFlags flags = 0);

	/**
	* @brief Does not need to do something at the current stage
	*
	* @return no
	*/
	~qtDLGBreakPointManager();

	/**
	* @brief Deletes the content of the completer which is used to offer auto completion
	* in the Offset LineEdit
	*
	* @return no
	*/
	void DeleteCompleterContent();

	/**
	* @brief Returns the current content of the completer list
	*
	* @return QStringList The current content of the completer list
	*/
	static QStringList ReturnCompleterList();

public slots:
	/**
	* @brief A Qt slot which is called when the clsDebugger instance added a new breakpoint
	* @param newBP A struct which contains the BP data
	* @param breakpointType A int which describes what kind of BP it is
	*
	* @return no
	*/
	void OnUpdate(BPStruct newBP, int breakpointType);
	/**
	* @brief A Qt slot which is called when the clsDebugger instance deleted a breakpoint
	* @param breakpointOffset The offset of the deleted bp
	*
	* @return no
	*/
	void OnDelete(quint64 breakpointOffset);
	/**
	* @brief A Qt slot which is called when the User pressed the "Del" key to remove a bp
	*
	* @return no
	*/
	void OnBPRemove();
	/**
	* @brief A Qt slot which is called from the clsDebugger instance when a new PE file was 
	* loaded in the PEManager
	* @param FilePath The path of the new file
	* @param processID The process id of the new process
	*
	* @return no
	*/
	void UpdateCompleter(std::wstring FilePath, int processID);

private:
	QStringList	m_completerList; /* a QStringList which contains all the imports of the loaded processes */
	QCompleter	*m_pAPICompleter; /* a pointer to the current QCompleter */
	static qtDLGBreakPointManager *pThis; /* a pointer to the instance of this BP Manager */

private slots:
	/**
	* @brief A Qt slot which is called when the user pushes the close button
	*
	* @return no
	*/
	void OnClose();
	/**
	* @brief A Qt slot which is called when the user adds or updates a BP
	*
	* @return no
	*/
	void OnAddUpdate();
	/**
	* @brief A Qt slot which is called when the user selects a bp from the table
	* @param iRow The selected row
	* @param iCol The selected column
	*
	* @return no
	*/
	void OnSelectedBPChanged(int iRow, int iCol);
};

#endif
