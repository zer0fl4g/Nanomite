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
#include "qtDLGAssembler.h"
#include "qtDLGPatchManager.h"

#include "clsMemManager.h"

#include <QFile>

qtDLGAssembler::qtDLGAssembler(QWidget *parent, Qt::WFlags flags,
	HANDLE processHandle,quint64 instructionOffset,clsDisassembler *pCurrentDisassembler,bool is64Bit)
	: QWidget(parent, flags),
	m_processHandle(processHandle),
	m_instructionOffset(instructionOffset),
	m_pCurrentDisassembler(pCurrentDisassembler),
	m_is64Bit(is64Bit)
{
	setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose,true);
	this->setFixedSize(this->width(),this->height());

	connect(lineEdit,SIGNAL(returnPressed()),this,SLOT(InsertNewInstructions()));
}

qtDLGAssembler::~qtDLGAssembler()
{

}

void qtDLGAssembler::InsertNewInstructions()
{
	if(lineEdit->text().length() <= 0)
	{
		close();
		return;
	}

	QMap<QString,DisAsDataRow>::const_iterator i = m_pCurrentDisassembler->SectionDisAs.constFind(QString("%1").arg(m_instructionOffset,16,16,QChar('0')).toUpper());
	if((QMapData::Node *)i == (QMapData::Node *)m_pCurrentDisassembler->SectionDisAs.constEnd()) 
	{
		close();
		return;
	}

	QString oldOpcodes = i.value().OpCodes;
	DWORD oldOpcodeLen = oldOpcodes.replace(" ", "").length() / 2,
		newOpcodeLen = NULL;
		
	QFile tempOutput("nanomite.asm");
	tempOutput.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream out(&tempOutput);

	if(m_is64Bit)
		out << "BITS 64\r\n";
	else
		out << "BITS 32\r\n";
	out << lineEdit->text();
	tempOutput.close();


	STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si,sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi,sizeof(pi));
	TCHAR szCommandLine[] = L"nasm.exe -o nanomite.bin nanomite.asm";

	if(!CreateProcess(NULL,szCommandLine,NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL,&si,&pi)) 
    {
        MessageBoxW(NULL,L"Error, unable to launch assembler!",L"Nanomite",MB_OK);
		close();
		return;
    }

    WaitForSingleObject(pi.hProcess,INFINITE);
	CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
	DeleteFile(L"nanomite.asm");

	HANDLE hFile = CreateFileW(L"nanomite.bin",GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,NULL,NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		close();
		return;
	}

	int iLen = GetFileSize(hFile,NULL);
	LPVOID pFileBuffer = clsMemManager::CAlloc(iLen);
	DWORD BytesRead = NULL;	
	if(!ReadFile(hFile,pFileBuffer,iLen,&BytesRead,NULL))
	{
		CloseHandle(hFile);
		DeleteFile(L"nanomite.bin");
		clsMemManager::CFree(pFileBuffer);
		MessageBoxW(NULL,L"Error, no valid opcodes found!",L"Nanomite",MB_OK);
		close();
		return;
	}
	CloseHandle(hFile);
	DeleteFile(L"nanomite.bin");


	if(BytesRead <= 0)
	{
		clsMemManager::CFree(pFileBuffer);
		MessageBoxW(NULL,L"Error, no valid opcodes found!",L"Nanomite",MB_OK);
		close();
		return;
	}

	if(oldOpcodeLen >= BytesRead)
		newOpcodeLen = oldOpcodeLen;
	else if(oldOpcodeLen < BytesRead)
	{
		newOpcodeLen = oldOpcodeLen;
		while(newOpcodeLen < BytesRead)
		{
			++i;
			if((QMapData::Node *)i == (QMapData::Node *)m_pCurrentDisassembler->SectionDisAs.constEnd()) return;
			oldOpcodes = i.value().OpCodes;
			newOpcodeLen += oldOpcodes.replace(" ", "").length() / 2;
		}
	}

	LPVOID pBuffer = clsMemManager::CAlloc(newOpcodeLen);
	memset(pBuffer,0x90,newOpcodeLen);
	memcpy(pBuffer,pFileBuffer,BytesRead);

	qtDLGPatchManager::AddNewPatch(0,m_processHandle,m_instructionOffset,newOpcodeLen,pBuffer);

	clsMemManager::CFree(pBuffer);
	clsMemManager::CFree(pFileBuffer);

	m_pCurrentDisassembler->SectionDisAs.clear();
	lineEdit->clear();
	close();
}