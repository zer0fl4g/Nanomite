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
	quint64 Offset;
	QString OpCodes;
	QString ASM;
	QString Comment;
};

class clsDisassembler: public QThread
{
	Q_OBJECT

public:
	QMap<quint64,DisAsDataRow> SectionDisAs;

	clsDisassembler();
	~clsDisassembler();

	bool InsertNewDisassembly(HANDLE hProc,quint64 dwEIP,bool bClear = false);
	bool GetPageRangeForOffset(quint64 IP, quint64 &PageBase, quint64 &PageEnd);

signals:
	void DisAsFinished(quint64 dwEIP);

private:
	HANDLE	m_processHandle;
	quint64 m_searchedOffset,
			m_startOffset,
			m_endOffset,
			m_startPage;

	bool m_isWorking;

	bool IsNewInsertNeeded();
	bool IsNewInsertPossible();

private slots:
	void OnThreadFinished();

protected:
	void run();
};

#endif