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
#include "qtDLGRegisters.h"
#include "qtDLGNanomite.h"
#include "qtDLGRegEdit.h"

#include "clsMemManager.h"
#include "clsAPIImport.h"

#include <QClipboard>
#include <QMenu>

#include <cmath>
#include <limits>

qtDLGRegisters::qtDLGRegisters(QWidget *parent)
	: QDockWidget(parent)
{
	setupUi(this);

	generalRegs->setLayout(verticalLayout);
	fpuRegs->setLayout(verticalLayout_2);
	mmxRegs->setLayout(verticalLayout_3);
	sseRegs->setLayout(verticalLayout_4);

	// List Register
	tblRegView->horizontalHeader()->resizeSection(0,75);
	tblRegView->horizontalHeader()->resizeSection(1,100);
	tblRegView->horizontalHeader()->setFixedHeight(21);

	tblFPU->horizontalHeader()->resizeSection(0,75);
	tblFPU->horizontalHeader()->resizeSection(1,100);
	tblFPU->horizontalHeader()->setFixedHeight(21);

	tblMMX->horizontalHeader()->resizeSection(0,75);
	tblMMX->horizontalHeader()->resizeSection(1,100);
	tblMMX->horizontalHeader()->setFixedHeight(21);

	tblSSE->horizontalHeader()->resizeSection(0,75);
	tblSSE->horizontalHeader()->resizeSection(1,100);
	tblSSE->horizontalHeader()->setFixedHeight(21);

	tabWidget->removeTab(3);
	tabWidget->removeTab(2);

	connect(tblRegView,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnContextMenu(QPoint)));
	connect(tblRegView,SIGNAL(itemDoubleClicked(QTableWidgetItem *)),this,SLOT(OnChangeRequest(QTableWidgetItem *)));
}

qtDLGRegisters::~qtDLGRegisters()
{

}

void qtDLGRegisters::OnContextMenu(QPoint qPoint)
{
	QMenu menu;

	m_iSelectedRow = tblRegView->indexAt(qPoint).row();
	if(m_iSelectedRow < 0) return;


	menu.addAction(new QAction("Send to StackView",this));
	menu.addAction(new QAction("Send to Disassembler",this));
	QMenu *submenu = menu.addMenu("Copy to Clipboard");
	submenu->addAction(new QAction("Line",this));
	submenu->addAction(new QAction("Value",this));

	menu.addMenu(submenu);
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));

	menu.exec(QCursor::pos());
}

void qtDLGRegisters::MenuCallback(QAction* pAction)
{
	qtDLGNanomite *pMainWindow = qtDLGNanomite::GetInstance();

	if(!pMainWindow->coreDebugger->GetDebuggingState()) return;

	if(QString().compare(pAction->text(),"Send to Disassembler") == 0)
	{
		if(!pMainWindow->coreDisAs->InsertNewDisassembly(pMainWindow->coreDebugger->GetCurrentProcessHandle(),tblRegView->item(m_iSelectedRow,1)->text().toULongLong(0,16)))
			emit OnDisplayDisassembly(tblRegView->item(m_iSelectedRow,1)->text().toULongLong(0,16));
	}
	else if(QString().compare(pAction->text(),"Send to StackView") == 0)
	{
#ifdef _AMD64_
		BOOL bIsWOW64 = false;
		if(clsAPIImport::pIsWow64Process)
			clsAPIImport::pIsWow64Process(pMainWindow->coreDebugger->GetCurrentProcessHandle(),&bIsWOW64);

		if(bIsWOW64)
			pMainWindow->stackView->LoadStackView(tblRegView->item(m_iSelectedRow,1)->text().toULongLong(0,16),4);
		else
			pMainWindow->stackView->LoadStackView(tblRegView->item(m_iSelectedRow,1)->text().toULongLong(0,16),8);
#else
		pMainWindow->stackView->LoadStackView(tblRegView->item(m_iSelectedRow,1)->text().toULongLong(0,16),4);
#endif
	}
	else if(QString().compare(pAction->text(),"Line") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(QString("%1:%2").arg(tblRegView->item(m_iSelectedRow,0)->text()).arg(tblRegView->item(m_iSelectedRow,1)->text()));
	}
	else if(QString().compare(pAction->text(),"Value") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblRegView->item(m_iSelectedRow,1)->text());
	}
}

