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
#ifndef QTDLGREGISTERS_H
#define QTDLGREGISTERS_H

#include "clsDebugger\clsDebugger.h"

#include <QDockWidget>

#include "ui_qtDLGRegisters.h"

#include <stdint.h>

class qtDLGRegisters : public QDockWidget, public Ui_qtDLGRegisters
{
	Q_OBJECT

public:
	qtDLGRegisters(QWidget *parent = 0);
	~qtDLGRegisters();

	void LoadRegView(clsDebugger *coreDebugger);

public slots:
	void LoadRegView();

signals:
	void OnDisplayDisassembly(quint64 dwEIP);

private:
	typedef struct {
		DWORD64 low;
		DWORD64 high;
	} uint128_t;

	int _iSelectedRow;

	void PrintValueInTable(QString regName, QString regValue);
	double readFloat80(const uint8_t buffer[10]);

private slots:
	void OnContextMenu(QPoint);
	void OnChangeRequest(QTableWidgetItem *pItem);
	void MenuCallback(QAction* pAction);
};

#endif // QTDLGREGISTERS_H
