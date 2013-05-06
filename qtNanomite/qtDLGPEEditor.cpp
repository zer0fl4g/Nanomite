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
			this->loadedFile->setText(QString(" File: %1").arg(QString().fromStdWString(_currentFile)));
		else
		{
			this->loadedFile->setText(QString(" File: %1").arg(QString().fromStdWString(_currentFile)));
			this->setWindowTitle(QString("[Nanomite] - PEEditor - PID: %1").arg(PID,8,16,QChar('0')));
		}

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
	treeImport->header()->resizeSection(0,220);
	// Exports
	treeExports->header()->resizeSection(0,220);
	// List Section
	tblSections->horizontalHeader()->resizeSection(0,85);
	// List DOS
	tblDOS->horizontalHeader()->resizeSection(0,150);
	// List FILE
	tblFH->horizontalHeader()->resizeSection(0,150);
	// List OPTIONAL
	tblOH->horizontalHeader()->resizeSection(0,220);
}

void qtDLGPEEditor::LoadPEView()
{
	InsertImports();
	InsertExports();
	InsertSections();
	InsertDosHeader();
	InsertFileHeader();
	InsertOptionalHeader();
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
	PIMAGE_DOS_HEADER currentDOS = _PEManager->getDosHeader(_currentFile);
	if(currentDOS == NULL) return;

	InsertHeaderData(tblDOS,"e_cblp",currentDOS->e_cblp);
	InsertHeaderData(tblDOS,"e_cp",currentDOS->e_cp);
	InsertHeaderData(tblDOS,"e_cparhdr",currentDOS->e_cparhdr);
	InsertHeaderData(tblDOS,"e_crlc",currentDOS->e_crlc);
	InsertHeaderData(tblDOS,"e_cs",currentDOS->e_cs);
	InsertHeaderData(tblDOS,"e_csum",currentDOS->e_csum);
	InsertHeaderData(tblDOS,"e_ip",currentDOS->e_ip);
	InsertHeaderData(tblDOS,"e_lfanew",currentDOS->e_lfanew);
	InsertHeaderData(tblDOS,"e_lfarlc",currentDOS->e_lfarlc);
	InsertHeaderData(tblDOS,"e_magic",currentDOS->e_magic);
	InsertHeaderData(tblDOS,"e_maxalloc",currentDOS->e_maxalloc);
	InsertHeaderData(tblDOS,"e_minalloc",currentDOS->e_minalloc);
	InsertHeaderData(tblDOS,"e_oemid",currentDOS->e_oemid);
	InsertHeaderData(tblDOS,"e_oeminfo",currentDOS->e_oeminfo);
	InsertHeaderData(tblDOS,"e_ovno",currentDOS->e_ovno);
	InsertHeaderData(tblDOS,"e_res",(quint64)currentDOS->e_res);
	InsertHeaderData(tblDOS,"e_res2",(quint64)currentDOS->e_res2);
	InsertHeaderData(tblDOS,"e_sp",currentDOS->e_sp);
	InsertHeaderData(tblDOS,"e_ss",currentDOS->e_ss);
}