void qtDLGRegisters::OnChangeRequest(QTableWidgetItem *pItem)
{
	qtDLGNanomite *pMainWindow = qtDLGNanomite::GetInstance();

	if(!pMainWindow->coreDebugger->GetDebuggingState())
		return;

	qtDLGRegEdit *newRegEditWindow;
#ifdef _AMD64_
	BOOL bIsWOW64 = false;
	if(clsAPIImport::pIsWow64Process)
		clsAPIImport::pIsWow64Process(pMainWindow->coreDebugger->GetCurrentProcessHandle(),&bIsWOW64);

	if(bIsWOW64)
		newRegEditWindow = new qtDLGRegEdit(this,Qt::Window,&pMainWindow->coreDebugger->wowProcessContext,false);
	else
		newRegEditWindow = new qtDLGRegEdit(this,Qt::Window,&pMainWindow->coreDebugger->ProcessContext,true);			
#else
	newRegEditWindow = new qtDLGRegEdit(this,Qt::Window,&pMainWindow->coreDebugger->ProcessContext,false);			
#endif	

	connect(newRegEditWindow,SIGNAL(OnUpdateRegView()),this,SLOT(LoadRegView()));
	newRegEditWindow->exec();
}

void qtDLGRegisters::LoadRegView()
{
	LoadRegView(qtDLGNanomite::GetInstance()->coreDebugger);
}

