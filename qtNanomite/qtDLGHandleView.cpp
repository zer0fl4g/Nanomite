#include "qtDLGHandleView.h"
#include "qtDLGNanomite.h"

#include "clsAPIImport.h"
#include "NativeHeaders.h"
#include "clsMemManager.h"

qtDLGHandleView::qtDLGHandleView(QWidget *parent, Qt::WFlags flags,qint32 iPID)
	: QWidget(parent, flags)
{
	setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose,true);
	this->setLayout(verticalLayout);

	_iPID = iPID;

	// Init List
	tblHandleView->horizontalHeader()->resizeSection(0,75);
	tblHandleView->horizontalHeader()->resizeSection(1,135);
	tblHandleView->horizontalHeader()->resizeSection(2,135);

	// Display
	qtDLGNanomite *myMainWindow = qtDLGNanomite::GetInstance();

	int iForEntry = 0;
	int iForEnd = myMainWindow->coreDebugger->PIDs.size();

	for(int i = 0; i < myMainWindow->coreDebugger->PIDs.size(); i++)
	{
		if(myMainWindow->coreDebugger->PIDs[i].dwPID == _iPID)
			iForEntry = i; iForEnd = i + 1;
	}

	for(int i = iForEntry; i < iForEnd;i++)
	{
		NTSTATUS status;
		ULONG handleInfoSize = 0x10000;
		DWORD dwPID = myMainWindow->coreDebugger->PIDs[i].dwPID;
		HANDLE hProc = myMainWindow->coreDebugger->PIDs[i].hProc;

		PSYSTEM_HANDLE_INFORMATION handleInfo = (PSYSTEM_HANDLE_INFORMATION)malloc(handleInfoSize);

		while ((status = clsAPIImport::pNtQuerySystemInformation(SystemHandleInformation,handleInfo,handleInfoSize,NULL)) == STATUS_INFO_LENGTH_MISMATCH)
			handleInfo = (PSYSTEM_HANDLE_INFORMATION)realloc(handleInfo, handleInfoSize *= 2);

		if (!NT_SUCCESS(status))
		{
			MessageBox(NULL,L"NtQuerySystemInformation failed!",L"Nanomite",MB_OK);
			close();
		}

		for (int iCount = 0; iCount < handleInfo->HandleCount; iCount++)
		{
			SYSTEM_HANDLE handle = handleInfo->Handles[iCount];
			HANDLE dupHandle = NULL;
			POBJECT_TYPE_INFORMATION objectTypeInfo;
			PVOID objectNameInfo;
			UNICODE_STRING objectName,
				objectTypeName;
			ULONG returnLength;
			BOOL bWideBool = false;

			if (handle.ProcessId != dwPID)
				continue;

			if (!NT_SUCCESS(clsAPIImport::pNtDuplicateObject(hProc,(HANDLE)handle.Handle,GetCurrentProcess(),&dupHandle,0,0,0)))
				continue;

			objectTypeInfo = (POBJECT_TYPE_INFORMATION)malloc(0x1000);
			if (!NT_SUCCESS(clsAPIImport::pNtQueryObject(dupHandle,ObjectTypeInformation,objectTypeInfo,0x1000,NULL)))
			{
				CloseHandle(dupHandle);
				free(objectTypeInfo);
				continue;
			}
			
			if (handle.GrantedAccess == 0x0012019f)
			{
				InsertDataIntoTable(dwPID,(DWORD)handle.Handle,objectTypeInfo->Name.Buffer,L"Didn´t get name");
				free(objectTypeInfo);
				CloseHandle(dupHandle);
				continue;
			}

			objectNameInfo = malloc(0x1000);
			if (!NT_SUCCESS(clsAPIImport::pNtQueryObject(dupHandle,ObjectNameInformation,objectNameInfo,0x1000,&returnLength)))
			{
				objectNameInfo = realloc(objectNameInfo, returnLength);
				if (!NT_SUCCESS(clsAPIImport::pNtQueryObject(dupHandle,ObjectNameInformation,objectNameInfo,returnLength,NULL)))
				{
					InsertDataIntoTable(dwPID,(DWORD)handle.Handle,objectTypeInfo->Name.Buffer,L"Couldn´t get name");
					free(objectTypeInfo);
					free(objectNameInfo);
					CloseHandle(dupHandle);
					continue;
				}
			}

			objectName = *(PUNICODE_STRING)objectNameInfo;

			if (objectName.Length)
				InsertDataIntoTable(dwPID,(DWORD)handle.Handle,objectTypeInfo->Name.Buffer,objectName.Buffer);
			else
				InsertDataIntoTable(dwPID,(DWORD)handle.Handle,objectTypeInfo->Name.Buffer,L"unnamed");

			free(objectTypeInfo);
			free(objectNameInfo);
			CloseHandle(dupHandle);
		}
		free(handleInfo);
	}
}

qtDLGHandleView::~qtDLGHandleView()
{

}

void qtDLGHandleView::InsertDataIntoTable(DWORD dwPID,DWORD dwHandle,PTCHAR ptType,PTCHAR ptName)
{
	tblHandleView->insertRow(tblHandleView->rowCount());

	tblHandleView->setItem(tblHandleView->rowCount() - 1,0,
		new QTableWidgetItem(QString().sprintf("%08X",dwPID)));

	tblHandleView->setItem(tblHandleView->rowCount() - 1,1,
		new QTableWidgetItem(QString().sprintf("%08X",dwHandle)));

	tblHandleView->setItem(tblHandleView->rowCount() - 1,2,
		new QTableWidgetItem(QString::fromStdWString(ptType)));

	tblHandleView->setItem(tblHandleView->rowCount() - 1,3,
		new QTableWidgetItem(QString::fromStdWString(ptName)));
}