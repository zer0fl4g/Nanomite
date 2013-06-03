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

	bool IsNewInsertNeeded();
	bool IsNewInsertPossible();
	
protected:
	void run();
};

#endif