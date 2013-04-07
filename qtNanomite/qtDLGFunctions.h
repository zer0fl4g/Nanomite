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

	QList<FunctionData>	functionList;

	void GetValidMemoryParts(PTCHAR lpCurrentName,HANDLE hProc);
	void ParseMemoryRangeForFunctions(HANDLE hProc,quint64 BaseAddress,quint64 Size);
	void InsertSymbolsIntoLists(HANDLE hProc);
	void DisplayFunctionLists();

	quint64 GetPossibleFunctionEnding(quint64 BaseAddress,quint64 Size,DWORD SearchPattern,LPVOID lpBuffer);

	QList<FunctionData> GetPossibleFunctionBeginning(quint64 StartOffset,quint64 Size,DWORD SearchPattern,LPVOID lpBuffer);
};

#endif
