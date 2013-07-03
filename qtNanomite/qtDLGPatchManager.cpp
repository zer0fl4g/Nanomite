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

#include <Psapi.h>

#include <QMenu>
#include <QClipboard>

qtDLGPatchManager *qtDLGPatchManager::pThis = NULL;

qtDLGPatchManager::qtDLGPatchManager(QWidget *parent, Qt::WFlags flags)
	: QWidget(parent, flags)
{
	setupUi(this);
	pThis = this;
	this->setLayout(verticalLayout);

	connect(tblPatches,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnCustomContextMenuRequested(QPoint)));

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

	_iSelectedRow = tblPatches->indexAt(qPoint).row();
	if(_iSelectedRow < 0) return;

	menu.addAction(new QAction("(Un)Do Patch",this));
	menu.addAction(new QAction("Remove Patch",this));
	menu.addAction(new QAction("Remove All Patches",this));
	menu.addAction(new QAction("Save Patch to File",this));
	menu.addAction(new QAction("Save All Patches to File",this));
	menu.addAction(new QAction("Send to Disassembler",this));
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
		if(tblPatches->item(_iSelectedRow,6)->text().contains("false"))
			AddNewPatch(NULL,NULL,tblPatches->item(_iSelectedRow,1)->text().toULongLong(0,16),NULL,NULL,true);
		else
			RemovePatch(tblPatches->item(_iSelectedRow,0)->text().toULongLong(0,16),
			tblPatches->item(_iSelectedRow,1)->text().toULongLong(0,16));

		qtDLGNanomite::GetInstance()->coreDisAs->SectionDisAs.clear();
		emit pThis->OnReloadDebugger();
		UpdatePatchTable();
	}
	else if(QString().compare(pAction->text(),"Remove Patch") == 0)
	{
		RemovePatch(tblPatches->item(_iSelectedRow,0)->text().toULongLong(0,16),
			tblPatches->item(_iSelectedRow,1)->text().toULongLong(0,16));
		DeletePatch(tblPatches->item(_iSelectedRow,0)->text().toULongLong(0,16),
			tblPatches->item(_iSelectedRow,1)->text().toULongLong(0,16));

		qtDLGNanomite::GetInstance()->coreDisAs->SectionDisAs.clear();
		emit pThis->OnReloadDebugger();
		UpdatePatchTable();
	}
	else if(QString().compare(pAction->text(),"Remove All Patches") == 0)
	{
		ClearAllPatches();
		qtDLGNanomite::GetInstance()->coreDisAs->SectionDisAs.clear();
		emit pThis->OnReloadDebugger();
		UpdatePatchTable();
	}
	else if(QString().compare(pAction->text(),"Save Patch to File") == 0)
	{
		SavePatchToFile(tblPatches->item(_iSelectedRow,0)->text().toULongLong(0,16),
			tblPatches->item(_iSelectedRow,1)->text().toULongLong(0,16));

		UpdatePatchTable();
	}
	else if(QString().compare(pAction->text(),"Save All Patches to File") == 0)
	{
		for(int i = 0; i < tblPatches->rowCount(); i++)
		{
			SavePatchToFile(tblPatches->item(i,0)->text().toULongLong(0,16),
			tblPatches->item(i,1)->text().toULongLong(0,16));
		}
		UpdatePatchTable();
	}
	else if(QString().compare(pAction->text(),"Send to Disassembler") == 0)
	{
		emit OnShowInDisassembler(tblPatches->item(_iSelectedRow,1)->text().toULongLong(0,16));
	}
	else if(QString().compare(pAction->text(),"Line") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(QString("%1:%2:%3:%4:%5:%6:%7")
			.arg(tblPatches->item(_iSelectedRow,0)->text())
			.arg(tblPatches->item(_iSelectedRow,1)->text())
			.arg(tblPatches->item(_iSelectedRow,2)->text())
			.arg(tblPatches->item(_iSelectedRow,3)->text())
			.arg(tblPatches->item(_iSelectedRow,4)->text())
			.arg(tblPatches->item(_iSelectedRow,5)->text())
			.arg(tblPatches->item(_iSelectedRow,6)->text()));
	}
	else if(QString().compare(pAction->text(),"Offset") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblPatches->item(_iSelectedRow,1)->text());
	}
	else if(QString().compare(pAction->text(),"Org. Bytes") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblPatches->item(_iSelectedRow,2)->text());
	}
	else if(QString().compare(pAction->text(),"New Bytes") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblPatches->item(_iSelectedRow,3)->text());
	}
	else if(QString().compare(pAction->text(),"Size") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblPatches->item(_iSelectedRow,4)->text());
	}
	else if(QString().compare(pAction->text(),"Saved") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblPatches->item(_iSelectedRow,5)->text());
	}
	else if(QString().compare(pAction->text(),"Written") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblPatches->item(_iSelectedRow,5)->text());
	}
}

