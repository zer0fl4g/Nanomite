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
#ifndef CLSFUNCTIONSWORKER_H
#define CLSFUNCTIONSWORKER_H

#include <Windows.h>

#include <QThread>
#include <QMap>

struct FunctionData
{
	quint64 FunctionOffset;
	quint64 FunctionSize;
	int PID;
	QString functionSymbol;
};

struct FunctionProcessingData
{
	PTCHAR currentModule;
	HANDLE hProc;
	int PID;
};

struct JumpData
{
	quint64 jumpOffset;
	quint64 jumpTarget;
};

class clsFunctionsViewWorker : public QThread
{
	Q_OBJECT

public:
	QList<FunctionData>	functionList;
	
	clsFunctionsViewWorker(QList<FunctionProcessingData> dataForProcessing);
	~clsFunctionsViewWorker();

private:
	QList<FunctionProcessingData> m_processingData;

	void GetValidMemoryParts(PTCHAR lpCurrentName,HANDLE hProc);
	void ParseMemoryRangeForCallFunctions(HANDLE hProc,quint64 BaseAddress,quint64 Size);
	void ParseMemoryRangeForJumpFunctions(HANDLE hProc,quint64 BaseAddress,quint64 Size);
	void InsertSymbolsIntoLists(HANDLE hProc);
	
	DWORD GetFunctionSizeFromCallPoint(HANDLE processHandle, quint64 functionOffset, quint64 pageEnd);

protected:
	void run();
};

bool OffsetLessThan(const FunctionData &f1, const FunctionData &f2);

#endif