void qtDLGRegisters::LoadRegView(clsDebugger *coreDebugger)
{
	tblRegView->setRowCount(0);
	tblFPU->setRowCount(0);
	tblMMX->setRowCount(0);
	tblSSE->setRowCount(0);

	tabWidget->removeTab(3);
	tabWidget->removeTab(2);

	DWORD dwEFlags = NULL;

#ifdef _AMD64_
	BOOL bIsWOW64 = false;

	if(clsAPIImport::pIsWow64Process)
		clsAPIImport::pIsWow64Process(coreDebugger->GetCurrentProcessHandle(),&bIsWOW64);

	if(bIsWOW64)
	{
		dwEFlags = coreDebugger->wowProcessContext.EFlags;

		PrintValueInTable(tblRegView,"EAX",QString("%1").arg(coreDebugger->wowProcessContext.Eax,16,16,QChar('0')));
		PrintValueInTable(tblRegView,"EBX",QString("%1").arg(coreDebugger->wowProcessContext.Ebx,16,16,QChar('0')));
		PrintValueInTable(tblRegView,"ECX",QString("%1").arg(coreDebugger->wowProcessContext.Ecx,16,16,QChar('0')));
		PrintValueInTable(tblRegView,"EDX",QString("%1").arg(coreDebugger->wowProcessContext.Edx,16,16,QChar('0')));
		PrintValueInTable(tblRegView,"ESP",QString("%1").arg(coreDebugger->wowProcessContext.Esp,16,16,QChar('0')));
		PrintValueInTable(tblRegView,"EBP",QString("%1").arg(coreDebugger->wowProcessContext.Ebp,16,16,QChar('0')));
		PrintValueInTable(tblRegView,"ESI",QString("%1").arg(coreDebugger->wowProcessContext.Esi,16,16,QChar('0')));
		PrintValueInTable(tblRegView,"EDI",QString("%1").arg(coreDebugger->wowProcessContext.Edi,16,16,QChar('0')));
		PrintValueInTable(tblRegView,"EIP",QString("%1").arg(coreDebugger->wowProcessContext.Eip,16,16,QChar('0')));
		PrintValueInTable(tblRegView,"SegCs",QString("%1").arg(coreDebugger->wowProcessContext.SegCs,16,16,QChar('0')));
		PrintValueInTable(tblRegView,"SegDs",QString("%1").arg(coreDebugger->wowProcessContext.SegDs,16,16,QChar('0')));
		PrintValueInTable(tblRegView,"SegEs",QString("%1").arg(coreDebugger->wowProcessContext.SegEs,16,16,QChar('0')));
		PrintValueInTable(tblRegView,"SegFs",QString("%1").arg(coreDebugger->wowProcessContext.SegFs,16,16,QChar('0')));
		PrintValueInTable(tblRegView,"SegGs",QString("%1").arg(coreDebugger->wowProcessContext.SegGs,16,16,QChar('0')));
		PrintValueInTable(tblRegView,"SegSs",QString("%1").arg(coreDebugger->wowProcessContext.SegSs,16,16,QChar('0')));
						
		for (int i = 0; i < 8; i++) {
			double value = readFloat80(&coreDebugger->wowProcessContext.FloatSave.RegisterArea[i * 10]);
			PrintValueInTable(tblFPU,QString("ST(%1)").arg(i), QString("%1").arg(value));
		}
		
		if (IsProcessorFeaturePresent(PF_MMX_INSTRUCTIONS_AVAILABLE) == true) {
			tabWidget->insertTab(2, mmxRegs, "MMX");

			DWORD64* pMMX;
			for (int i = 0; i < 8; i++) {
				pMMX = (DWORD64*)&coreDebugger->wowProcessContext.FloatSave.RegisterArea[i * 10];
				PrintValueInTable(tblMMX,QString("MMX%1").arg(i), QString("%1").arg(*pMMX, 16, 16, QChar('0')));
			}
		}
		else
			tabWidget->removeTab(2);

		if (IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE) == true) {
			tabWidget->insertTab(3, sseRegs, "SSE");

			uint128_t *pXMM;
			for (int i = 0; i < 8; i++) {
				pXMM = (uint128_t*)&coreDebugger->wowProcessContext.ExtendedRegisters[(10 + i) * 16];
				PrintValueInTable(	tblSSE,
									QString("XMM%1").arg(i), 
									QString("%1 %2").arg((*pXMM).low, 16, 16, QChar('0')).arg((*pXMM).high, 16, 16, QChar('0')));
			}		
		}
		else
			tabWidget->removeTab(3);

		// EFlags
		PrintValueInTable(tblRegView,"EFlags", QString("%1").arg(coreDebugger->wowProcessContext.EFlags, 16, 16, QChar('0')));
	}
	else
	{
		dwEFlags = coreDebugger->ProcessContext.EFlags;
		
		PrintValueInTable(tblRegView,"RAX",QString("%1").arg(coreDebugger->ProcessContext.Rax,16,16,QChar('0')));
		PrintValueInTable(tblRegView,"RBX",QString("%1").arg(coreDebugger->ProcessContext.Rbx,16,16,QChar('0')));
		PrintValueInTable(tblRegView,"RCX",QString("%1").arg(coreDebugger->ProcessContext.Rcx,16,16,QChar('0')));
		PrintValueInTable(tblRegView,"RDX",QString("%1").arg(coreDebugger->ProcessContext.Rdx,16,16,QChar('0')));
		PrintValueInTable(tblRegView,"RSP",QString("%1").arg(coreDebugger->ProcessContext.Rsp,16,16,QChar('0')));
		PrintValueInTable(tblRegView,"RBP",QString("%1").arg(coreDebugger->ProcessContext.Rbp,16,16,QChar('0')));
		PrintValueInTable(tblRegView,"RSI",QString("%1").arg(coreDebugger->ProcessContext.Rsi,16,16,QChar('0')));
		PrintValueInTable(tblRegView,"RDI",QString("%1").arg(coreDebugger->ProcessContext.Rdi,16,16,QChar('0')));
		PrintValueInTable(tblRegView,"RIP",QString("%1").arg(coreDebugger->ProcessContext.Rip,16,16,QChar('0')));
		PrintValueInTable(tblRegView,"R8",QString("%1").arg(coreDebugger->ProcessContext.R8,16,16,QChar('0')));
		PrintValueInTable(tblRegView,"R9",QString("%1").arg(coreDebugger->ProcessContext.R9,16,16,QChar('0')));
		PrintValueInTable(tblRegView,"R10",QString("%1").arg(coreDebugger->ProcessContext.R10,16,16,QChar('0')));
		PrintValueInTable(tblRegView,"R11",QString("%1").arg(coreDebugger->ProcessContext.R11,16,16,QChar('0')));
		PrintValueInTable(tblRegView,"R12",QString("%1").arg(coreDebugger->ProcessContext.R12,16,16,QChar('0')));
		PrintValueInTable(tblRegView,"R13",QString("%1").arg(coreDebugger->ProcessContext.R13,16,16,QChar('0')));
		PrintValueInTable(tblRegView,"R14",QString("%1").arg(coreDebugger->ProcessContext.R14,16,16,QChar('0')));
		PrintValueInTable(tblRegView,"R15",QString("%1").arg(coreDebugger->ProcessContext.R15,16,16,QChar('0')));		
		PrintValueInTable(tblRegView,"SegCs",QString("%1").arg(coreDebugger->ProcessContext.SegCs,16,16,QChar('0')));
		PrintValueInTable(tblRegView,"SegDs",QString("%1").arg(coreDebugger->ProcessContext.SegDs,16,16,QChar('0')));
		PrintValueInTable(tblRegView,"SegEs",QString("%1").arg(coreDebugger->ProcessContext.SegEs,16,16,QChar('0')));
		PrintValueInTable(tblRegView,"SegFs",QString("%1").arg(coreDebugger->ProcessContext.SegFs,16,16,QChar('0')));
		PrintValueInTable(tblRegView,"SegGs",QString("%1").arg(coreDebugger->ProcessContext.SegGs,16,16,QChar('0')));
		PrintValueInTable(tblRegView,"SegSs",QString("%1").arg(coreDebugger->ProcessContext.SegSs,16,16,QChar('0')));
						
		for (int i = 0; i < 8; i++) {
			PrintValueInTable(	tblFPU,
								QString("ST(%1)").arg(i),
								QString("%1 %2").arg(coreDebugger->ProcessContext.FltSave.FloatRegisters[i].Low, 16, 16,QChar('0'))
												.arg(coreDebugger->ProcessContext.FltSave.FloatRegisters[i].High, 16, 16, QChar('0')));
		}

		if (IsProcessorFeaturePresent(PF_MMX_INSTRUCTIONS_AVAILABLE) == true) {
			tabWidget->insertTab(2, mmxRegs, "MMX");

			for (int i = 0; i < 8; i++) {
				PrintValueInTable(	tblMMX,
									QString("MMX%1").arg(i),
									QString("%1").arg(coreDebugger->ProcessContext.FltSave.FloatRegisters[i].Low, 16, 16, QChar('0')));
			}
		}
		else
			tabWidget->removeTab(2);

		if (IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE) == true) {
			tabWidget->insertTab(3, sseRegs, "SSE");

			for(int i = 0; i < 16; i++) {
				PrintValueInTable(	tblSSE,
									QString("XMM%1").arg(i), 
									QString("%1 %2").arg(coreDebugger->ProcessContext.FltSave.XmmRegisters[i].Low, 16, 16,QChar('0'))
													.arg(coreDebugger->ProcessContext.FltSave.XmmRegisters[i].High, 16, 16, QChar('0')));
			}
		}
		else
			tabWidget->removeTab(3);

		// EFlags
		PrintValueInTable(tblRegView,"EFlags",QString("%1").arg(coreDebugger->ProcessContext.EFlags,16,16,QChar('0')));
	}
