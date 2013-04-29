#ifndef QTDLGASSEMBLER_H
#define QTDLGASSEMBLER_H

#include "ui_qtDLGAssembler.h"

#include <Windows.h>

#include "clsDisassembler.h"

class qtDLGAssembler : public QWidget, public Ui_qtDLGAssemblerClass
{
	Q_OBJECT

public:
	qtDLGAssembler(QWidget *parent, Qt::WFlags flags,HANDLE hProc,quint64 InstructionOffset,clsDisassembler *pDisAs, bool Is64Bit);
	~qtDLGAssembler();

signals:
	void OnReloadDebugger();

private: 
	HANDLE _hProc;
	quint64 _InstructionOffset;
	clsDisassembler *_pDisAs;
	bool _Is64Bit;

	private slots:
		void InsertNewInstructions();
};

#endif
