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
	void LoadPEView();
	void InitList();
};

#endif