void qtDLGPEEditor::InsertFileHeader()
{
	if(_PEManager->is64BitFile(_currentFile))
	{
		PIMAGE_NT_HEADERS64 currentFileHeader = _PEManager->getNTHeader64(_currentFile);
		if(currentFileHeader == NULL) return;
	
		InsertHeaderData(tblFH,"Characteristics",currentFileHeader->FileHeader.Characteristics);
		InsertHeaderData(tblFH,"Machine",currentFileHeader->FileHeader.Machine);
		InsertHeaderData(tblFH,"NumberOfSections",currentFileHeader->FileHeader.NumberOfSections);
		InsertHeaderData(tblFH,"NumberOfSymbols",currentFileHeader->FileHeader.NumberOfSymbols);
		InsertHeaderData(tblFH,"PointerToSymbolTable",currentFileHeader->FileHeader.PointerToSymbolTable);
		InsertHeaderData(tblFH,"SizeOfOptionalHeader",currentFileHeader->FileHeader.SizeOfOptionalHeader);
		InsertHeaderData(tblFH,"TimeDateStamp",currentFileHeader->FileHeader.TimeDateStamp);
	}
	else
	{
		PIMAGE_NT_HEADERS32 currentFileHeader = _PEManager->getNTHeader32(_currentFile);
		if(currentFileHeader == NULL) return;

		InsertHeaderData(tblFH,"Characteristics",currentFileHeader->FileHeader.Characteristics);
		InsertHeaderData(tblFH,"Machine",currentFileHeader->FileHeader.Machine);
		InsertHeaderData(tblFH,"NumberOfSections",currentFileHeader->FileHeader.NumberOfSections);
		InsertHeaderData(tblFH,"NumberOfSymbols",currentFileHeader->FileHeader.NumberOfSymbols);
		InsertHeaderData(tblFH,"PointerToSymbolTable",currentFileHeader->FileHeader.PointerToSymbolTable);
		InsertHeaderData(tblFH,"SizeOfOptionalHeader",currentFileHeader->FileHeader.SizeOfOptionalHeader);
		InsertHeaderData(tblFH,"TimeDateStamp",currentFileHeader->FileHeader.TimeDateStamp);
	}
}

