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
#ifndef CLSYMBOLANDSYNTAX_H
#define CLSYMBOLANDSYNTAX_H

#include "clsDisassembler.h"

#define COLOR_BP		1
#define COLOR_CALLS		2
#define COLOR_JUMP		4
#define COLOR_MOVE		8
#define COLOR_STACK		16
#define COLOR_MATH		32

class clsSymbolAndSyntax
{
public:
	clsSymbolAndSyntax(HANDLE hProc);
	~clsSymbolAndSyntax();

	bool CreateDataForRow(DisAsDataRow *pDataRow); 
	bool IsEIP(quint64 Offset);

	QString CreateSymbols(quint64 FuncOffset);
	
private:
	HANDLE _hProc;
};

#endif