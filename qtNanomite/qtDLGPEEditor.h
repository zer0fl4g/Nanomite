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
#ifndef QTDLGPEEDITOR_H
#define QTDLGPEEDITOR_H

#include "ui_qtDLGPEEditor.h"

#include "clsPEManager.h"

/**
* @file qtDLGPEEditor.h
* @brief Displaying PEEditor and functionality of it
*/
class qtDLGPEEditor : public QWidget, Ui_qtDLGPEEditorClass
{
	Q_OBJECT

public:
	/**
	* @brief Calls the PEEditor with several parameters.
	* @param *PEManager takes a pointer to current PEManager
	* @param parent Takes the a QWidget pointer of the calling QObject
	* @param flags A value of the Qt::WFlags enum which defines how the Dialog is shown
	* @param processID is the ID of the process to open
	* @param FileName from the PE file
	*
	* @return no
	*/
	qtDLGPEEditor(clsPEManager *PEManager,QWidget *parent = 0, Qt::WFlags flags = 0, int processID = 0, QString FileName = "");
	/**
	* @brief Deconstructs qtDLGPEEditor()
	*
	* @return no
	*/
	~qtDLGPEEditor();

private:
	int m_processID,
		m_selectedRow;

	quint64 m_selectedOffset;

	QString m_currentFile;

	clsPEManager* m_pEManager;

	/**
	* @brief Inserts data from PIMAGE_DOS_HEADER of current file to topElement.
	*
	* @return no
	*/
	void InsertDosHeader();
	/**
	* @brief Inserts needed data from PIMAGE_NT_HEADERS32 or PIMAGE_NT_HEADERS64 
	* of current file to topElement.
	*
	* @return no
	*/
	void InsertFileHeader();
	/**
	* @brief Inserts optional data from PIMAGE_NT_HEADERS32 or PIMAGE_NT_HEADERS64 
	* of current file to topElement.
	* 
	* @return no
	*/
	void InsertOptionalHeader();
	/**
	* @brief Inserts collected header data from topElement to dataElement.
	* @param topElement is the QTreeWidgetItem where header data collected in
	* @param ValueName is the name of the value from topElement to write in dataElement
	* @param dwValue is the Value from ValueName in topElement to write in dataElement
	*
	* @return no
	*/
	void InsertHeaderData(QTreeWidgetItem *topElement,QString ValueName,quint64 dwValue);
	/**
	* @brief Inserts exports(modules) data to topElement.
	* 
	* @return no
	*/
	void InsertExports();
	/**
	* @brief Inserts imports(modules) data to topElement.
	* 
	* @return no
	*/
	void InsertImports();
	/**
	* @brief Inserts sections data to topElement.
	* 
	* @return no
	*/
	void InsertSections();
	/**
	* @brief Calls several functions to collect needed data for PEEditor. 
	* 
	* @return no
	*/
	void LoadPEView();
	/**
	* @brief Inits needed treePE sections.
	* 
	* @return no
	*/
	void InitList();

private slots:
	/**
	* @brief Qt slot which is triggered when right click on a table row.
	* A contextmenu will open to show up disassembly view.
	* @param qAction takes a pointer to selected table column.
	*
	* @return no
	*/
	void MenuCallback(QAction* pAction);
	/**
	* @brief Qt slot which is triggered when right click on a table row.
	* The contextmenu will open.
	* @param qAction takes a pointer to selected table column.
	*
	* @return no
	*/
	void OnCustomContextMenu(QPoint qPoint);

};

#endif
