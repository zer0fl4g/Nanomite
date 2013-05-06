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
