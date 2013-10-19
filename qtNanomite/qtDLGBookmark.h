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
#ifndef QTDLGBOOKMARK_H
#define QTDLGBOOKMARK_H

#include "ui_qtDLGBookmark.h"

#include <QList>

#include <Windows.h>

struct BookmarkData
{
	int		bookmarkPID;
	quint64 bookmarkOffset;
	quint64 bookmarkBaseOffset;
	QString	bookmarkComment;
	QString bookmarkModule;
	QString bookmarkProcessModule;
};

class qtDLGBookmark : public QWidget, public Ui_qtDLGBookmarkClass
{
	Q_OBJECT

public:
	qtDLGBookmark(QWidget *parent = 0, Qt::WFlags flags = 0);
	~qtDLGBookmark();

	static bool BookmarkAdd(int processID, quint64 bookmarkOffset, QString bookmarkComment = "");
	static bool BookmarkRemove(quint64 bookmarkOffset);
	
	static QString BookmarkGetComment(int processID, quint64 bookmarkOffset);
	
	static void BookmarkClear();

	static QList<BookmarkData> BookmarkGetList();

	void BookmarkInsertFromProjectFile(BookmarkData newBookmark);

signals:
	void ShowInDisassembler(quint64 bookmarkOffset);

public slots:
	void UpdateBookmarks(QString fileName, int processID);
	void BookmarkUpdateOffsets(HANDLE processHandle, int processID);

private:
	static qtDLGBookmark *pThis;

	QList<BookmarkData> m_bookmarkData;

	void CleanIfOffsetLoaded(quint64 bookmarkOffset);

private slots:
	void CellDataChanged(int,int);
	void SendToDisassembler(QTableWidgetItem *);
	void RemoveSelectedBookmark();
	void UpdateDisplay();
};

#endif