bool qtDLGPatchManager::AddNewPatch(int PID, HANDLE hProc, quint64 Offset, int PatchSize, LPVOID newData, bool reWrite)
{
	if(pThis == NULL) return false;

	for(QList<PatchData>::iterator i = pThis->patches.begin(); i != pThis->patches.end(); ++i)
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
	
	PatchData newPatch;
	memset((LPVOID)&newPatch,0,sizeof(PatchData));
	newPatch.hProc = hProc;
	newPatch.Offset = Offset;
	newPatch.PID = PID;
	newPatch.PatchSize = PatchSize;
	newPatch.ModuleName = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));
	newPatch.newData = clsMemManager::CAlloc(PatchSize);
	newPatch.orgData = clsMemManager::CAlloc(PatchSize);
	newPatch.bSaved = false;

	memset(newPatch.ModuleName,0,MAX_PATH * sizeof(TCHAR));
	memcpy(newPatch.newData,newData,PatchSize);

	if(pThis->WritePatchToProc(hProc,Offset,PatchSize,newData,newPatch.orgData))
		newPatch.bWritten = true;
	else
		newPatch.bWritten = false;
	
	pThis->patches.push_back(newPatch);
	pThis->UpdateOffsetPatch(hProc,PID);

	return true;
}

bool qtDLGPatchManager::RemovePatch(int PID, quint64 Offset)
{
	// Delete Patch from Memory
	if(pThis == NULL) return false;

	for(QList<PatchData>::iterator i = pThis->patches.begin(); i != pThis->patches.end(); ++i)
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

	for(QList<PatchData>::iterator i = pThis->patches.begin(); i != pThis->patches.end(); ++i)
	{
		if(i->Offset == Offset)
		{
			clsMemManager::CFree(i->orgData);
			clsMemManager::CFree(i->newData);
			clsMemManager::CFree(i->ModuleName);

			pThis->patches.erase(i);
			return true;
		}
	}
	return false;
}
	
void qtDLGPatchManager::ClearAllPatches()
{
	if(pThis == NULL) return;

	for(QList<PatchData>::iterator i = pThis->patches.begin(); i != pThis->patches.end(); ++i)
	{
		RemovePatch(i->PID,i->Offset);
		clsMemManager::CFree(i->orgData);
		clsMemManager::CFree(i->newData);
		clsMemManager::CFree(i->ModuleName);
	}

	pThis->patches.clear();
}

bool qtDLGPatchManager::WritePatchToProc(HANDLE hProc, quint64 Offset, int PatchSize, LPVOID DataToWrite, LPVOID OrgData, bool bRemove)
{
	DWORD 	NewProtection = PAGE_READWRITE,
			OldProtection = NULL;

	VirtualProtectEx(hProc,(LPVOID)Offset,PatchSize,NewProtection,&OldProtection);
	
	if(!bRemove)
	{
		if(!ReadProcessMemory(hProc,(LPVOID)Offset,OrgData,PatchSize,NULL))
		{
			VirtualProtectEx(hProc,(LPVOID)Offset,PatchSize,OldProtection,&NewProtection);
			return false;
		}
	}

	if(!WriteProcessMemory(hProc,(LPVOID)Offset,DataToWrite,PatchSize,NULL))
	{
		VirtualProtectEx(hProc,(LPVOID)Offset,PatchSize,OldProtection,&NewProtection);
		return false;
	}

	VirtualProtectEx(hProc,(LPVOID)Offset,PatchSize,OldProtection,&NewProtection);
	return true;
}

