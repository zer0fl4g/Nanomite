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
#include "qtDLGTIBView.h"

#include "clsAPIImport.h"
#include "clsMemManager.h"
#include "clsHelperClass.h"

#include <QClipboard>
#include <QMenu>

qtDLGTIBView::qtDLGTIBView(HANDLE processHandle, HANDLE threadHandle, QWidget *parent, Qt::WFlags flags)
	: QWidget(parent, flags)
{
	this->setupUi(this);
	this->setLayout(verticalLayout);
	this->setAttribute(Qt::WA_DeleteOnClose,true);
	this->setStyleSheet(clsHelperClass::LoadStyleSheet());

	treeTIB->header()->resizeSection(0,250);

	ShowTIBForThread(processHandle,threadHandle);
}

qtDLGTIBView::~qtDLGTIBView()
{

}

void qtDLGTIBView::ShowTIBForThread(HANDLE processHandle, HANDLE threadHandle)
{
	DWORD retLen = NULL;
	THREAD_BASIC_INFORMATION pTIB;

	if(clsAPIImport::pNtQueryInformationThread(threadHandle,ThreadBasicInformation,&pTIB,sizeof(pTIB),&retLen) != NULL)
	{
		MessageBoxW(NULL,L"ERROR, NtQueryInformationThread failed!",L"Nanomite",MB_OK);
		this->close();
		return;
	}

	PTEB pTEB = (PTEB)clsMemManager::CAlloc(sizeof(TEB));
	SIZE_T bytesRead = NULL;
	if(!ReadProcessMemory(processHandle,pTIB.TebBaseAddress,pTEB,sizeof(TEB),&bytesRead))
	{
		MessageBoxW(NULL,L"ERROR, Could not read the TEB address!",L"Nanomite",MB_OK);
		clsMemManager::CFree(pTEB);
		this->close();
		return;
	}

	// Insert into Tree
	QTreeWidgetItem *topElement = new QTreeWidgetItem();
	topElement->setText(0,"TBI");
	treeTIB->addTopLevelItem(topElement);

	InsertDataIntoTable(topElement, "AffinityMask", pTIB.AffinityMask);
	InsertDataIntoTable(topElement, "BasePriority", pTIB.BasePriority);
	InsertDataIntoTable(topElement, "ClientId.UniqueProcess", (DWORD)pTIB.ClientId.UniqueProcess);
	InsertDataIntoTable(topElement, "ClientId.UniqueThread", (DWORD)pTIB.ClientId.UniqueThread);
	InsertDataIntoTable(topElement, "ExitStatus", pTIB.ExitStatus);
	InsertDataIntoTable(topElement, "Priority", pTIB.Priority);
	InsertDataIntoTable(topElement, "TebBaseAddress", (DWORD64)pTIB.TebBaseAddress);

	topElement = new QTreeWidgetItem();
	topElement->setText(0,"TEB");
	treeTIB->addTopLevelItem(topElement);

	InsertDataIntoTable(topElement, "ActiveRpcInfo", (DWORD)pTEB->ActiveRpcInfo);
	InsertDataIntoTable(topElement, "Cid.UniqueProcess", (DWORD)pTEB->Cid.UniqueProcess);
	InsertDataIntoTable(topElement, "Cid.UniqueThread", (DWORD)pTEB->Cid.UniqueThread);
	InsertDataIntoTable(topElement, "CountOfOwnedCriticalSections", (DWORD)pTEB->CountOfOwnedCriticalSections);
	InsertDataIntoTable(topElement, "CsrClientThread", (DWORD)pTEB->CsrClientThread);
	InsertDataIntoTable(topElement, "CurrentLocale", (DWORD)pTEB->CurrentLocale);
	InsertDataIntoTable(topElement, "DbgSsReserved", (DWORD)pTEB->DbgSsReserved);
	InsertDataIntoTable(topElement, "DeallocationStack", (DWORD)pTEB->DeallocationStack);
	InsertDataIntoTable(topElement, "EnvironmentPointer", (DWORD)pTEB->EnvironmentPointer);
	InsertDataIntoTable(topElement, "ExceptionCode", (DWORD)pTEB->ExceptionCode);
	InsertDataIntoTable(topElement, "FpSoftwareStatusRegister", (DWORD)pTEB->FpSoftwareStatusRegister);
	InsertDataIntoTable(topElement, "GdiBatchCount", (DWORD)pTEB->GdiBatchCount);
	InsertDataIntoTable(topElement, "GdiBrush", (DWORD)pTEB->GdiBrush);
	InsertDataIntoTable(topElement, "GdiCachedProcessHandle", (DWORD)pTEB->GdiCachedProcessHandle);
	InsertDataIntoTable(topElement, "GdiClientPID", (DWORD)pTEB->GdiClientPID);
	InsertDataIntoTable(topElement, "GdiClientTID", (DWORD)pTEB->GdiClientTID);
	InsertDataIntoTable(topElement, "GdiPen", (DWORD)pTEB->GdiPen);
	InsertDataIntoTable(topElement, "GdiRgn", (DWORD)pTEB->GdiRgn);
	InsertDataIntoTable(topElement, "GdiThreadLocaleInfo", (DWORD)pTEB->GdiThreadLocaleInfo);
	InsertDataIntoTable(topElement, "GlContext", (DWORD)pTEB->GlContext);
	InsertDataIntoTable(topElement, "GlCurrentRC", (DWORD)pTEB->GlCurrentRC);
	InsertDataIntoTable(topElement, "GlDispatchTable", (DWORD)pTEB->GlDispatchTable);
	InsertDataIntoTable(topElement, "GlReserved1", (DWORD)pTEB->GlReserved1);
	InsertDataIntoTable(topElement, "GlReserved2", (DWORD)pTEB->GlReserved2);
	InsertDataIntoTable(topElement, "GlSection", (DWORD)pTEB->GlSection);
	InsertDataIntoTable(topElement, "GlSectionInfo", (DWORD)pTEB->GlSectionInfo);
	InsertDataIntoTable(topElement, "GlTable", (DWORD)pTEB->GlTable);
	InsertDataIntoTable(topElement, "HardErrorDisabled", (DWORD)pTEB->HardErrorDisabled);
	InsertDataIntoTable(topElement, "Instrumentation", (DWORD)pTEB->Instrumentation);
	InsertDataIntoTable(topElement, "LastErrorValue", (DWORD)pTEB->LastErrorValue);
	InsertDataIntoTable(topElement, "LastStatusValue", (DWORD)pTEB->LastStatusValue);
	InsertDataIntoTable(topElement, "Peb", (DWORD)pTEB->Peb);
	InsertDataIntoTable(topElement, "RealClientId.UniqueProcess", (DWORD)pTEB->RealClientId.UniqueProcess);
	InsertDataIntoTable(topElement, "RealClientId.UniqueThread", (DWORD)pTEB->RealClientId.UniqueThread);
	InsertDataIntoTable(topElement, "ReservedForNtRpc", (DWORD)pTEB->ReservedForNtRpc);
	InsertDataIntoTable(topElement, "ReservedForOle", (DWORD)pTEB->ReservedForOle);
	InsertDataIntoTable(topElement, "Spare1", (DWORD)pTEB->Spare1);
	InsertDataIntoTable(topElement, "Spare2", (DWORD)pTEB->Spare2);
	InsertDataIntoTable(topElement, "Spare3", (DWORD)pTEB->Spare3);
	InsertDataIntoTable(topElement, "Spare4", (DWORD)pTEB->Spare4);
	InsertDataIntoTable(topElement, "SpareBytes1", (DWORD)pTEB->SpareBytes1);
	InsertDataIntoTable(topElement, "StackCommit", (DWORD)pTEB->StackCommit);
	InsertDataIntoTable(topElement, "StackCommitMax", (DWORD)pTEB->StackCommitMax);
	InsertDataIntoTable(topElement, "StackReserved", (DWORD)pTEB->StackReserved);
//	InsertDataIntoTable(topElement, "StaticUnicodeBuffer", (DWORD)pTEB->StaticUnicodeBuffer);
//	InsertDataIntoTable(topElement, "StaticUnicodeString", (DWORD)pTEB->StaticUnicodeString);
	InsertDataIntoTable(topElement, "SystemReserved1", (DWORD)pTEB->SystemReserved1);
	InsertDataIntoTable(topElement, "SystemReserved2", (DWORD)pTEB->SystemReserved2);
	InsertDataIntoTable(topElement, "ThreadLocalStoragePointer", (DWORD)pTEB->ThreadLocalStoragePointer);
//	InsertDataIntoTable(topElement, "Tib", (DWORD)pTEB->Tib);
//	InsertDataIntoTable(topElement, "TlsLinks", (DWORD)pTEB->TlsLinks);
//	InsertDataIntoTable(topElement, "TlsSlots", (DWORD)pTEB->TlsSlots);
	InsertDataIntoTable(topElement, "UserReserved", (DWORD)pTEB->UserReserved);
	InsertDataIntoTable(topElement, "Vdm", (DWORD)pTEB->Vdm);
	InsertDataIntoTable(topElement, "WaitingOnLoaderLock", (DWORD)pTEB->WaitingOnLoaderLock);
	InsertDataIntoTable(topElement, "Win32ClientInfo", (DWORD)pTEB->Win32ClientInfo);
	InsertDataIntoTable(topElement, "Win32ThreadInfo", (DWORD)pTEB->Win32ThreadInfo);
	InsertDataIntoTable(topElement, "WinSockData", (DWORD)pTEB->WinSockData);
	InsertDataIntoTable(topElement, "WOW32Reserved", (DWORD)pTEB->WOW32Reserved);

	clsMemManager::CFree(pTEB);
	return;
}

void qtDLGTIBView::InsertDataIntoTable(QTreeWidgetItem *pTopElement, QString valueName, DWORD64 value)
{
	QTreeWidgetItem *newItem = new QTreeWidgetItem(pTopElement);
	newItem->setText(0,valueName);
	newItem->setText(1,QString("%1").arg(value,16,16,QChar('0')));
}