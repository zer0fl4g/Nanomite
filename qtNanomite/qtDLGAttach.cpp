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
#include "qtDLGAttach.h"

#include "clsAPIImport.h"
#include "clsHelperClass.h"
#include "clsMemManager.h"

#include <Windows.h>
#include <Psapi.h>
#include <TlHelp32.h>


qtDLGAttach::qtDLGAttach(QWidget *parent, Qt::WFlags flags)
	: QDialog(parent, flags)
{
	setupUi(this);
	this->setLayout(verticalLayout);
	this->setStyleSheet(clsHelperClass::LoadStyleSheet());

	tblProcList->horizontalHeader()->resizeSection(0,135);
	tblProcList->horizontalHeader()->resizeSection(1,50);
	tblProcList->horizontalHeader()->resizeSection(2,50);
	tblProcList->horizontalHeader()->setFixedHeight(21);

	connect(tblProcList,SIGNAL(cellDoubleClicked(int,int)),this,SLOT(OnProcessDoubleClick(int,int)));
	connect(new QShortcut(QKeySequence("F5"),this),SIGNAL(activated()),this,SLOT(FillProcessList()));
	connect(new QShortcut(QKeySequence::InsertParagraphSeparator,this),SIGNAL(activated()),this,SLOT(OnReturnPressed()));

	FillProcessList();
}

qtDLGAttach::~qtDLGAttach()
{

}

void qtDLGAttach::FillProcessList()
{
	tblProcList->setRowCount(0);

	HANDLE hToolSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	if( hToolSnapShot != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 pProcessEntry;
		pProcessEntry.dwSize = sizeof(PROCESSENTRY32);

		if(Process32First(hToolSnapShot,&pProcessEntry))
		{
			PTCHAR ProcessFile = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));
			do 
			{
				HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,false,pProcessEntry.th32ProcessID);
				if(hProc != NULL)
				{
					tblProcList->insertRow(tblProcList->rowCount());

					// ProcessName		
					tblProcList->setItem(tblProcList->rowCount() - 1,0,
						new QTableWidgetItem(QString().fromWCharArray(pProcessEntry.szExeFile)));

					// PID
					tblProcList->setItem(tblProcList->rowCount() - 1,1,
						new QTableWidgetItem(QString().sprintf("%d",pProcessEntry.th32ProcessID)));

					// Type
					if(clsAPIImport::pIsWow64Process == NULL)
						tblProcList->setItem(tblProcList->rowCount() - 1,2,
							new QTableWidgetItem(QString("x86")));
					else
					{
						BOOL isWoW64 = FALSE;
						clsAPIImport::pIsWow64Process(hProc,&isWoW64);
						if(isWoW64)
						{
							tblProcList->setItem(tblProcList->rowCount() - 1,2,
								new QTableWidgetItem(QString("x86")));
						}
						else
						{
							tblProcList->setItem(tblProcList->rowCount() - 1,2,
								new QTableWidgetItem(QString("x64")));
						}
					}

					// Process Path
					memset(ProcessFile,0,MAX_PATH * sizeof(TCHAR));
					if(GetModuleFileNameEx(hProc,NULL,ProcessFile,MAX_PATH) > 0)
					{
						QString processPath = QString().fromWCharArray(ProcessFile);
						if(processPath.contains("SystemRoot"))
						{
							PTCHAR tempPath = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));

							processPath.replace("SystemRoot","%SystemRoot%");
							processPath.toWCharArray(ProcessFile);

							if(ExpandEnvironmentStrings(ProcessFile,tempPath,MAX_PATH))
							{
								processPath = QString().fromWCharArray(tempPath);
								processPath.replace("\\C:\\","C:\\");
							}

							clsMemManager::CFree((LPVOID)tempPath);
						}

						tblProcList->setItem(tblProcList->rowCount() - 1,3,
							new QTableWidgetItem(processPath));
					}
					else
						tblProcList->removeRow(tblProcList->rowCount() - 1);
					//	tblProcList->setItem(tblProcList->rowCount() - 1,3,
					//		new QTableWidgetItem(""));

					CloseHandle(hProc);
				}
			} while (Process32Next(hToolSnapShot,&pProcessEntry));
			clsMemManager::CFree(ProcessFile);
		}

		CloseHandle(hToolSnapShot);
	}
}

void qtDLGAttach::OnProcessDoubleClick(int iRow,int iColumn)
{
	QString targetFile = tblProcList->item(iRow,3)->text();
	if(!targetFile.isEmpty() && targetFile.length() > 0)
	{
		emit StartAttachedDebug(tblProcList->item(iRow,1)->text().toInt(),targetFile);
		close();
	}	
	else
		QMessageBox::critical(this,"Nanomite","This is a invalid File! Please select another one!",QMessageBox::Ok,QMessageBox::Ok);
}

void qtDLGAttach::OnReturnPressed()
{
	if(tblProcList->selectedItems().count() <= 0) return;

	QTableWidgetItem *pItem = tblProcList->selectedItems()[0];

	OnProcessDoubleClick(pItem->row(),pItem->column());
}

