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
#include "qtDLGPatchManager.h"
#include "qtDLGNanomite.h"

#include "clsMemManager.h"
#include "clsHelperClass.h"
#include "clsMemoryProtector.h"

#include <Psapi.h>

#include <QMenu>
#include <QClipboard>
#include <QMap>

qtDLGPatchManager *qtDLGPatchManager::pThis = NULL;

qtDLGPatchManager::qtDLGPatchManager(QWidget *parent, Qt::WFlags flags)
	: QWidget(parent, flags)
{
	setupUi(this);
	pThis = this;
	this->setLayout(verticalLayout);

	connect(tblPatches,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnCustomContextMenuRequested(QPoint)));
	connect(new QShortcut(Qt::Key_Escape,this),SIGNAL(activated()),this,SLOT(close()));
	connect(new QShortcut(QKeySequence(QKeySequence::Delete),this),SIGNAL(activated()),this,SLOT(OnPatchRemove()));
	connect(new QShortcut(QKeySequence::InsertParagraphSeparator,this),SIGNAL(activated()),this,SLOT(OnReturnPressed()));
	connect(tblPatches,SIGNAL(itemDoubleClicked(QTableWidgetItem *)),this,SLOT(OnSendToDisassembler(QTableWidgetItem *)));

	//Init List
	tblPatches->horizontalHeader()->resizeSection(0,75);
	tblPatches->horizontalHeader()->resizeSection(1,135);
	tblPatches->horizontalHeader()->resizeSection(2,135);
	tblPatches->horizontalHeader()->resizeSection(3,135);
	tblPatches->horizontalHeader()->resizeSection(4,50);
	tblPatches->horizontalHeader()->resizeSection(5,50);
	tblPatches->horizontalHeader()->setFixedHeight(21);
}

qtDLGPatchManager::~qtDLGPatchManager()
{
	ClearAllPatches();
	pThis = NULL;
}

void qtDLGPatchManager::OnCustomContextMenuRequested(QPoint qPoint)
{
	QMenu menu;

	m_iSelectedRow = tblPatches->indexAt(qPoint).row();
	if(m_iSelectedRow < 0) return;

	menu.addAction(new QAction("(Un)Do Patch",this));
	menu.addAction(new QAction("Remove Patch",this));
	menu.addAction(new QAction("Remove All Patches",this));
	menu.addAction(new QAction("Save Patch to File",this));
	menu.addAction(new QAction("Save All Patches to File",this));
	menu.addAction(new QAction("Send to Disassembler",this));
	menu.addSeparator();
	QMenu *submenu = menu.addMenu("Copy to Clipboard");
	submenu->addAction(new QAction("Line",this));
	submenu->addAction(new QAction("Offset",this));
	submenu->addAction(new QAction("Org. Bytes",this));
	submenu->addAction(new QAction("New Bytes",this));
	submenu->addAction(new QAction("Size",this));
	submenu->addAction(new QAction("Saved",this));
	submenu->addAction(new QAction("Written",this));

	menu.addMenu(submenu);
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));

	menu.exec(QCursor::pos());
}

