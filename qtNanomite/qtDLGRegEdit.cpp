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
#include "qtDLGRegEdit.h"

#include "clsMemManager.h"
#include "clsHelperClass.h"

#include <QTCore>
#include <QtGui>

qtDLGRegEdit::qtDLGRegEdit(QWidget *parent, Qt::WFlags flags,LPVOID pProcessContext,bool bIs64)
	: QDialog(parent, flags)
{
	_bIs64 = bIs64;
	_pProcessContext = pProcessContext;

	if(_bIs64)
	{
		ui64.setupUi(this);
		connect(ui64.pbExit,SIGNAL(clicked()),this,SLOT(OnExit()));
		connect(ui64.pbSave,SIGNAL(clicked()),this,SLOT(OnSaveAndExit()));
	}
	else
	{
		ui86.setupUi(this);
		connect(ui86.pbExit,SIGNAL(clicked()),this,SLOT(OnExit()));
		connect(ui86.pbSave,SIGNAL(clicked()),this,SLOT(OnSaveAndExit()));
	}
	this->setAttribute(Qt::WA_DeleteOnClose,true);

	FillGUI();

	this->setFixedSize(this->width(),this->height());
	this->setStyleSheet(clsHelperClass::LoadStyleSheet(this));
}

qtDLGRegEdit::~qtDLGRegEdit()
{

}

void qtDLGRegEdit::FillGUI()
{
#ifdef _AMD64_
	if(_bIs64)
	{
		CONTEXT *pProcContext = (CONTEXT *)_pProcessContext;
		ui64.lineRAX->setText(QString("%1").arg(pProcContext->Rax,16,16,QChar('0')));
		ui64.lineRBX->setText(QString("%1").arg(pProcContext->Rbx,16,16,QChar('0')));
		ui64.lineRCX->setText(QString("%1").arg(pProcContext->Rcx,16,16,QChar('0')));
		ui64.lineRDX->setText(QString("%1").arg(pProcContext->Rdx,16,16,QChar('0')));
		ui64.lineRSP->setText(QString("%1").arg(pProcContext->Rsp,16,16,QChar('0')));
		ui64.lineRSI->setText(QString("%1").arg(pProcContext->Rsi,16,16,QChar('0')));
		ui64.lineRDI->setText(QString("%1").arg(pProcContext->Rdi,16,16,QChar('0')));
		ui64.lineRIP->setText(QString("%1").arg(pProcContext->Rip,16,16,QChar('0')));
		ui64.lineRBP->setText(QString("%1").arg(pProcContext->Rbp,16,16,QChar('0')));
		ui64.lineR8->setText(QString("%1").arg(pProcContext->R8,16,16,QChar('0')));
		ui64.lineR9->setText(QString("%1").arg(pProcContext->R9,16,16,QChar('0')));
		ui64.lineR10->setText(QString("%1").arg(pProcContext->R10,16,16,QChar('0')));
		ui64.lineR11->setText(QString("%1").arg(pProcContext->R11,16,16,QChar('0')));
		ui64.lineR12->setText(QString("%1").arg(pProcContext->R12,16,16,QChar('0')));
		ui64.lineR13->setText(QString("%1").arg(pProcContext->R13,16,16,QChar('0')));
		ui64.lineR14->setText(QString("%1").arg(pProcContext->R14,16,16,QChar('0')));
		ui64.lineR15->setText(QString("%1").arg(pProcContext->R15,16,16,QChar('0')));
		ui64.lineEFlags->setText(QString("%1").arg(pProcContext->EFlags,16,16,QChar('0')));
	}
	else
	{
		WOW64_CONTEXT *pProcContext = (WOW64_CONTEXT *)_pProcessContext;
		ui86.lineEAX->setText(QString("%1").arg(pProcContext->Eax,8,16,QChar('0')));
		ui86.lineEBX->setText(QString("%1").arg(pProcContext->Ebx,8,16,QChar('0')));
		ui86.lineECX->setText(QString("%1").arg(pProcContext->Ecx,8,16,QChar('0')));
		ui86.lineEDX->setText(QString("%1").arg(pProcContext->Edx,8,16,QChar('0')));
		ui86.lineESP->setText(QString("%1").arg(pProcContext->Esp,8,16,QChar('0')));
		ui86.lineESI->setText(QString("%1").arg(pProcContext->Esi,8,16,QChar('0')));
		ui86.lineEDI->setText(QString("%1").arg(pProcContext->Edi,8,16,QChar('0')));
		ui86.lineEIP->setText(QString("%1").arg(pProcContext->Eip,8,16,QChar('0')));
		ui86.lineEBP->setText(QString("%1").arg(pProcContext->Ebp,8,16,QChar('0')));
		ui86.lineEFlags->setText(QString("%1").arg(pProcContext->EFlags,8,16,QChar('0')));
	}
#else
	CONTEXT *pProcContext = (CONTEXT *)_pProcessContext;
	ui86.lineEAX->setText(QString("%1").arg(pProcContext->Eax,8,16,QChar('0')));
	ui86.lineEBX->setText(QString("%1").arg(pProcContext->Ebx,8,16,QChar('0')));
	ui86.lineECX->setText(QString("%1").arg(pProcContext->Ecx,8,16,QChar('0')));
	ui86.lineEDX->setText(QString("%1").arg(pProcContext->Edx,8,16,QChar('0')));
	ui86.lineESP->setText(QString("%1").arg(pProcContext->Esp,8,16,QChar('0')));
	ui86.lineESI->setText(QString("%1").arg(pProcContext->Esi,8,16,QChar('0')));
	ui86.lineEDI->setText(QString("%1").arg(pProcContext->Edi,8,16,QChar('0')));
	ui86.lineEIP->setText(QString("%1").arg(pProcContext->Eip,8,16,QChar('0')));
	ui86.lineEBP->setText(QString("%1").arg(pProcContext->Ebp,8,16,QChar('0')));
	ui86.lineEFlags->setText(QString("%1").arg(pProcContext->EFlags,8,16,QChar('0')));
#endif
}

