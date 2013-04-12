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