void qtDLGPEEditor::InsertOptionalHeader()
{
	if(_PEManager->is64BitFile(_currentFile))
	{
		PIMAGE_NT_HEADERS64 currentFileHeader = _PEManager->getNTHeader64(_currentFile);
		if(currentFileHeader == NULL) return;

		InsertHeaderData(tblOH,"AddressOfEntryPoint",currentFileHeader->OptionalHeader.AddressOfEntryPoint);
		InsertHeaderData(tblOH,"BaseOfCode",currentFileHeader->OptionalHeader.BaseOfCode);
		InsertHeaderData(tblOH,"CheckSum",currentFileHeader->OptionalHeader.CheckSum);
		//InsertHeaderData(tblOH,"DataDirectory",currentFileHeader->OptionalHeader.DataDirectory);
		InsertHeaderData(tblOH,"DllCharacteristics",currentFileHeader->OptionalHeader.DllCharacteristics);
		InsertHeaderData(tblOH,"FileAlignment",currentFileHeader->OptionalHeader.FileAlignment);
		InsertHeaderData(tblOH,"ImageBase",currentFileHeader->OptionalHeader.ImageBase);
		InsertHeaderData(tblOH,"LoaderFlags",currentFileHeader->OptionalHeader.LoaderFlags);
		InsertHeaderData(tblOH,"Magic",currentFileHeader->OptionalHeader.Magic);
		InsertHeaderData(tblOH,"MajorImageVersion",currentFileHeader->OptionalHeader.MajorImageVersion);
		InsertHeaderData(tblOH,"MajorLinkerVersion",currentFileHeader->OptionalHeader.MajorLinkerVersion);
		InsertHeaderData(tblOH,"MajorOperatingSystemVersion",currentFileHeader->OptionalHeader.MajorOperatingSystemVersion);
		InsertHeaderData(tblOH,"MajorSubsystemVersion",currentFileHeader->OptionalHeader.MajorSubsystemVersion);
		InsertHeaderData(tblOH,"MinorImageVersion",currentFileHeader->OptionalHeader.MinorImageVersion);
		InsertHeaderData(tblOH,"MinorLinkerVersion",currentFileHeader->OptionalHeader.MinorLinkerVersion);
		InsertHeaderData(tblOH,"MinorOperatingSystemVersion",currentFileHeader->OptionalHeader.MinorOperatingSystemVersion);
		InsertHeaderData(tblOH,"MinorSubsystemVersion",currentFileHeader->OptionalHeader.MinorSubsystemVersion);
		InsertHeaderData(tblOH,"NumberOfRvaAndSizes",currentFileHeader->OptionalHeader.NumberOfRvaAndSizes);
		InsertHeaderData(tblOH,"SectionAlignment",currentFileHeader->OptionalHeader.SectionAlignment);
		InsertHeaderData(tblOH,"SizeOfCode",currentFileHeader->OptionalHeader.SizeOfCode);
		InsertHeaderData(tblOH,"SizeOfHeaders",currentFileHeader->OptionalHeader.SizeOfHeaders);
		InsertHeaderData(tblOH,"SizeOfHeapCommit",currentFileHeader->OptionalHeader.SizeOfHeapCommit);
		InsertHeaderData(tblOH,"SizeOfHeapReserve",currentFileHeader->OptionalHeader.SizeOfHeapReserve);
		InsertHeaderData(tblOH,"SizeOfImage",currentFileHeader->OptionalHeader.SizeOfImage);
		InsertHeaderData(tblOH,"SizeOfInitializedData",currentFileHeader->OptionalHeader.SizeOfInitializedData);
		InsertHeaderData(tblOH,"SizeOfStackCommit",currentFileHeader->OptionalHeader.SizeOfStackCommit);
		InsertHeaderData(tblOH,"SizeOfStackReserve",currentFileHeader->OptionalHeader.SizeOfStackReserve);
		InsertHeaderData(tblOH,"SizeOfUninitializedData",currentFileHeader->OptionalHeader.SizeOfUninitializedData);
		InsertHeaderData(tblOH,"Subsystem",currentFileHeader->OptionalHeader.Subsystem);
		InsertHeaderData(tblOH,"Win32VersionValue",currentFileHeader->OptionalHeader.Win32VersionValue);
	}
	else
	{
		PIMAGE_NT_HEADERS32 currentFileHeader = _PEManager->getNTHeader32(_currentFile);
		if(currentFileHeader == NULL) return;

		InsertHeaderData(tblOH,"AddressOfEntryPoint",currentFileHeader->OptionalHeader.AddressOfEntryPoint);
		InsertHeaderData(tblOH,"BaseOfCode",currentFileHeader->OptionalHeader.BaseOfCode);
		InsertHeaderData(tblOH,"CheckSum",currentFileHeader->OptionalHeader.CheckSum);
		//InsertHeaderData(tblOH,"DataDirectory",currentFileHeader->OptionalHeader.DataDirectory);
		InsertHeaderData(tblOH,"DllCharacteristics",currentFileHeader->OptionalHeader.DllCharacteristics);
		InsertHeaderData(tblOH,"FileAlignment",currentFileHeader->OptionalHeader.FileAlignment);
		InsertHeaderData(tblOH,"ImageBase",currentFileHeader->OptionalHeader.ImageBase);
		InsertHeaderData(tblOH,"LoaderFlags",currentFileHeader->OptionalHeader.LoaderFlags);
		InsertHeaderData(tblOH,"Magic",currentFileHeader->OptionalHeader.Magic);
		InsertHeaderData(tblOH,"MajorImageVersion",currentFileHeader->OptionalHeader.MajorImageVersion);
		InsertHeaderData(tblOH,"MajorLinkerVersion",currentFileHeader->OptionalHeader.MajorLinkerVersion);
		InsertHeaderData(tblOH,"MajorOperatingSystemVersion",currentFileHeader->OptionalHeader.MajorOperatingSystemVersion);
		InsertHeaderData(tblOH,"MajorSubsystemVersion",currentFileHeader->OptionalHeader.MajorSubsystemVersion);
		InsertHeaderData(tblOH,"MinorImageVersion",currentFileHeader->OptionalHeader.MinorImageVersion);
		InsertHeaderData(tblOH,"MinorLinkerVersion",currentFileHeader->OptionalHeader.MinorLinkerVersion);
		InsertHeaderData(tblOH,"MinorOperatingSystemVersion",currentFileHeader->OptionalHeader.MinorOperatingSystemVersion);
		InsertHeaderData(tblOH,"MinorSubsystemVersion",currentFileHeader->OptionalHeader.MinorSubsystemVersion);
		InsertHeaderData(tblOH,"NumberOfRvaAndSizes",currentFileHeader->OptionalHeader.NumberOfRvaAndSizes);
		InsertHeaderData(tblOH,"SectionAlignment",currentFileHeader->OptionalHeader.SectionAlignment);
		InsertHeaderData(tblOH,"SizeOfCode",currentFileHeader->OptionalHeader.SizeOfCode);
		InsertHeaderData(tblOH,"SizeOfHeaders",currentFileHeader->OptionalHeader.SizeOfHeaders);
		InsertHeaderData(tblOH,"SizeOfHeapCommit",currentFileHeader->OptionalHeader.SizeOfHeapCommit);
		InsertHeaderData(tblOH,"SizeOfHeapReserve",currentFileHeader->OptionalHeader.SizeOfHeapReserve);
		InsertHeaderData(tblOH,"SizeOfImage",currentFileHeader->OptionalHeader.SizeOfImage);
		InsertHeaderData(tblOH,"SizeOfInitializedData",currentFileHeader->OptionalHeader.SizeOfInitializedData);
		InsertHeaderData(tblOH,"SizeOfStackCommit",currentFileHeader->OptionalHeader.SizeOfStackCommit);
		InsertHeaderData(tblOH,"SizeOfStackReserve",currentFileHeader->OptionalHeader.SizeOfStackReserve);
		InsertHeaderData(tblOH,"SizeOfUninitializedData",currentFileHeader->OptionalHeader.SizeOfUninitializedData);
		InsertHeaderData(tblOH,"Subsystem",currentFileHeader->OptionalHeader.Subsystem);
		InsertHeaderData(tblOH,"Win32VersionValue",currentFileHeader->OptionalHeader.Win32VersionValue);
	}
}

