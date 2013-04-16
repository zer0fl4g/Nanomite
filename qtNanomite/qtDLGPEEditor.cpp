#include "qtDLGPEEditor.h"

#include "clsPEManager.h"
#include "clsHelperClass.h"

qtDLGPEEditor::qtDLGPEEditor(clsPEManager *PEManager,QWidget *parent, Qt::WFlags flags, int PID)
{
	setupUi(this);
	
	this->setStyleSheet(clsHelperClass::LoadStyleSheet());
	this->setAttribute(Qt::WA_DeleteOnClose,true);
	this->setFixedSize(this->width(),this->height());

	_PID = PID;
	_PEManager = PEManager;
	
	if(_PEManager != NULL)
	{	
		_currentFile = _PEManager->getFilenameFromPID(_PID);
		this->setWindowTitle(QString("[Nanomite] - PEEditor - PID: %1 - File: %2").arg(PID,8,16,QChar('0')).arg(QString().fromStdWString(_currentFile)));
		
		InitList();
		LoadPEView();
	}
	else
	{
		MessageBoxW(NULL,L"Could not load File!",L"Nanomite",MB_OK);
		close();
	}
}

qtDLGPEEditor::~qtDLGPEEditor()
{
	_PEManager = NULL;
}

void qtDLGPEEditor::InitList()
{
	// Imports
	treeImport->header()->resizeSection(0,135);
	treeImport->header()->resizeSection(1,135);

	// List Import
	//tblImports->horizontalHeader()->resizeSection(0,135);
	//tblImports->horizontalHeader()->resizeSection(1,135);

	// List Section
	tblSections->horizontalHeader()->resizeSection(0,85);

	// List DOS
	tblDOS->horizontalHeader()->resizeSection(0,135);
	tblDOS->horizontalHeader()->resizeSection(1,135);

	// List FILE
	tblFH->horizontalHeader()->resizeSection(0,135);
	tblFH->horizontalHeader()->resizeSection(1,250);

	// List OPTIONAL
	tblOH->horizontalHeader()->resizeSection(0,125);
	tblOH->horizontalHeader()->resizeSection(0,125);
}

void qtDLGPEEditor::LoadPEView()
{
	InsertImports();
}

void qtDLGPEEditor::InsertImports()
{
	QTreeWidgetItem* topElement = new QTreeWidgetItem();
	QList<ImportAPI> imports = _PEManager->getImports(_currentFile);
	QString lastTopElement;

	for(int importCount = 0; importCount < imports.size(); importCount++)
	{
		QStringList currentElement = imports.value(importCount).APIName.split("::");

		if(currentElement[0].compare(lastTopElement) != NULL)
		{	
			topElement = new QTreeWidgetItem();
			topElement->setText(0,currentElement[0]);
			treeImport->addTopLevelItem(topElement);	  
			lastTopElement = currentElement[0];
		}
		
		QTreeWidgetItem* childElement = new QTreeWidgetItem(topElement);
		childElement->setText(1,currentElement[1]);
		childElement->setText(2,QString("%1").arg(imports.value(importCount).APIOffset,16,16,QChar('0')));
	}
}