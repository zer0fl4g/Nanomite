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
	this->setWindowTitle(QString("[ Nanomite ] - Show Memory - PID - %1 - From: %2 - To: %3").arg(dwPID,8,16,QChar('0')).arg(StartOffset,8,16,QChar('0')).arg(StartOffset + Size,8,16,QChar('0')));

	tblHexView->horizontalHeader()->resizeSection(0,75);
	tblHexView->horizontalHeader()->resizeSection(1,135);
	tblHexView->horizontalHeader()->resizeSection(2,375);
	tblHexView->horizontalHeader()->resizeSection(3,230);

	qtDLGNanomite *MyMainWindow = qtDLGNanomite::GetInstance();

	SIZE_T dwBytesRead = NULL;
	DWORD dwCounter = NULL,
		dwProtection = NULL,
		dwStepSize = 0x10;
	DWORD64	dwBaseOffset = StartOffset;
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

	//if(!VirtualProtectEx(hProcess,(LPVOID)StartOffset,Size,PAGE_EXECUTE_READWRITE,&dwProtection))
	//{
	//	clsMemManager::CFree(pBuffer);
	//	clsMemManager::CFree(tcAsciiHexTemp);
	//	clsMemManager::CFree(tcTempBuffer);
	//	return;
	//}

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
			wsprintf(tcAsciiHexTemp,L"%02X ",*(LPBYTE)((DWORD)pBuffer + dwCounter + i));
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

	//VirtualProtectEx(hProcess,(LPVOID)StartOffset,Size,dwProtection,NULL);

	clsMemManager::CFree(pBuffer);
	clsMemManager::CFree(tcAsciiHexTemp);
	clsMemManager::CFree(tcTempBuffer);
}

qtDLGHexView::~qtDLGHexView()
{

}