#else
	dwEFlags = coreDebugger->ProcessContext.EFlags;

	PrintValueInTable(tblRegView,"EAX",QString("%1").arg(coreDebugger->ProcessContext.Eax,8,16,QChar('0')));
	PrintValueInTable(tblRegView,"EBX",QString("%1").arg(coreDebugger->ProcessContext.Ebx,8,16,QChar('0')));
	PrintValueInTable(tblRegView,"ECX",QString("%1").arg(coreDebugger->ProcessContext.Ecx,8,16,QChar('0')));
	PrintValueInTable(tblRegView,"EDX",QString("%1").arg(coreDebugger->ProcessContext.Edx,8,16,QChar('0')));
	PrintValueInTable(tblRegView,"ESP",QString("%1").arg(coreDebugger->ProcessContext.Esp,8,16,QChar('0')));
	PrintValueInTable(tblRegView,"EBP",QString("%1").arg(coreDebugger->ProcessContext.Ebp,8,16,QChar('0')));
	PrintValueInTable(tblRegView,"ESI",QString("%1").arg(coreDebugger->ProcessContext.Esi,8,16,QChar('0')));
	PrintValueInTable(tblRegView,"EDI",QString("%1").arg(coreDebugger->ProcessContext.Edi,8,16,QChar('0')));
	PrintValueInTable(tblRegView,"EIP",QString("%1").arg(coreDebugger->ProcessContext.Eip,8,16,QChar('0')));
	PrintValueInTable(tblRegView,"SegCs",QString("%1").arg(coreDebugger->ProcessContext.SegCs,8,16,QChar('0')));
	PrintValueInTable(tblRegView,"SegDs",QString("%1").arg(coreDebugger->ProcessContext.SegDs,8,16,QChar('0')));
	PrintValueInTable(tblRegView,"SegEs",QString("%1").arg(coreDebugger->ProcessContext.SegEs,8,16,QChar('0')));
	PrintValueInTable(tblRegView,"SegFs",QString("%1").arg(coreDebugger->ProcessContext.SegFs,8,16,QChar('0')));
	PrintValueInTable(tblRegView,"SegGs",QString("%1").arg(coreDebugger->ProcessContext.SegGs,8,16,QChar('0')));
	PrintValueInTable(tblRegView,"SegSs",QString("%1").arg(coreDebugger->ProcessContext.SegSs,8,16,QChar('0')));

	for (int i = 0; i < 8; i++) {
		double value = readFloat80(&coreDebugger->ProcessContext.FloatSave.RegisterArea[i * 10]);
		PrintValueInTable(tblFPU,QString("ST(%1)").arg(i), QString("%1").arg(value));	
	}

	if (IsProcessorFeaturePresent(PF_MMX_INSTRUCTIONS_AVAILABLE) == true) {
		tabWidget->insertTab(2, mmxRegs, "MMX");

		DWORD64* pMMX;
		for (int i = 0; i < 8; i++) {
			pMMX = (DWORD64*)&coreDebugger->ProcessContext.FloatSave.RegisterArea[i * 10];
			PrintValueInTable(tblMMX,QString("MMX%1").arg(i),QString("%1").arg(*pMMX, 16, 16, QChar('0')));
		}
	}
	else
		tabWidget->removeTab(2);

	if (IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE) == true) {
		tabWidget->insertTab(3, sseRegs, "SSE");

		uint128_t *pXMM;
		for (int i = 0; i < 8; i++) {
			pXMM = (uint128_t*)&coreDebugger->ProcessContext.ExtendedRegisters[(10 + i) * 16];
			PrintValueInTable(tblSSE,QString("XMM%1").arg(i), QString("%1 %2").arg((*pXMM).low, 16, 16, QChar('0')).arg((*pXMM).high, 16, 16, QChar('0')));
		}		
	}
	else
		tabWidget->removeTab(3);

	// EFlags
	PrintValueInTable(tblRegView,"EFlags",QString("%1").arg(coreDebugger->ProcessContext.EFlags,8,16,QChar('0')));
