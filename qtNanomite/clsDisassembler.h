#ifndef CLSDISAS_H
#define CLSDISAS_H

#include <Windows.h>

#include <QtCore>
#include <QtGui>

class clsDisassembler: public QThread
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

	clsDisassembler();
	~clsDisassembler();

	bool InsertNewDisassembly(HANDLE hProc,quint64 dwEIP);

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
	
	void SyntaxHighLight(QTableWidgetItem *newItem);

protected:
	void run();
};

#endif