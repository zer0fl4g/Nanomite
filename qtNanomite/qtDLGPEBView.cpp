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
#include "qtDLGPEBView.h"

#include "clsAPIImport.h"
#include "clsMemManager.h"
#include "clsHelperClass.h"

#include <QClipboard>
#include <QMenu>

qtDLGPEBView::qtDLGPEBView(HANDLE hProc, QWidget *parent, Qt::WFlags flags)
	: QWidget(parent, flags),
	m_SelectedRow(0)
{
	this->setupUi(this);
	this->setLayout(verticalLayout);
	this->setAttribute(Qt::WA_DeleteOnClose,true);
	this->setStyleSheet(clsHelperClass::LoadStyleSheet());

	treePEB->header()->resizeSection(0,250);
	//connect(treeTIB,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnCustomContextMenuRequested(QPoint)));

	ShowPEBForProcess(hProc);
}

qtDLGPEBView::~qtDLGPEBView()
{

}

void qtDLGPEBView::ShowPEBForProcess(HANDLE hProc)
{
	DWORD retLen = NULL;
	PROCESS_BASIC_INFORMATION pPBI;
	
	if(clsAPIImport::pNtQueryInformationProcess(hProc,ProcessBasicInformation,&pPBI,sizeof(pPBI),&retLen) != NULL)
	{
		MessageBoxW(NULL,L"ERROR, NtQueryInformationProcess failed!",L"Nanomite",MB_OK);
		this->close();
		return;
	}
		
	PPEB pPEB = (PPEB)clsMemManager::CAlloc(sizeof(_PEB));
	SIZE_T bytesRead = NULL;
	if(!ReadProcessMemory(hProc,pPBI.PebBaseAddress,pPEB,sizeof(_PEB),&bytesRead))
	{
		MessageBoxW(NULL,L"ERROR, Read from process memory to get PEB failed!",L"Nanomite",MB_OK);
		clsMemManager::CFree(pPEB);
		this->close();
		return;
	}
	
	// Insert into Tree
	QTreeWidgetItem *topElement = new QTreeWidgetItem();
	topElement->setText(0,"PBI");
	treePEB->addTopLevelItem(topElement);

	InsertDataIntoTable(topElement, "AffinityMask", pPBI.AffinityMask);
	InsertDataIntoTable(topElement, "BasePriority", pPBI.BasePriority);
	InsertDataIntoTable(topElement, "ExitStatus", pPBI.ExitStatus);
	InsertDataIntoTable(topElement, "InheritedFromUniqueProcessId", pPBI.InheritedFromUniqueProcessId);
	InsertDataIntoTable(topElement, "PebBaseAddress", (DWORD64)pPBI.PebBaseAddress);
	InsertDataIntoTable(topElement, "UniqueProcessId", pPBI.UniqueProcessId);

	topElement = new QTreeWidgetItem();
	topElement->setText(0,"PEB");
	treePEB->addTopLevelItem(topElement);

	InsertDataIntoTable(topElement, "ActivationContextData", pPEB->ActivationContextData);
	InsertDataIntoTable(topElement, "AnsiCodePageData", pPEB->AnsiCodePageData);
//	InsertDataIntoTable(topElement, "AppCompatFlags", pPEB->AppCompatFlags);
//	InsertDataIntoTable(topElement, "AppCompatFlagsUser", pPEB->AppCompatFlagsUser);
	InsertDataIntoTable(topElement, "AppCompatInfo", pPEB->AppCompatInfo);
	InsertDataIntoTable(topElement, "BeingDebugged", pPEB->BeingDebugged);
//	InsertDataIntoTable(topElement, "CriticalSectionTimeout", pPEB->CriticalSectionTimeout);
//	InsertDataIntoTable(topElement, "AffinityMask", pPEB->CSDVersion.Buffer);
	InsertDataIntoTable(topElement, "EnviromentUpdateCount", pPEB->EnviromentUpdateCount);
	InsertDataIntoTable(topElement, "FastPebLock", pPEB->FastPebLock);
	InsertDataIntoTable(topElement, "FastPebLockRoutine", pPEB->FastPebLockRoutine);
	InsertDataIntoTable(topElement, "FastPebUnlockRoutine", pPEB->FastPebUnlockRoutine);
	InsertDataIntoTable(topElement, "FlsBitmap", pPEB->FlsBitmap);
	InsertDataIntoTable(topElement, "FlsBitmapBits", *pPEB->FlsBitmapBits);
	InsertDataIntoTable(topElement, "FlsCallback", pPEB->FlsCallback);
	InsertDataIntoTable(topElement, "FlsHighIndex", pPEB->FlsHighIndex);
	InsertDataIntoTable(topElement, "FlsListHead_Blink", pPEB->FlsListHead_Blink);
	InsertDataIntoTable(topElement, "FlsListHead_Flink", pPEB->FlsListHead_Flink);
	InsertDataIntoTable(topElement, "FreeList", pPEB->FreeList);
	InsertDataIntoTable(topElement, "GdiDCAttributeList", pPEB->GdiDCAttributeList);
	InsertDataIntoTable(topElement, "GdiHandleBuffer", *pPEB->GdiHandleBuffer);
	InsertDataIntoTable(topElement, "GdiSharedHandleTable", pPEB->GdiSharedHandleTable);
	InsertDataIntoTable(topElement, "HeapDeCommitFreeBlockThreshold", pPEB->HeapDeCommitFreeBlockThreshold);
	InsertDataIntoTable(topElement, "HeapDeCommitTotalFreeThreshold", pPEB->HeapDeCommitTotalFreeThreshold);
	InsertDataIntoTable(topElement, "HeapSegmentCommit", pPEB->HeapSegmentCommit);
	InsertDataIntoTable(topElement, "HeapSegmentReserve", pPEB->HeapSegmentReserve);
	InsertDataIntoTable(topElement, "ImageBaseAddress", pPEB->ImageBaseAddress);
	InsertDataIntoTable(topElement, "ImageProcessAffinityMask", pPEB->ImageProcessAffinityMask);
	InsertDataIntoTable(topElement, "ImageSubsystem", pPEB->ImageSubsystem);
	InsertDataIntoTable(topElement, "ImageSubsystemMajorVersion", pPEB->ImageSubsystemMajorVersion);
	InsertDataIntoTable(topElement, "ImageSubsystemMinorVersion", pPEB->ImageSubsystemMinorVersion);
	InsertDataIntoTable(topElement, "InheritedAddressSpace", pPEB->InheritedAddressSpace);
	InsertDataIntoTable(topElement, "KernelCallbackTable", pPEB->KernelCallbackTable);
	InsertDataIntoTable(topElement, "LoaderData", pPEB->LoaderData);
	InsertDataIntoTable(topElement, "LoaderLock", pPEB->LoaderLock);
	InsertDataIntoTable(topElement, "MaximumNumberOfHeaps", pPEB->MaximumNumberOfHeaps);
	InsertDataIntoTable(topElement, "MinimumStackCommit", pPEB->MinimumStackCommit);
	InsertDataIntoTable(topElement, "Mutant", pPEB->Mutant);
	InsertDataIntoTable(topElement, "NtGlobalFlag", pPEB->NtGlobalFlag);
	InsertDataIntoTable(topElement, "NumberOfHeaps", pPEB->NumberOfHeaps);
	InsertDataIntoTable(topElement, "NumberOfProcessors", pPEB->NumberOfProcessors);
	InsertDataIntoTable(topElement, "OemCodePageData", pPEB->OemCodePageData);
	InsertDataIntoTable(topElement, "OSBuildNumber", pPEB->OSBuildNumber);
	InsertDataIntoTable(topElement, "OSCSDVersion", pPEB->OSCSDVersion);
	InsertDataIntoTable(topElement, "OSMajorVersion", pPEB->OSMajorVersion);
	InsertDataIntoTable(topElement, "OSMinorVersion", pPEB->OSMinorVersion);
	InsertDataIntoTable(topElement, "OSPlatformId", pPEB->OSPlatformId);
	InsertDataIntoTable(topElement, "PostProcessInitRoutine", pPEB->PostProcessInitRoutine);
	InsertDataIntoTable(topElement, "ProcessAssemblyStorageMap", pPEB->ProcessAssemblyStorageMap);
	InsertDataIntoTable(topElement, "ProcessHeap", pPEB->ProcessHeap);
	InsertDataIntoTable(topElement, "ProcessHeaps", pPEB->ProcessHeaps);
	InsertDataIntoTable(topElement, "ProcessParameters", pPEB->ProcessParameters);
	InsertDataIntoTable(topElement, "ProcessStarterHelper", pPEB->ProcessStarterHelper);
	InsertDataIntoTable(topElement, "pShimData", pPEB->pShimData);
	InsertDataIntoTable(topElement, "ReadImageFileExecOptions", pPEB->ReadImageFileExecOptions);
	InsertDataIntoTable(topElement, "ReadOnlySharedMemoryBase", pPEB->ReadOnlySharedMemoryBase);
	InsertDataIntoTable(topElement, "ReadOnlySharedMemoryHeap", pPEB->ReadOnlySharedMemoryHeap);
	InsertDataIntoTable(topElement, "ReadOnlyStaticServerData", pPEB->ReadOnlyStaticServerData);
	InsertDataIntoTable(topElement, "Reserved", pPEB->Reserved);
	InsertDataIntoTable(topElement, "SessionId", pPEB->SessionId);
	InsertDataIntoTable(topElement, "SpareBool", pPEB->SpareBool);
	InsertDataIntoTable(topElement, "SubSystemData", pPEB->SubSystemData);
	InsertDataIntoTable(topElement, "SystemAssemblyStorageMap", pPEB->SystemAssemblyStorageMap);
	InsertDataIntoTable(topElement, "SystemDefaultActivationContextData", pPEB->SystemDefaultActivationContextData);
	InsertDataIntoTable(topElement, "ThunksOrOptions", pPEB->ThunksOrOptions);
	InsertDataIntoTable(topElement, "TlsBitmap", pPEB->TlsBitmap);
	InsertDataIntoTable(topElement, "TlsBitmapBits", *pPEB->TlsBitmapBits);
	InsertDataIntoTable(topElement, "TlsExpansionBitmap", pPEB->TlsExpansionBitmap);
	InsertDataIntoTable(topElement, "TlsExpansionBitmapBits", *pPEB->TlsExpansionBitmapBits);
	InsertDataIntoTable(topElement, "TlsExpansionCounter", pPEB->TlsExpansionCounter);
	InsertDataIntoTable(topElement, "UnicodeCaseTableData", pPEB->UnicodeCaseTableData);
	InsertDataIntoTable(topElement, "UserSharedInfoPtr", pPEB->UserSharedInfoPtr);

	clsMemManager::CFree(pPEB);
	return;
}
	
