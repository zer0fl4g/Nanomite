#ifndef CLSYMBOLANDSYNTAX_H
#define CLSYMBOLANDSYNTAX_H

#include "clsDisassembler.h"

#define COLOR_BP		1
#define COLOR_CALLS		2
#define COLOR_JUMP		4
#define COLOR_MOVE		8
#define COLOR_STACK		16

class clsSymbolAndSyntax
{
public:
	clsSymbolAndSyntax(HANDLE hProc);
	~clsSymbolAndSyntax();

	bool CreateDataForRow(DisAsDataRow *pDataRow); 
	bool IsOffsetAnBP(quint64 Offset);

	QString CreateSymbols(quint64 FuncOffset);
	
private:
	HANDLE _hProc;
};

#endif