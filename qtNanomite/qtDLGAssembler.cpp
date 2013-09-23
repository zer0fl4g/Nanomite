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
#include <QProcess>

qtDLGAssembler::qtDLGAssembler(QWidget *parent, Qt::WFlags flags, HANDLE processHandle, quint64 instructionOffset, QString currentInstruction, clsDisassembler *pCurrentDisassembler, bool is64Bit)
	: QWidget(parent, flags),
	m_processHandle(processHandle),
	m_instructionOffset(instructionOffset),
	m_pCurrentDisassembler(pCurrentDisassembler),
	m_is64Bit(is64Bit)
{
	setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose,true);
	this->setFixedSize(this->width(),this->height());

	connect(new QShortcut(QKeySequence(Qt::Key_Escape),this),SIGNAL(activated()),this,SLOT(close()));
	connect(lineEdit,SIGNAL(returnPressed()),this,SLOT(InsertNewInstructions()));

	this->setWindowTitle(QString("Assemble at %1").arg(instructionOffset,16,16,QChar('0')));

	lineEdit->setText(currentInstruction);
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
	if(i == m_pCurrentDisassembler->SectionDisAs.constEnd()) 
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
		out << "BITS 64\n";
	else
		out << "BITS 32\n";

	out << "org 0x" << i.value().Offset << "\r\n";
	out << lineEdit->text();
	tempOutput.close();

	QProcess nasm;
	nasm.setReadChannel(QProcess::StandardOutput);
    nasm.setProcessChannelMode(QProcess::MergedChannels);
    nasm.start("nasm.exe -o nanomite.bin nanomite.asm");
    if (!nasm.waitForStarted())
	{
		QMessageBox::critical(this, "Nanomite", "Unable to launch assembler!", QMessageBox::Ok, QMessageBox::Ok);
		close();
		return;
	}

	while(nasm.state() != QProcess::NotRunning)
	{
        nasm.waitForReadyRead();
		QString errorMessage = nasm.readAll();

		if(errorMessage.contains("nanomite.asm:3:"))
		{
			errorMessage.replace("nanomite.asm:3:","");
			QMessageBox::critical(this, "Nanomite", errorMessage, QMessageBox::Ok, QMessageBox::Ok);
			lineEdit->clear();
			return;
		}
    }
	DeleteFile(L"nanomite.asm");

	HANDLE hFile = CreateFileW(L"nanomite.bin",GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,NULL,NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		lineEdit->clear();
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
		QMessageBox::critical(this,"Nanomite","no valid opcodes found!",QMessageBox::Ok,QMessageBox::Ok);
		lineEdit->clear();
		return;
	}
	CloseHandle(hFile);
	DeleteFile(L"nanomite.bin");


	if(BytesRead <= 0)
	{
		clsMemManager::CFree(pFileBuffer);
		QMessageBox::critical(this,"Nanomite","no valid opcodes found!",QMessageBox::Ok,QMessageBox::Ok);
		lineEdit->clear();
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
			if(i == m_pCurrentDisassembler->SectionDisAs.constEnd()) return;
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

	lineEdit->clear();
	close();
}