#endif

	BOOL bCF = false, // Carry Flag
		bPF = false, // Parity Flag
		bAF = false, // Auxiliarty carry flag
		bZF = false, // Zero Flag
		bSF = false, // Sign Flag
		bTF = false, // Trap Flag
		bIF = false, // Interrupt Flag
		bDF = false, // Direction Flag
		bOF = false; // Overflow Flag

	bOF = (dwEFlags & 0x800) ? true : false;
	bDF = (dwEFlags & 0x400) ? true : false;
	bTF = (dwEFlags & 0x100) ? true : false;
	bSF = (dwEFlags & 0x80) ? true : false;
	bZF = (dwEFlags & 0x40) ? true : false;
	bAF = (dwEFlags & 0x10) ? true : false;
	bPF = (dwEFlags & 0x4) ? true : false;
	bCF = (dwEFlags & 0x1) ? true : false;

	PrintValueInTable(tblRegView,"OF",QString("%1").arg(bOF));
	PrintValueInTable(tblRegView,"DF",QString("%1").arg(bDF));
	PrintValueInTable(tblRegView,"TF",QString("%1").arg(bTF));
	PrintValueInTable(tblRegView,"SF",QString("%1").arg(bSF));
	PrintValueInTable(tblRegView,"ZF",QString("%1").arg(bZF));
	PrintValueInTable(tblRegView,"AF",QString("%1").arg(bAF));
	PrintValueInTable(tblRegView,"PF",QString("%1").arg(bPF));
	PrintValueInTable(tblRegView,"CF",QString("%1").arg(bCF));
}