void qtDLGPatchManager::UpdatePatchTable()
{	
	QString strTemp;

	tblPatches->setRowCount(0);
	for(QList<PatchData>::iterator i = patches.begin(); i != patches.end(); ++i)
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
	bool bThingsChanged = false;

	for(QList<PatchData>::iterator i = patches.begin(); i != patches.end(); ++i)
	{
		DWORD64 newBaseOffset = clsHelperClass::CalcOffsetForModule(i->ModuleName,i->Offset,newPID);
		if(newBaseOffset != i->Offset && newBaseOffset != i->BaseOffset)
		{
			if(i->BaseOffset == NULL)
			{
				i->OldBaseOffset = i->Offset;
				i->BaseOffset = newBaseOffset;
			}
			else
			{
				DWORD64 newOffset = (i->Offset - i->BaseOffset);

				i->OldBaseOffset = i->Offset;
				i->BaseOffset = newBaseOffset;
				i->Offset = newOffset + newBaseOffset;
			}

			i->PID = newPID;
			i->hProc = newProc;
			bThingsChanged = WritePatchToProc(i->hProc,i->Offset,i->PatchSize,i->newData,NULL,true);
			if(bThingsChanged)
				i->bWritten = true;
			else
				i->bWritten = false;
		}
	}

	if(bThingsChanged)
	{
		qtDLGNanomite::GetInstance()->coreDisAs->SectionDisAs.clear();
		emit pThis->OnReloadDebugger();
	}
	UpdatePatchTable();
}

void qtDLGPatchManager::SavePatchToFile(int PID, quint64 Offset)
{
	for(QList<PatchData>::iterator i = patches.begin(); i != patches.end(); ++i)
	{
		if(i->Offset == Offset)
		{
			PTCHAR	pCurrentFileName = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR)),
					pNewFileName = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));

			if(GetModuleFileNameEx(i->hProc,(HMODULE)i->BaseOffset,pCurrentFileName,MAX_PATH) < 0)
			{
				clsMemManager::CFree(pCurrentFileName);
				clsMemManager::CFree(pNewFileName);
				continue;
			}

			wcscpy_s(pNewFileName,MAX_PATH,pCurrentFileName);
			wcscat_s(pNewFileName,MAX_PATH,L"_patched.exe");
			CopyFile(pCurrentFileName,pNewFileName,false);

			HANDLE hFile = CreateFileW(pNewFileName,GENERIC_READ | GENERIC_WRITE,NULL,NULL,OPEN_EXISTING,NULL,NULL);
			if(hFile == INVALID_HANDLE_VALUE)
			{
				DeleteFile(pNewFileName);
				clsMemManager::CFree(pCurrentFileName);
				clsMemManager::CFree(pNewFileName);
				continue;
			}
			
			HANDLE hFileMap = CreateFileMapping(hFile,NULL,PAGE_READWRITE,NULL,NULL,NULL);
			LPVOID lpFileBuffer = MapViewOfFile(hFileMap,FILE_MAP_WRITE | FILE_MAP_READ,NULL,NULL,NULL);
			if(lpFileBuffer == NULL)
			{
				UnmapViewOfFile(lpFileBuffer);
				CloseHandle(hFileMap);
				CloseHandle(hFile);

				DeleteFile(pNewFileName);
				clsMemManager::CFree(pNewFileName);
				clsMemManager::CFree(pCurrentFileName);

				continue;
			}

			DWORD64 fileDataOffset = (DWORD64)lpFileBuffer + clsPEManager::GetInstance()->VAtoRaw(pCurrentFileName,NULL,i->Offset - i->BaseOffset);
			if(fileDataOffset <= NULL)
			{
				UnmapViewOfFile(lpFileBuffer);
				CloseHandle(hFileMap);
				CloseHandle(hFile);

				DeleteFile(pNewFileName);
				clsMemManager::CFree(pNewFileName);
				clsMemManager::CFree(pCurrentFileName);

				continue;
			}

			if(memcmp((LPVOID)fileDataOffset,i->orgData,i->PatchSize) != 0)
			{
				UnmapViewOfFile(lpFileBuffer);
				CloseHandle(hFileMap);
				CloseHandle(hFile);

				DeleteFile(pNewFileName);
				clsMemManager::CFree(pNewFileName);
				clsMemManager::CFree(pCurrentFileName);

				continue;
			}

			memcpy((LPVOID)fileDataOffset,i->newData,i->PatchSize);

			DWORD BytesWritten = NULL;
			if(!WriteFile(hFile,lpFileBuffer,GetFileSize(hFile,NULL),&BytesWritten,NULL))
			{
				UnmapViewOfFile(lpFileBuffer);
				CloseHandle(hFileMap);
				CloseHandle(hFile);

				DeleteFile(pNewFileName);
				clsMemManager::CFree(pNewFileName);
				clsMemManager::CFree(pCurrentFileName);

				continue;
			}

			i->bSaved = true;

			UnmapViewOfFile(lpFileBuffer);
			CloseHandle(hFileMap);
			CloseHandle(hFile);

			clsMemManager::CFree(pNewFileName);
			clsMemManager::CFree(pCurrentFileName);
		}
	}
}