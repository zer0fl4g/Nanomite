#ifndef CLSDISAS_H
#define CLSDISAS_H

#include <Windows.h>

#include <QtCore>
#include <QtGui>

struct DisAsDataRow
{
	quint64 itemStyle;
	QString Offset;
	QString OpCodes;
	QString ASM;
	QString Comment;
};

class clsDisassembler: public QThread
{
	Q_OBJECT

public:
	QMap<QString,DisAsDataRow> SectionDisAs;

	clsDisassembler();
	~clsDisassembler();

	bool InsertNewDisassembly(HANDLE hProc,quint64 dwEIP,bool bClear = false);

signals:
	void DisAsFinished(quint64 dwEIP);

private:
	HANDLE	_hProc;
	quint64 _dwEIP,
			_dwStartOffset,
			_dwEndOffset;
	bool	_bEndOfSection,
			_bStartOfSection;

	bool IsNewInsertNeeded();
	bool IsNewInsertPossible();
	
protected:
	void run();
};

#endif