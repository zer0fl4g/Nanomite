#include "qtDLGPEEditor.h"

#include "clsMemManager.h"
#include "clsPEManager.h"
#include "clsHelperClass.h"

qtDLGPEEditor::qtDLGPEEditor(clsPEManager *PEManager,QWidget *parent, Qt::WFlags flags, int PID, std::wstring FileName)
{
	setupUi(this);
	
	this->setStyleSheet(clsHelperClass::LoadStyleSheet());
	this->setAttribute(Qt::WA_DeleteOnClose,true);
	this->setFixedSize(this->width(),this->height());

	_PID = PID;
	_PEManager = PEManager;
	
	if(_PEManager != NULL)
	{	
		if(FileName.length() > 0)
			_currentFile = FileName;
		else
			_currentFile = _PEManager->getFilenameFromPID(_PID);
		if(_currentFile.length() <= 0) 
		{
			MessageBoxW(NULL,L"Could not load File!",L"Nanomite",MB_OK);
			close();
		}

		if(_PID == -1)
			this->setWindowTitle(QString("[Nanomite] - PEEditor - File: %1").arg(QString().fromStdWString(_currentFile)));
		else
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
	if(_PID == -1)
		_PEManager->CloseFile(_currentFile,-1);
	_PEManager = NULL;
}

void qtDLGPEEditor::InitList()
{
	// Imports
	treeImport->header()->resizeSection(0,250);
	treeImport->header()->resizeSection(1,135);

	// Exports
	treeExports->header()->resizeSection(0,250);
	treeExports->header()->resizeSection(1,135);

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
	InsertExports();
	//InsertSections();
	InsertDosHeader();
	InsertNTHeader();
}

void qtDLGPEEditor::InsertImports()
{
	QTreeWidgetItem* topElement = new QTreeWidgetItem();
	QList<APIData> imports = _PEManager->getImports(_currentFile);
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
		childElement->setText(0,currentElement[1]);
		childElement->setText(1,QString("%1").arg(imports.value(importCount).APIOffset,16,16,QChar('0')));
	}
}

void qtDLGPEEditor::InsertExports()
{
	QTreeWidgetItem* topElement = new QTreeWidgetItem();
	QList<APIData> exports = _PEManager->getExports(_currentFile);
	QString lastTopElement;

	for(int exportsCount = 0; exportsCount < exports.size(); exportsCount++)
	{		
		topElement = new QTreeWidgetItem();
		topElement->setText(0,exports.at(exportsCount).APIName);
		topElement->setText(1,QString("%1").arg(exports.value(exportsCount).APIOffset,16,16,QChar('0')));
		treeExports->addTopLevelItem(topElement);	  
	}
}

void qtDLGPEEditor::InsertDosHeader()
{
		
}

void qtDLGPEEditor::InsertNTHeader()
{

}