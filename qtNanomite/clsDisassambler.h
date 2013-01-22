#ifndef CLSDISAS_H
#define CLSDISAS_H

#include <Windows.h>

#include <QtCore>
#include <QtGui>

class clsDisassambler: public QThread
{
	Q_OBJECT

public:
	struct DisAsDataRow
	{
		QString Offset;
		QString OpCodes;
		QString ASM;
		QString Comment;
	};

	QMap<QString,DisAsDataRow> SectionDisAs;

	clsDisassambler();
	~clsDisassambler();

	bool InsertNewDisassambly(HANDLE hProc,DWORD64 dwEIP);

signals:
	void DisAsFinished(DWORD64 dwEIP);

private:
	HANDLE	_hProc;
	DWORD64 _dwEIP;
	DWORD64 _dwStartOffset;
	DWORD64 _dwEndOffset;

	bool CalcIfNewInsertIsNeeded();
	bool CalcMemoryRange(DWORD64 *StartOffset,DWORD64 *EndOffset);
	
	void SyntaxHighLight(QTableWidgetItem *newItem);

protected:
	void run();
};

#endif