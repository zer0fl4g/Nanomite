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
	{	
		_currentFile = _PEManager->getFilenameFromPID(_PID);
		this->setWindowTitle(QString("[Nanomite] - PEEditor : %1 - %2").arg(PID,8,16,QChar('0')).arg(QString().fromStdWString(_currentFile)));
		
		InitList();
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

void qtDLGPEEditor::InitList()
{
	//// List Import
	//tblImports->horizontalHeader()->resizeSection(0,135);
	//tblImports->horizontalHeader()->resizeSection(1,135);

	//// List Section
	////tblSections->horizontalHeader()->resizeSection(0,85);

	//// List DOS
	//tblDOS->horizontalHeader()->resizeSection(0,135);
	//tblDOS->horizontalHeader()->resizeSection(1,135);

	//// List FILE
	//tblFH->horizontalHeader()->resizeSection(0,135);
	//tblFH->horizontalHeader()->resizeSection(1,250);

	//// List OPTIONAL
	//tblOH->horizontalHeader()->resizeSection(0,125);
	//tblOH->horizontalHeader()->resizeSection(0,125);
}

void qtDLGPEEditor::LoadPEView()
{

}

