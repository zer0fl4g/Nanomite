#include "qtDLGAttach.h"

#include <Windows.h>
#include <Psapi.h>
#include <TlHelp32.h>

qtDLGAttach::qtDLGAttach(QWidget *parent, Qt::WFlags flags)
	: QDialog(parent, flags)
{
	setupUi(this);
	this->setLayout(verticalLayout);
	this->setStyleSheet("background: rgb(230, 235, 230)");

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
			do 
			{
				HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS,false,pProcessEntry.th32ProcessID);
				if(hProc != INVALID_HANDLE_VALUE)
				{
					tblProcList->insertRow(tblProcList->rowCount());

					// ProcessName
					tblProcList->setItem(tblProcList->rowCount() - 1,0,
						new QTableWidgetItem(QString().fromWCharArray(pProcessEntry.szExeFile)));

					// PID
					tblProcList->setItem(tblProcList->rowCount() - 1,1,
						new QTableWidgetItem(QString().sprintf("%d",pProcessEntry.th32ProcessID)));

					MODULEENTRY32 pModEntry;
					pModEntry.dwSize = sizeof(MODULEENTRY32);

					HANDLE hModules = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,pProcessEntry.th32ProcessID);
					if(hModules != INVALID_HANDLE_VALUE && Module32First(hModules,&pModEntry))
					{
						do 
						{
							if(pModEntry.th32ProcessID == pProcessEntry.th32ProcessID)
							{
								tblProcList->setItem(tblProcList->rowCount() - 1,2,
									new QTableWidgetItem(QString().fromWCharArray(pModEntry.szExePath)));
								break;
							}
						} while (Module32Next(hModules,&pModEntry));
					}
				}
			} while (Process32Next(hToolSnapShot,&pProcessEntry));
		}
	}
}

void qtDLGAttach::OnProcessDoubleClick(int iRow,int iColumn)
{
	emit StartAttachedDebug(tblProcList->item(iRow,1)->text().toInt());
	close();
	return;
}