void qtDLGPatchManager::MenuCallback(QAction* pAction)
{
	if(QString().compare(pAction->text(),"(Un)Do Patch") == 0)
	{
		if(tblPatches->item(m_iSelectedRow,6)->text().contains("false"))
			AddNewPatch(NULL,NULL,tblPatches->item(m_iSelectedRow,1)->text().toULongLong(0,16),NULL,NULL,true);
		else
			RemovePatch(tblPatches->item(m_iSelectedRow,0)->text().toULongLong(0,16),
			tblPatches->item(m_iSelectedRow,1)->text().toULongLong(0,16));

		qtDLGNanomite::GetInstance()->coreDisAs->InsertNewDisassembly(qtDLGNanomite::GetInstance()->coreDebugger->GetCurrentProcessHandle(),
			tblPatches->item(m_iSelectedRow,1)->text().toULongLong(0,16),
			true);

		UpdatePatchTable();
	}
	else if(QString().compare(pAction->text(),"Remove Patch") == 0)
	{
		RemovePatch(tblPatches->item(m_iSelectedRow,0)->text().toULongLong(0,16),
			tblPatches->item(m_iSelectedRow,1)->text().toULongLong(0,16));
		DeletePatch(tblPatches->item(m_iSelectedRow,0)->text().toULongLong(0,16),
			tblPatches->item(m_iSelectedRow,1)->text().toULongLong(0,16));

		qtDLGNanomite::GetInstance()->coreDisAs->InsertNewDisassembly(qtDLGNanomite::GetInstance()->coreDebugger->GetCurrentProcessHandle(),
			tblPatches->item(m_iSelectedRow,1)->text().toULongLong(0,16),
			true);

		UpdatePatchTable();
	}
	else if(QString().compare(pAction->text(),"Remove All Patches") == 0)
	{
		ClearAllPatches();
		qtDLGNanomite::GetInstance()->coreDisAs->SectionDisAs.clear();
		emit OnReloadDebugger();
		UpdatePatchTable();
	}
	else if(QString().compare(pAction->text(),"Save Patch to File") == 0)
	{
		SavePatch(tblPatches->item(m_iSelectedRow,1)->text().toULongLong(0,16));
		UpdatePatchTable();
	}
	else if(QString().compare(pAction->text(),"Save All Patches to File") == 0)
	{
		SavePatch(NULL,true);
		UpdatePatchTable();
	}
	else if(QString().compare(pAction->text(),"Send to Disassembler") == 0)
	{
		qtDLGNanomite::GetInstance()->coreDisAs->InsertNewDisassembly(qtDLGNanomite::GetInstance()->coreDebugger->GetCurrentProcessHandle(),
			tblPatches->item(m_iSelectedRow,1)->text().toULongLong(0,16),
			true);
	}
	else if(QString().compare(pAction->text(),"Line") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(QString("%1:%2:%3:%4:%5:%6:%7")
			.arg(tblPatches->item(m_iSelectedRow,0)->text())
			.arg(tblPatches->item(m_iSelectedRow,1)->text())
			.arg(tblPatches->item(m_iSelectedRow,2)->text())
			.arg(tblPatches->item(m_iSelectedRow,3)->text())
			.arg(tblPatches->item(m_iSelectedRow,4)->text())
			.arg(tblPatches->item(m_iSelectedRow,5)->text())
			.arg(tblPatches->item(m_iSelectedRow,6)->text()));
	}
	else if(QString().compare(pAction->text(),"Offset") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblPatches->item(m_iSelectedRow,1)->text());
	}
	else if(QString().compare(pAction->text(),"Org. Bytes") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblPatches->item(m_iSelectedRow,2)->text());
	}
	else if(QString().compare(pAction->text(),"New Bytes") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblPatches->item(m_iSelectedRow,3)->text());
	}
	else if(QString().compare(pAction->text(),"Size") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblPatches->item(m_iSelectedRow,4)->text());
	}
	else if(QString().compare(pAction->text(),"Saved") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblPatches->item(m_iSelectedRow,5)->text());
	}
	else if(QString().compare(pAction->text(),"Written") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblPatches->item(m_iSelectedRow,5)->text());
	}
}

bool qtDLGPatchManager::AddNewPatch(int PID, HANDLE hProc, quint64 Offset, int PatchSize, LPVOID newData, bool reWrite)
{
	if(pThis == NULL) return false;

	for(QList<PatchData>::iterator i = pThis->m_patches.begin(); i != pThis->m_patches.end(); ++i)
	{
		if(i->Offset == Offset)
		{
			if(reWrite)
			{
				pThis->WritePatchToProc(i->hProc,i->Offset,i->PatchSize,i->newData,i->orgData);
				i->bWritten = true;
				
				return true;
			}
			return false;
		}
	}
	
	PatchData newPatch = { 0 };
	newPatch.hProc = hProc;
	newPatch.Offset = Offset;
	newPatch.PID = PID;
	newPatch.PatchSize = PatchSize;
	newPatch.ModuleName = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));
	newPatch.processModule = qtDLGNanomite::GetInstance()->PEManager->getFilenameFromPID(PID);
	newPatch.newData = clsMemManager::CAlloc(PatchSize);
	newPatch.orgData = clsMemManager::CAlloc(PatchSize);
	newPatch.bSaved = false;
	
	memset(newPatch.ModuleName,0,MAX_PATH * sizeof(TCHAR));
	memcpy(newPatch.newData,newData,PatchSize);

	newPatch.BaseOffset = clsHelperClass::CalcOffsetForModule(newPatch.ModuleName, Offset, PID);

	if(pThis->WritePatchToProc(hProc,Offset,PatchSize,newData,newPatch.orgData))
		newPatch.bWritten = true;
	else
		newPatch.bWritten = false;
	
	pThis->m_patches.append(newPatch);
	pThis->UpdateOffsetPatch(hProc,PID);

	
	qtDLGNanomite::GetInstance()->coreDisAs->InsertNewDisassembly(qtDLGNanomite::GetInstance()->coreDebugger->GetCurrentProcessHandle(), Offset, true);

	return true;
}