void qtDLGRegisters::PrintValueInTable(QTableWidget *pTable, QString regName, QString regValue)
{
	pTable->insertRow(pTable->rowCount());
	pTable->setItem(pTable->rowCount() - 1,0,new QTableWidgetItem(regName));
	pTable->setItem(pTable->rowCount() - 1,1,new QTableWidgetItem(regValue));
}

// FIXME: maybe rewrite this function
double qtDLGRegisters::readFloat80(const uint8_t buffer[10]) 
{
	 //80 bit floating point value according to IEEE-754:
    //1 bit sign, 15 bit exponent, 64 bit mantissa

    const uint16_t SIGNBIT    = 1 << 15;
    const uint16_t EXP_BIAS   = (1 << 14) - 1; // 2^(n-1) - 1 = 16383
    const uint16_t SPECIALEXP = (1 << 15) - 1; // all bits set
    const uint64_t HIGHBIT    = (uint64_t)1 << 63;
    const uint64_t QUIETBIT   = (uint64_t)1 << 62;

    // Extract sign, exponent and mantissa
    uint16_t exponent = *((uint16_t*)&buffer[8]);
    uint64_t mantissa = *((uint64_t*)&buffer[0]);

    double sign = (exponent & SIGNBIT) ? -1.0 : 1.0;
    exponent   &= ~SIGNBIT;

    // Check for undefined values
    if((!exponent && (mantissa & HIGHBIT)) || (exponent && !(mantissa & HIGHBIT))) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    // Check for special values (infinity, NaN)
    if(exponent == 0) {
        if(mantissa == 0) {
            return sign * 0.0;
        } else {
            // denormalized
        }
    } else if(exponent == SPECIALEXP) {
        if(!(mantissa & ~HIGHBIT)) {
            return sign * std::numeric_limits<double>::infinity();
        } else {
            if(mantissa & QUIETBIT) {
                return std::numeric_limits<double>::quiet_NaN();
            } else {
                return std::numeric_limits<double>::signaling_NaN();
            }
        }
    }

    //value = (-1)^s * (m / 2^63) * 2^(e - 16383)
    double significand = ((double)mantissa / ((uint64_t)1 << 63));
    return sign * ldexp(significand, exponent - EXP_BIAS);
}