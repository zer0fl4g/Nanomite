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
		
	PPEB pPEB = (PPEB)clsMemManager::CAlloc(sizeof(PEB));
	SIZE_T bytesRead = NULL;
	if(!ReadProcessMemory(hProc,pPBI.PebBaseAddress,pPEB,sizeof(PEB),&bytesRead))
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

	//InsertDataIntoTable(topElement, "AffinityMask", pPBI.AffinityMask);
	//InsertDataIntoTable(topElement, "BasePriority", pPBI.BasePriority);
	//InsertDataIntoTable(topElement, "ExitStatus", pPBI.ExitStatus);
	//InsertDataIntoTable(topElement, "InheritedFromUniqueProcessId", pPBI.InheritedFromUniqueProcessId);
	//InsertDataIntoTable(topElement, "PebBaseAddress", (DWORD64)pPBI.PebBaseAddress);
	//InsertDataIntoTable(topElement, "UniqueProcessId", pPBI.UniqueProcessId);

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