bool qtDLGPatchManager::RemovePatch(int PID, quint64 Offset)
{
	// Delete Patch from Memory
	if(pThis == NULL) return false;

	for(QList<PatchData>::iterator i = pThis->m_patches.begin(); i != pThis->m_patches.end(); ++i)
	{
		if(i->Offset == Offset)
		{
			if(pThis->WritePatchToProc(i->hProc,i->Offset,i->PatchSize,i->orgData,NULL,true))
			{
				i->bWritten = false;
				return true;
			}
		}
	}
	return false;
}

bool qtDLGPatchManager::DeletePatch(int PID, quint64 Offset)
{
	// Delete Patch from List
	if(pThis == NULL) return false;

	for(QList<PatchData>::iterator i = pThis->m_patches.begin(); i != pThis->m_patches.end(); ++i)
	{
		if(i->Offset == Offset)
		{
			clsMemManager::CFree(i->orgData);
			clsMemManager::CFree(i->newData);
			clsMemManager::CFree(i->ModuleName);

			pThis->m_patches.erase(i);
			return true;
		}
	}
	return false;
}
	
void qtDLGPatchManager::ClearAllPatches()
{
	if(pThis == NULL) return;

	for(QList<PatchData>::iterator i = pThis->m_patches.begin(); i != pThis->m_patches.end(); ++i)
	{
		RemovePatch(i->PID,i->Offset);
		clsMemManager::CFree(i->orgData);
		clsMemManager::CFree(i->newData);
		clsMemManager::CFree(i->ModuleName);
	}

	pThis->m_patches.clear();
}

bool qtDLGPatchManager::WritePatchToProc(HANDLE hProc, quint64 Offset, int PatchSize, LPVOID DataToWrite, LPVOID OrgData, bool bRemove)
{
	bool	worked			= false;

	clsMemoryProtector tempMemoryProtector(hProc, PAGE_READWRITE, PatchSize, Offset, &worked);
	
	if(!bRemove)
	{
		if(!ReadProcessMemory(hProc,(LPVOID)Offset,OrgData,PatchSize,NULL))
		{
			return false;
		}
	}

	if(!WriteProcessMemory(hProc,(LPVOID)Offset,DataToWrite,PatchSize,NULL))
	{
		return false;
	}

	return true;
}

void qtDLGPatchManager::UpdatePatchTable()
{	
	QString strTemp;

	tblPatches->setRowCount(0);
	for(QList<PatchData>::iterator i = m_patches.begin(); i != m_patches.end(); ++i)
	{
		tblPatches->insertRow(tblPatches->rowCount());		

		tblPatches->setItem(tblPatches->rowCount() - 1,0,
			new QTableWidgetItem(QString("%1").arg(i->PID,8,16,QChar('0'))));

		tblPatches->setItem(tblPatches->rowCount() - 1,1,
			new QTableWidgetItem(QString("%1").arg(i->Offset,16,16,QChar('0'))));

		strTemp.clear();
		for(int a = 0; a < i->PatchSize; a++)
		{
			strTemp.append(QString().sprintf("%02X ",*(LPBYTE)((DWORD)i->orgData + a)));
		}
		pThis->tblPatches->setItem(pThis->tblPatches->rowCount() - 1,2,
			new QTableWidgetItem(strTemp));

		strTemp.clear();
		for(int a = 0; a < i->PatchSize; a++)
		{
			strTemp.append(QString().sprintf("%02X ",*(LPBYTE)((DWORD)i->newData + a)));
		}
		pThis->tblPatches->setItem(pThis->tblPatches->rowCount() - 1,3,
			new QTableWidgetItem(strTemp));

		tblPatches->setItem(tblPatches->rowCount() - 1,4,
			new QTableWidgetItem(QString("%1").arg(i->PatchSize,6,16,QChar('0'))));

		if(i->bSaved)
			tblPatches->setItem(tblPatches->rowCount() - 1,5,
				new QTableWidgetItem("true"));
		else
			tblPatches->setItem(tblPatches->rowCount() - 1,5,
				new QTableWidgetItem("false"));

		if(i->bWritten)
			tblPatches->setItem(tblPatches->rowCount() - 1,6,
				new QTableWidgetItem("true"));
		else
			tblPatches->setItem(tblPatches->rowCount() - 1,6,
				new QTableWidgetItem("false"));
	}
}