void qtDLGPEEditor::InsertHeaderData(QTableWidget* tblHeaderTable,QString ValueName,quint64 dwValue)
{
	tblHeaderTable->insertRow(tblHeaderTable->rowCount());
	tblHeaderTable->setItem(tblHeaderTable->rowCount() - 1,0,
		new QTableWidgetItem(ValueName));
	tblHeaderTable->setItem(tblHeaderTable->rowCount() - 1,1,
		new QTableWidgetItem(QString("%1").arg(dwValue,16,16,QChar('0'))));
}

void qtDLGPEEditor::InsertSections()
{
	QList<PESectionData> sections = _PEManager->getSections(_currentFile);
	if(sections.size() <= 0) return;

	for(int i = 0; i < sections.size(); i++)
	{
		tblSections->insertRow(tblSections->rowCount());
		tblSections->setItem(tblSections->rowCount() - 1,0,
			new QTableWidgetItem(sections.at(i).SectionName));
		tblSections->setItem(tblSections->rowCount() - 1,1,
			new QTableWidgetItem(QString("%1").arg(sections.at(i).VirtualAddress,8,16,QChar('0'))));
		tblSections->setItem(tblSections->rowCount() - 1,2,
			new QTableWidgetItem(QString("%1").arg(sections.at(i).VirtualSize,8,16,QChar('0'))));
		tblSections->setItem(tblSections->rowCount() - 1,3,
			new QTableWidgetItem(QString("%1").arg(sections.at(i).PointerToRawData,8,16,QChar('0'))));
		tblSections->setItem(tblSections->rowCount() - 1,4,
			new QTableWidgetItem(QString("%1").arg(sections.at(i).SizeOfRawData,8,16,QChar('0'))));
		tblSections->setItem(tblSections->rowCount() - 1,5,
			new QTableWidgetItem(QString("%1").arg(sections.at(i).Characteristics,8,16,QChar('0'))));
	}
}