void qtDLGRegEdit::OnExit()
{
	close();
}

void qtDLGRegEdit::OnSaveAndExit()
{
	DWORD dwEFlags = NULL;
#ifdef _AMD64_
	if(_bIs64)
	{
		CONTEXT *pProcContext = (CONTEXT *)_pProcessContext;
		pProcContext->Rax = ui64.lineRAX->text().toULongLong(0,16);
		pProcContext->Rbx = ui64.lineRBX->text().toULongLong(0,16);
		pProcContext->Rcx = ui64.lineRCX->text().toULongLong(0,16);
		pProcContext->Rdx = ui64.lineRDX->text().toULongLong(0,16);
		pProcContext->Rsp = ui64.lineRSP->text().toULongLong(0,16);
		pProcContext->Rsi = ui64.lineRSI->text().toULongLong(0,16);
		pProcContext->Rdi = ui64.lineRDI->text().toULongLong(0,16);
		pProcContext->Rip = ui64.lineRIP->text().toULongLong(0,16);
		pProcContext->Rbp = ui64.lineRBP->text().toULongLong(0,16);
		pProcContext->R8 = ui64.lineR8->text().toULongLong(0,16);
		pProcContext->R9 = ui64.lineR9->text().toULongLong(0,16);
		pProcContext->R10 = ui64.lineR10->text().toULongLong(0,16);
		pProcContext->R11 = ui64.lineR11->text().toULongLong(0,16);
		pProcContext->R12 = ui64.lineR12->text().toULongLong(0,16);
		pProcContext->R13 = ui64.lineR13->text().toULongLong(0,16);
		pProcContext->R14 = ui64.lineR14->text().toULongLong(0,16);
		pProcContext->R15 = ui64.lineR15->text().toULongLong(0,16);
		pProcContext->EFlags = ui64.lineEFlags->text().toULongLong(0,16);

	}
	else
	{
		WOW64_CONTEXT *pProcContext = (WOW64_CONTEXT *)_pProcessContext;
		pProcContext->Eax = ui86.lineEAX->text().toULongLong(0,16);
		pProcContext->Ebx = ui86.lineEBX->text().toULongLong(0,16);
		pProcContext->Ecx = ui86.lineECX->text().toULongLong(0,16);
		pProcContext->Edx = ui86.lineEDX->text().toULongLong(0,16);
		pProcContext->Esp = ui86.lineESP->text().toULongLong(0,16);
		pProcContext->Esi = ui86.lineESI->text().toULongLong(0,16);
		pProcContext->Edi = ui86.lineEDI->text().toULongLong(0,16);
		pProcContext->Eip = ui86.lineEIP->text().toULongLong(0,16);
		pProcContext->Ebp = ui86.lineEBP->text().toULongLong(0,16);
		pProcContext->EFlags = ui86.lineEFlags->text().toULongLong(0,16);
	}
#else
	CONTEXT *pProcContext = (CONTEXT *)_pProcessContext;
	pProcContext->Eax = ui86.lineEAX->text().toULongLong(0,16);
	pProcContext->Ebx = ui86.lineEBX->text().toULongLong(0,16);
	pProcContext->Ecx = ui86.lineECX->text().toULongLong(0,16);
	pProcContext->Edx = ui86.lineEDX->text().toULongLong(0,16);
	pProcContext->Esp = ui86.lineESP->text().toULongLong(0,16);
	pProcContext->Esi = ui86.lineESI->text().toULongLong(0,16);
	pProcContext->Edi = ui86.lineEDI->text().toULongLong(0,16);
	pProcContext->Eip = ui86.lineEIP->text().toULongLong(0,16);
	pProcContext->Ebp = ui86.lineEBP->text().toULongLong(0,16);
	pProcContext->EFlags = ui86.lineEFlags->text().toULongLong(0,16);
#endif

	emit OnUpdateRegView();

	MessageBoxW(NULL,L"Registers got saved!",L"Nanomite",MB_OK);
}