#ifndef QTDLGASSEMBLER_H
#define QTDLGASSEMBLER_H

#include "ui_qtDLGAssembler.h"

#include <Windows.h>

class qtDLGAssembler : public QWidget, public Ui_qtDLGAssemblerClass
{
	Q_OBJECT

public:
	qtDLGAssembler(QWidget *parent, Qt::WFlags flags,HANDLE hProc);
	~qtDLGAssembler();

private: 
	HANDLE _hProc;

};

#endif
