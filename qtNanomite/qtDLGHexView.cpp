#include "qtDLGHexView.h"
#include "qtDLGNanomite.h"

#include "clsMemManager.h"

#include <Windows.h>

qtDLGHexView::qtDLGHexView(QWidget *parent, Qt::WFlags flags,unsigned long dwPID, unsigned long long StartOffset,unsigned long long Size)
	: QWidget(parent, flags)
{
	setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose,true);
	this->setLayout(verticalLayout);

	tblHexView->horizontalHeader()->resizeSection(0,75);
	tblHexView->horizontalHeader()->resizeSection(1,135);
	tblHexView->horizontalHeader()->resizeSection(2,375);
	tblHexView->horizontalHeader()->resizeSection(3,230);

	qtDLGNanomite *MyMainWindow = qtDLGNanomite::GetInstance();

	SIZE_T dwBytesRead = NULL;
	DWORD dwCounter = NULL,
		dwProtection = NULL,
		dwStepSize = 0x10,
		dwBaseOffset = StartOffset;
	HANDLE hProcess = NULL;
	LPVOID pBuffer = malloc(Size);
	TCHAR *tcTempBuffer = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR)),
		*tcAsciiHexTemp = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(TCHAR));

	tblHexView->setRowCount(0);

	for(size_t i = 0;i < MyMainWindow->coreDebugger->PIDs.size();i++)
	{
		if(dwPID == MyMainWindow->coreDebugger->PIDs[i].dwPID)
			hProcess = MyMainWindow->coreDebugger->PIDs[i].hProc;
	}

	if(hProcess == INVALID_HANDLE_VALUE)
		return;

	if(!VirtualProtectEx(hProcess,(LPVOID)StartOffset,Size,PAGE_EXECUTE_READWRITE,&dwProtection))
	{
		clsMemManager::CFree(pBuffer);
		clsMemManager::CFree(tcAsciiHexTemp);
		clsMemManager::CFree(tcTempBuffer);
		return;
	}

	if(!ReadProcessMemory(hProcess,(LPVOID)StartOffset,(LPVOID)pBuffer,Size,&dwBytesRead))
	{
		clsMemManager::CFree(pBuffer);
		clsMemManager::CFree(tcAsciiHexTemp);
		clsMemManager::CFree(tcTempBuffer);
		return;
	}

	for(int iLineStep = 0; iLineStep < (Size / dwStepSize); iLineStep++)
	{
		if(StartOffset > (dwBaseOffset + Size))
			break;

		tblHexView->insertRow(tblHexView->rowCount());
		tblHexView->setItem(tblHexView->rowCount() - 1,0,
			new QTableWidgetItem(QString("%1").arg(dwPID,8,16,QChar('0'))));

		// Offset
		tblHexView->setItem(tblHexView->rowCount() - 1,1,
			new QTableWidgetItem(QString("%1").arg(StartOffset,16,16,QChar('0'))));

		// Hexiss
		memset(tcTempBuffer,0,sizeof(MAX_PATH * sizeof(TCHAR)));
		for(size_t i = 0;i < dwStepSize;i++)
		{
			wsprintf(tcAsciiHexTemp,L"%02X ",*(PCHAR)((DWORD)pBuffer + dwCounter + i));
			if(wcsstr(tcAsciiHexTemp,L"FFF") != NULL)
				wcscat(tcTempBuffer,L"FF ");
			else
				wcscat(tcTempBuffer,tcAsciiHexTemp);
		}
		tblHexView->setItem(tblHexView->rowCount() - 1,2,
			new QTableWidgetItem(QString::fromWCharArray(tcTempBuffer)));

		//Acsii
		memset(tcTempBuffer,0,sizeof(MAX_PATH * sizeof(TCHAR)));
		memset(tcAsciiHexTemp,0,sizeof(MAX_PATH * sizeof(TCHAR)));
		for(size_t i = 0;i < dwStepSize;i++)
		{
			wsprintf(tcAsciiHexTemp,L"%c ",*(PCHAR)((DWORD)pBuffer + dwCounter + i));
			if(wcsstr(tcAsciiHexTemp,L"  ") != NULL)
				wcscat(tcTempBuffer,L". ");
			else if(wcsstr(tcAsciiHexTemp,L"\r") != NULL)
				wcscat(tcTempBuffer,L". ");
			else if(wcsstr(tcAsciiHexTemp,L"\t") != NULL)
				wcscat(tcTempBuffer,L". ");
			else if(wcsstr(tcAsciiHexTemp,L"\n") != NULL)
				wcscat(tcTempBuffer,L". ");
			else if(*(PCHAR)((DWORD)pBuffer + dwCounter + i) ==  0x00)
				wcscat(tcTempBuffer,L". ");
			else
				wcscat(tcTempBuffer,tcAsciiHexTemp);
		}
		tblHexView->setItem(tblHexView->rowCount() - 1,3,
			new QTableWidgetItem(QString::fromWCharArray(tcTempBuffer)));

		StartOffset += dwStepSize;
		dwCounter += dwStepSize;
	}

	VirtualProtectEx(hProcess,(LPVOID)StartOffset,Size,dwProtection,NULL);

	clsMemManager::CFree(pBuffer);
	clsMemManager::CFree(tcAsciiHexTemp);
	clsMemManager::CFree(tcTempBuffer);
}

qtDLGHexView::~qtDLGHexView()
{

}