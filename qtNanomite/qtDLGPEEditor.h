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

class qtDLGPEEditor : public QWidget, Ui_qtDLGPEEditorClass
{
	Q_OBJECT

public:
	qtDLGPEEditor(clsPEManager *PEManager,QWidget *parent = 0, Qt::WFlags flags = 0, int PID = 0, std::wstring FileName = L"");
	~qtDLGPEEditor();
	
private:
	int _PID;

	std::wstring _currentFile;

	clsPEManager* _PEManager;

	void InsertDosHeader();
	void InsertFileHeader();
	void InsertOptionalHeader();
	void InsertHeaderData(QTableWidget* tblHeaderTable,QString ValueName,quint64 dwValue);
	void InsertExports();
	void InsertImports();
	void InsertSections();
	void LoadPEView();
	void InitList();
};

#endif
