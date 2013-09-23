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
#ifndef QTDLGDISASSEMBLER_H
#define QTDLGDISASSEMBLER_H

#include "ui_qtDLGDisassembler.h"

#include "clsPEManager.h"
#include "clsDisassembler.h"
#include "clsSymbolAndSyntax.h"
#include "../clsDebugger/clsDebugger.h"

#include "qtDLGSourceViewer.h"

/**
* @file qtDLGDisassembler.h
* @brief Displaying stringdata from debugged file
*/
class qtDLGDisassembler : public QWidget, public Ui_qtDLGDisassemblerClass
{
	Q_OBJECT

public:
	/**
	* @brief Calls the disassembler.
	* @param parent Takes the a QWidget pointer of the calling QObject
	*
	* @return no
	*/
	qtDLGDisassembler(QWidget *parent);
	/**
	* @brief cleanup used data.
	*
	* @return no
	*/
	~qtDLGDisassembler();

	qtDLGSourceViewer *dlgSourceViewer;
	clsPEManager *PEManager;

signals:
	/**
	* @brief Emited when debugger breaks.
	*
	* @return no
	*/
	void OnDebuggerBreak();

public slots:
	/**
	* @brief 
	*
	* @return no
	*/
	void OnDisplayDisassembly(quint64 dwEIP);

private slots:
	void OnCustomDisassemblerContextMenu(QPoint qPoint);
	void OnDisAsReturnPressed();
	void OnDisAsReturn();
	void CustomDisassemblerMenuCallback(QAction*);
	void OnDisAsScroll(int iValue);
	void OnF2BreakPointPlace();
	void OnEditInstruction();

private:
	clsDebugger		*coreDebugger;
	clsDisassembler	*coreDisAs;
	
	qtNanomiteDisAsColorSettings *qtNanomiteDisAsColor;

	QList<quint64> m_offsetWalkHistory;

	QStringList m_searchedOffsetList;

	int m_iSelectedRow,
		m_maxRows;

	quint64 m_lastEIP;

	void resizeEvent(QResizeEvent *event);

	bool OnMoveUpOrDown(bool isUp, bool isPage = false);

protected:
	bool eventFilter(QObject *pOpject,QEvent *event);
};

#endif