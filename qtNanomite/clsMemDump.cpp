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
#include "clsMemDump.h"
#include "clsMemManager.h"

#include <QFileDialog>
#include <QMessageBox>

clsMemDump::clsMemDump( HANDLE hProc, PTCHAR FileBaseName, DWORD64 BaseOffset, DWORD Size, QWidget *pParent)
{
	bool	isProtectionChanged = false;
	DWORD	OldProtection	= NULL,
			NewProtection	= PAGE_READWRITE,
			BytesWrote		= NULL;
	SIZE_T	BytesReaded		= NULL;
	LPVOID pBuffer			= malloc(Size);

	if(!ReadProcessMemory(hProc,(LPVOID)BaseOffset,pBuffer,Size,&BytesReaded))
	{
		if(!VirtualProtectEx(hProc,(LPVOID)BaseOffset,Size,NewProtection,&OldProtection))
		{
			QMessageBox::critical(pParent,"Nanomite","Failed to access Memory!",QMessageBox::Ok,QMessageBox::Ok);
			free(pBuffer);
			return;
		}
		isProtectionChanged = true;

		if(!ReadProcessMemory(hProc,(LPVOID)BaseOffset,pBuffer,Size,&BytesReaded))
		{
			QMessageBox::critical(pParent,"Nanomite","Failed to read Memory!",QMessageBox::Ok,QMessageBox::Ok);
			free(pBuffer);
			return;
		}
	}
	
	PTCHAR FileName = (PTCHAR)malloc(MAX_PATH * sizeof(TCHAR));
	if(wcslen(FileBaseName) <= 0)
		wsprintf(FileName,L"NANOMITEDUMP_%016I64X-%016I64X_%08X.bin",BaseOffset,BaseOffset + Size,Size);
	else
		wsprintf(FileName,L"%s_%016I64X-%016I64X_%08X.bin",FileBaseName,BaseOffset,BaseOffset + Size,Size);

	QString filePath = QFileDialog::getSaveFileName(pParent,
		"Please select a place to save the dump",
		QString("%1\\%2").arg(QDir::currentPath()).arg(QString::fromWCharArray(FileName)),
		"Dump files (*.dmp *.bin)",
		NULL,
		QFileDialog::DontUseNativeDialog);

	free(FileName);

	HANDLE hFile = CreateFile(filePath.toStdWString().c_str(), GENERIC_READ | GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		QMessageBox::critical(pParent,"Nanomite","Failed to create File!",QMessageBox::Ok,QMessageBox::Ok);
		free(pBuffer);
		return;
	}

	if(!WriteFile(hFile,pBuffer,Size,&BytesWrote,NULL))
		QMessageBox::critical(pParent,"Nanomite","Failed to write to File!",QMessageBox::Ok,QMessageBox::Ok);

	free(pBuffer);
	CloseHandle(hFile);

	if(isProtectionChanged && !VirtualProtectEx(hProc,(LPVOID)BaseOffset,Size,OldProtection,&NewProtection))
		QMessageBox::critical(pParent,"Nanomite","Failed to reprotect the Memory!",QMessageBox::Ok,QMessageBox::Ok);
	
	QMessageBox::information(pParent,"Nanomite","Memory Dump finished!",QMessageBox::Ok,QMessageBox::Ok);
}

clsMemDump::~clsMemDump()
{

}