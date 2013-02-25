#include "qtDLGPEEditor.h"
#include "clsPEManager.h"

qtDLGPEEditor::qtDLGPEEditor(clsPEManager *PEManager,QWidget *parent, Qt::WFlags flags, int PID)
{
	ui.setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose,true);
	this->setFixedSize(this->width(),this->height());

	_PID = PID;
	_PEManager = PEManager;
	
	if(_PEManager != NULL)
	{	_currentFile = _PEManager->getFilenameFromPID(_PID);

		LoadPEView();
	}
	else
	{
		MessageBoxW(NULL,L"Could not load Filename!",L"Nanomite",MB_OK);
		close();
	}
}

qtDLGPEEditor::~qtDLGPEEditor()
{

}

void qtDLGPEEditor::LoadPEView()
{

}