void qtDLGPEBView::InsertDataIntoTable(QTreeWidgetItem *pTopElement, QString valueName, DWORD64 value)
{
	QTreeWidgetItem *newItem = new QTreeWidgetItem(pTopElement);
	newItem->setText(0,valueName);
	newItem->setText(1,QString("%1").arg(value,16,16,QChar('0')));	
}

//void qtDLGTIBView::OnCustomContextMenuRequested(QPoint qPoint)
//{
//	QMenu menu;
//
//	m_SelectedRow = treeTIB->indexAt(qPoint).row();
//	if(m_SelectedRow < 0) return;
//
//	QMenu *submenu = menu.addMenu("Copy to Clipboard");
//	submenu->addAction(new QAction("Line",this));
//	submenu->addAction(new QAction("Value",this));
//
//	menu.addMenu(submenu);
//	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));
//
//	menu.exec(QCursor::pos());
//}
//
//void qtDLGTIBView::MenuCallback(QAction* pAction)
//{
//	//if(QString().compare(pAction->text(),"Line") == 0)
//	//{
//	//	QClipboard* clipboard = QApplication::clipboard();
//	//	clipboard->setText(QString("%1:%2")
//	//		.arg(treeTIB->item(_iSelectedRow,0)->text())
//	//		.arg(treeTIB->item(_iSelectedRow,1)->text()));
//	//}
//	//else if(QString().compare(pAction->text(),"Debug String") == 0)
//	//{
//	//	QClipboard* clipboard = QApplication::clipboard();
//	//	clipboard->setText(treeTIB->item(_iSelectedRow,1)->text());
//	//}
//}
