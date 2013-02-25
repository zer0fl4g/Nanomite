#ifndef QTDLGPEEDITOR_H
#define QTDLGPEEDITOR_H

#include "ui_qtDLGPEEditor.h"

#include "clsPEManager.h"

class qtDLGPEEditor : public QWidget
{
	Q_OBJECT

public:
	qtDLGPEEditor(clsPEManager *PEManager,QWidget *parent = 0, Qt::WFlags flags = 0, int PID = 0);
	~qtDLGPEEditor();
	
private:
	Ui::qtDLGPEEditorClass ui;

	int _PID;

	std::wstring _currentFile;

	clsPEManager* _PEManager;

	void LoadPEView();
};

#endif
