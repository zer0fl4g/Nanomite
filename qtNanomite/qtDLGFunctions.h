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
#ifndef QTDLGFUNCTIONS_H
#define QTDLGFUNCTIONS_H

#include "ui_qtDLGFunctions.h"

#include <Windows.h>
#include <QtCore>
#include <string>

struct FunctionData
{
	quint64 FunctionOffset;
	quint64 FunctionSize;
	WORD PID;
	QString functionSymbol;
};

class qtDLGFunctions : public QWidget, public Ui_qtDLGFunctionsClass
{
	Q_OBJECT

public:
	qtDLGFunctions(QWidget *parent = 0, Qt::WFlags flags = 0,qint32 iPID = 0);
	~qtDLGFunctions();

signals:
	void ShowInDisAs(quint64 Offset);

private:
	int _iPID;
	int _iSelectedRow;

	QList<FunctionData>	functionList;

	void GetValidMemoryParts(PTCHAR lpCurrentName,HANDLE hProc);
	void ParseMemoryRangeForFunctions(HANDLE hProc,quint64 BaseAddress,quint64 Size);
	void InsertSymbolsIntoLists(HANDLE hProc,WORD PID);
	void DisplayFunctionLists();

	quint64 GetPossibleFunctionEnding(quint64 BaseAddress,quint64 Size,DWORD SearchPattern,LPVOID lpBuffer,int SpaceLen);

	QList<FunctionData> GetPossibleFunctionBeginning(quint64 StartOffset,quint64 Size,DWORD SearchPattern,LPVOID lpBuffer,int SpaceLen);

	private slots:
		void OnCustomContextMenu(QPoint qPoint);
		void MenuCallback(QAction* pAction);
};

#endif
