#include "qtDLGAttach.h"
#include "clsHelperClass.h"

#include <Windows.h>
#include <Psapi.h>
#include <TlHelp32.h>

#include "clsMemManager.h"

qtDLGAttach::qtDLGAttach(QWidget *parent, Qt::WFlags flags)
	: QDialog(parent, flags)
{
	setupUi(this);
	this->setLayout(verticalLayout);
	this->setStyleSheet(clsHelperClass::LoadStyleSheet());

	tblProcList->horizontalHeader()->resizeSection(0,135);
	tblProcList->horizontalHeader()->resizeSection(1,50);

	connect(tblProcList,SIGNAL(cellDoubleClicked(int,int)),this,SLOT(OnProcessDoubleClick(int,int)));

	FillProcessList();
}

qtDLGAttach::~qtDLGAttach()
{

}

void qtDLGAttach::FillProcessList()
{
	HANDLE hToolSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	if( hToolSnapShot != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 pProcessEntry;
		pProcessEntry.dwSize = sizeof(PROCESSENTRY32);

		if(Process32First(hToolSnapShot,&pProcessEntry))
		{
			PTCHAR ProcessFile = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));
			do 
			{
				HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,false,pProcessEntry.th32ProcessID);
				if(hProc != INVALID_HANDLE_VALUE)
				{
					tblProcList->insertRow(tblProcList->rowCount());

					// ProcessName
					tblProcList->setItem(tblProcList->rowCount() - 1,0,
						new QTableWidgetItem(QString().fromWCharArray(pProcessEntry.szExeFile)));

					// PID
					tblProcList->setItem(tblProcList->rowCount() - 1,1,
						new QTableWidgetItem(QString().sprintf("%d",pProcessEntry.th32ProcessID)));

					// Process Path
					if(GetModuleFileNameEx(hProc,NULL,ProcessFile,MAX_PATH) > 0)
						tblProcList->setItem(tblProcList->rowCount() - 1,2,
							new QTableWidgetItem(QString().fromWCharArray(ProcessFile)));
					else
						tblProcList->setItem(tblProcList->rowCount() - 1,2,
							new QTableWidgetItem(""));

					CloseHandle(hProc);
				}
			} while (Process32Next(hToolSnapShot,&pProcessEntry));
			free(ProcessFile);
		}
	}
}

void qtDLGAttach::OnProcessDoubleClick(int iRow,int iColumn)
{
	QString targetFile = tblProcList->item(iRow,2)->text();
	if(!targetFile.isEmpty() && targetFile.length() > 0)
	{
		emit StartAttachedDebug(tblProcList->item(iRow,1)->text().toInt(),targetFile);
		close();
	}	
	else
		MessageBoxW(NULL,L"This is a invalid File! Please select another one!",L"Nanomite",MB_OK);
}