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

/**
* @file qtDLGAssembler.h
* @brief Displaying the assembler widget
*/
class qtDLGAssembler : public QWidget, public Ui_qtDLGAssemblerClass
{
	Q_OBJECT

public:
	/**
	* @brief Responsible for initializing and showing the GUI of the assembler dialog.
	* @param parent Takes the a QWidget pointer of the calling QObject
	* @param flags A value of the Qt::WFlags enum which defines how the Dialog is shown
	* @param processHandle Takes the handle of the process which will be patched
	* @param instructionOffset The offset of the instruction which will be modified
	* @param currentInstruction The current instruction which will be modified
	* @param pCurrentDisassembler A pointer to the current disassembler interface
	* @param is64Bit A bool which is true if the process to be patched is x64
	*
	* @return no
	*/
	qtDLGAssembler(QWidget *parent, Qt::WFlags flags, HANDLE processHandle, quint64 instructionOffset, QString currentInstruction, clsDisassembler *pCurrentDisassembler, bool is64Bit);

	/**
	* @brief Does not need to do something at the current stage
	*
	* @return no
	*/
	~qtDLGAssembler();

signals:
	/**
	* @brief A Qt signal which is send if the new opcodes has been inserted to reload the gui / disassembler view
	*
	* @return no
	*/
	void OnReloadDebugger();

private: 
	HANDLE			m_processHandle; /* stores the process handle which will be patched */
	quint64			m_instructionOffset; /* stores the current instruction offset */
	clsDisassembler *m_pCurrentDisassembler; /* stores the disassembler interface */
	bool			m_is64Bit; /* stores if the process is x64 or not*/

private slots:
	/**
	* @brief A qt slot which is triggered when the user press return in the line edit
	*
	* @return no
	*/
	void InsertNewInstructions();
};

#endif