void qtDLGPatchManager::UpdateOffsetPatch(HANDLE newProc, int newPID)
{
	QString currentPModule = qtDLGNanomite::GetInstance()->PEManager->getFilenameFromPID(newPID);
	for(QList<PatchData>::iterator i = m_patches.begin(); i != m_patches.end(); ++i)
	{
		if(currentPModule.contains(i->processModule, Qt::CaseInsensitive))
		{
			DWORD64 newBaseOffset = clsHelperClass::CalcOffsetForModule(i->ModuleName, NULL, newPID);
			if(newBaseOffset != i->BaseOffset)
			{
				i->Offset = (i->Offset - i->BaseOffset) + newBaseOffset;
				i->BaseOffset = newBaseOffset;
			}

			i->PID = newPID;
			i->hProc = newProc;

			if(WritePatchToProc(i->hProc,i->Offset,i->PatchSize,i->newData,NULL,true))
				i->bWritten = true;
			else
				i->bWritten = false;
		}
	}

	UpdatePatchTable();
}

void qtDLGPatchManager::SavePatch(quint64 Offset, bool saveAll)
{
	QMap<QString,QString> fileMapping;

	for(QList<PatchData>::iterator i = m_patches.begin(); i != m_patches.end(); ++i)
	{
		if(saveAll || i->Offset == Offset)
		{
			PTCHAR pCurrentFilePath = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));

			if(!GetModuleFileNameEx(i->hProc, (HMODULE)i->BaseOffset, pCurrentFilePath, MAX_PATH))
			{
				clsMemManager::CFree(pCurrentFilePath);
				return;
			}

			QString patchFilePath, 
					currentFilePath = QString::fromWCharArray(pCurrentFilePath);
			QMap<QString,QString>::iterator fileMap = fileMapping.find(currentFilePath);

			bool isNewFile = false;

			if(fileMap == fileMapping.end())
			{		
				isNewFile = true;

				if(currentFilePath.contains(".exe"))
				{
					QString tempPath = currentFilePath;
					patchFilePath = QFileDialog::getSaveFileName(this,
						"Please select a place to save the patched executable",
						tempPath.replace(".exe", "_patched.exe"),
						"Executable files (*.exe)",
						NULL,
						QFileDialog::DontUseNativeDialog);
				}
				else
				{
					QString tempPath = currentFilePath;
					patchFilePath = QFileDialog::getSaveFileName(this,
						"Please select a place to save the patched dll",
						tempPath.replace(".dll", "_patched.dll"),
						"Dynamic link library (*.dll)",
						NULL,
						QFileDialog::DontUseNativeDialog);
				}

				fileMapping.insert(currentFilePath, patchFilePath);

				if(currentFilePath.compare(patchFilePath) == 0)
					isNewFile = false;
				else
					CopyFile(currentFilePath.toStdWString().c_str(), patchFilePath.toStdWString().c_str(), true);
			}
			else
				patchFilePath = fileMap.value();

			i->bSaved = SavePatchToFile(patchFilePath, currentFilePath, i);

			if(!i->bSaved && isNewFile)
			{
				fileMapping.remove(currentFilePath);
				DeleteFile(patchFilePath.toStdWString().c_str());
			}

			clsMemManager::CFree(pCurrentFilePath);
		}
	}
}

bool qtDLGPatchManager::SavePatchToFile(QString patchFilePath, QString currentFilePath, QList<PatchData>::iterator currentPatch)
{
	HANDLE hFile = CreateFileW(patchFilePath.toStdWString().c_str(),GENERIC_READ | GENERIC_WRITE,NULL,NULL,OPEN_EXISTING,NULL,NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return false;

	HANDLE hFileMap = CreateFileMapping(hFile,NULL,PAGE_READWRITE,NULL,NULL,NULL);
	LPVOID lpFileBuffer = MapViewOfFile(hFileMap,FILE_MAP_WRITE | FILE_MAP_READ,NULL,NULL,NULL);
	if(lpFileBuffer == NULL)
	{
		UnmapViewOfFile(lpFileBuffer);
		CloseHandle(hFileMap);
		CloseHandle(hFile);

		return false;
	}

	clsPEManager *pPEManager = clsPEManager::GetInstance();
	DWORD64 rawOffset = pPEManager->VAtoRaw(currentFilePath,NULL,currentPatch->Offset - currentPatch->BaseOffset);
	if(rawOffset <= 0)
	{
		pPEManager->OpenFile(currentFilePath);

		rawOffset = pPEManager->VAtoRaw(currentFilePath,NULL,currentPatch->Offset - currentPatch->BaseOffset);
		
		pPEManager->CloseFile(currentFilePath,-1);
	}

	DWORD64 fileDataOffset = (DWORD64)lpFileBuffer + rawOffset;
	if(fileDataOffset <= NULL)
	{
		UnmapViewOfFile(lpFileBuffer);
		CloseHandle(hFileMap);
		CloseHandle(hFile);

		return false;
	}

	if(memcmp((LPVOID)fileDataOffset,currentPatch->orgData,currentPatch->PatchSize) != 0)
	{
		UnmapViewOfFile(lpFileBuffer);
		CloseHandle(hFileMap);
		CloseHandle(hFile);

		return false;
	}

	memcpy((LPVOID)fileDataOffset,currentPatch->newData,currentPatch->PatchSize);

	DWORD BytesWritten = NULL;
	if(!WriteFile(hFile,lpFileBuffer,GetFileSize(hFile,NULL),&BytesWritten,NULL))
	{
		UnmapViewOfFile(lpFileBuffer);
		CloseHandle(hFileMap);
		CloseHandle(hFile);

		return false;
	}

	UnmapViewOfFile(lpFileBuffer);
	CloseHandle(hFileMap);
	CloseHandle(hFile);

	return true;
}

void qtDLGPatchManager::OnPatchRemove()
{
	if(tblPatches->selectedItems().count() <= 0) return;

	QTableWidgetItem *pItem = tblPatches->selectedItems()[0];
	RemovePatch(tblPatches->item(pItem->row(),0)->text().toULongLong(0,16),tblPatches->item(pItem->row(),1)->text().toULongLong(0,16));
	DeletePatch(tblPatches->item(pItem->row(),0)->text().toULongLong(0,16),tblPatches->item(pItem->row(),1)->text().toULongLong(0,16));
	
	qtDLGNanomite::GetInstance()->coreDisAs->InsertNewDisassembly(qtDLGNanomite::GetInstance()->coreDebugger->GetCurrentProcessHandle(),
		tblPatches->item(pItem->row(),1)->text().toULongLong(0,16),
		true);

	UpdatePatchTable();	
}

void qtDLGPatchManager::ResetPatches()
{
	if(pThis == NULL) return;

	for(QList<PatchData>::iterator i = pThis->m_patches.begin(); i != pThis->m_patches.end(); ++i)
	{
		i->bSaved = false;
		i->bWritten = false;
	}

	pThis->UpdatePatchTable();
}

void qtDLGPatchManager::OnSendToDisassembler(QTableWidgetItem *pSelectedRow)
{
	qtDLGNanomite::GetInstance()->coreDisAs->InsertNewDisassembly(qtDLGNanomite::GetInstance()->coreDebugger->GetCurrentProcessHandle(),
		tblPatches->item(pSelectedRow->row(),1)->text().toULongLong(0,16));
}

void qtDLGPatchManager::OnReturnPressed()
{
	if(tblPatches->selectedItems().count() <= 0) return;

	OnSendToDisassembler(tblPatches->selectedItems()[0]);
}

void qtDLGPatchManager::InsertPatchFromProjectFile(PatchData newPatchData)
{
	pThis->m_patches.append(newPatchData);
}

QList<PatchData> qtDLGPatchManager::GetPatchList()
{
	return pThis->m_patches;
}