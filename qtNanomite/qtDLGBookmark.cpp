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
#include "qtDLGBookmark.h"
#include "qtDLGNanomite.h"

#include "clsMemManager.h"
#include "clsHelperClass.h"

#include <QShortcut>

qtDLGBookmark *qtDLGBookmark::pThis = NULL;

qtDLGBookmark::qtDLGBookmark(QWidget *parent, Qt::WFlags flags) :
	QWidget(parent, flags)
{
	pThis = this;

	setupUi(this);
	setLayout(verticalLayout);

	// Init List
	tblBookmark->horizontalHeader()->resizeSection(0,150);
	tblBookmark->horizontalHeader()->resizeSection(1,125);
	tblBookmark->horizontalHeader()->setFixedHeight(21);

	connect(new QShortcut(QKeySequence("F5"), this), SIGNAL(activated()), this, SLOT(UpdateDisplay()));
	connect(new QShortcut(Qt::Key_Escape,this),SIGNAL(activated()),this,SLOT(close()));
	connect(new QShortcut(QKeySequence(QKeySequence::Delete), this), SIGNAL(activated()), this, SLOT(RemoveSelectedBookmark()));
	connect(tblBookmark,SIGNAL(itemDoubleClicked(QTableWidgetItem *)),this,SLOT(SendToDisassembler(QTableWidgetItem *)));
	connect(tblBookmark,SIGNAL(cellChanged(int,int)),this,SLOT(CellDataChanged(int,int)));
}

qtDLGBookmark::~qtDLGBookmark()
{
	pThis = NULL;
}

void qtDLGBookmark::UpdateDisplay()
{
	tblBookmark->blockSignals(true);

	tblBookmark->setRowCount(0);

	for(int i = 0; i < m_bookmarkData.size(); i++)
	{
		tblBookmark->insertRow(i);

		tblBookmark->setItem(i, 0, new QTableWidgetItem(m_bookmarkData.at(i).bookmarkModule));
		tblBookmark->setItem(i, 1, new QTableWidgetItem(QString("%1").arg(m_bookmarkData.at(i).bookmarkOffset, 16, 16, QChar('0'))));
		tblBookmark->setItem(i, 2, new QTableWidgetItem(m_bookmarkData.at(i).bookmarkComment));
	}

	tblBookmark->blockSignals(false);
}

void qtDLGBookmark::RemoveSelectedBookmark()
{
	if(tblBookmark->selectedItems().count() <= 0) return;

	BookmarkRemove(tblBookmark->item(tblBookmark->selectedItems()[0]->row(), 1)->text().toULongLong(0,16));
}

void qtDLGBookmark::UpdateBookmarks(QString fileName, int processID)
{
	for(int i = 0; i < m_bookmarkData.size(); i++)
	{
		if(fileName.contains(m_bookmarkData.at(i).bookmarkModule))
		{
			m_bookmarkData[i].bookmarkPID = processID;
			break;
		}
	}
}

void qtDLGBookmark::BookmarkClear()
{
	if(pThis == NULL) return;

	pThis->m_bookmarkData.clear();
	pThis->UpdateDisplay();
}

void qtDLGBookmark::SendToDisassembler(QTableWidgetItem *pItem)
{
	emit ShowInDisassembler(tblBookmark->item(pItem->row(), 1)->text().toULongLong(0, 16));
}

void qtDLGBookmark::CellDataChanged(int row, int column)
{
	if(column != 2)
	{
		UpdateDisplay();
		return;
	}

	QString changedModule = tblBookmark->item(row, 0)->text();
	quint64 changedOffset = tblBookmark->item(row, 1)->text().toULongLong(0, 16);

	for(int i = 0; i < m_bookmarkData.size(); i++)
	{
		if(m_bookmarkData.at(i).bookmarkOffset == changedOffset && m_bookmarkData.at(i).bookmarkModule.contains(changedModule))
		{
			m_bookmarkData[i].bookmarkComment = tblBookmark->item(row, 2)->text();

			CleanIfOffsetLoaded(changedOffset);
			
			break;
		}
	}
}

void qtDLGBookmark::CleanIfOffsetLoaded(quint64 bookmarkOffset)
{
	if(qtDLGNanomite::GetInstance()->coreDisAs->SectionDisAs.contains(bookmarkOffset))
	{
		qtDLGNanomite::GetInstance()->coreDisAs->SectionDisAs.clear();
		emit pThis->ShowInDisassembler(bookmarkOffset);
	}
}

bool qtDLGBookmark::BookmarkAdd(int processID, quint64 bookmarkOffset, QString bookmarkComment)
{
	if(pThis == NULL) return false;

	QList<BookmarkData> *pBookmarkList = &pThis->m_bookmarkData;

	for(int i = 0; i < pBookmarkList->size(); i++)
	{
		if(pBookmarkList->at(i).bookmarkOffset == bookmarkOffset && pBookmarkList->at(i).bookmarkPID == processID)
			return false;
	}

	TCHAR moduleName[MAX_PATH * sizeof(TCHAR)] = { 0 };
	BookmarkData newBookmark;
	newBookmark.bookmarkComment = bookmarkComment;
	newBookmark.bookmarkOffset = bookmarkOffset;
	newBookmark.bookmarkPID = processID;
	newBookmark.bookmarkProcessModule = qtDLGNanomite::GetInstance()->PEManager->getFilenameFromPID(processID);
	newBookmark.bookmarkBaseOffset = clsHelperClass::CalcOffsetForModule(moduleName, bookmarkOffset, processID);
	newBookmark.bookmarkModule = QString::fromWCharArray(moduleName); 

	pBookmarkList->append(newBookmark);
	pThis->UpdateDisplay();
	pThis->CleanIfOffsetLoaded(bookmarkOffset);

	return true;
}

bool qtDLGBookmark::BookmarkRemove(quint64 bookmarkOffset)
{
	if(pThis == NULL) return false;

	QList<BookmarkData> *pBookmarkList = &pThis->m_bookmarkData;

	for(int i = 0; i < pBookmarkList->size(); i++)
	{
		if(pBookmarkList->at(i).bookmarkOffset == bookmarkOffset)
		{
			pBookmarkList->removeAt(i);

			pThis->UpdateDisplay();
			pThis->CleanIfOffsetLoaded(bookmarkOffset);
			return true;
		}
	}

	return false;
}

QString qtDLGBookmark::BookmarkGetComment(int processID, quint64 bookmarkOffset)
{
	if(pThis == NULL) return false;

	QList<BookmarkData> *pBookmarkList = &pThis->m_bookmarkData;

	for(int i = 0; i < pBookmarkList->size(); i++)
	{
		if(pBookmarkList->at(i).bookmarkOffset == bookmarkOffset/* && pBookmarkList->at(i).bookmarkPID == processID*/)
		{
			return pBookmarkList->at(i).bookmarkComment;
		}
	}

	return QString("");
}

QList<BookmarkData> qtDLGBookmark::BookmarkGetList()
{
	return pThis->m_bookmarkData;
}

void qtDLGBookmark::BookmarkUpdateOffsets(HANDLE processHandle, int processID)
{
	QString processModule = qtDLGNanomite::GetInstance()->PEManager->getFilenameFromPID(processID);

	for(int i = 0; i < m_bookmarkData.size(); i++)
	{
		if(processModule.contains(m_bookmarkData.at(i).bookmarkProcessModule, Qt::CaseInsensitive))
		{
			DWORD64 newBaseOffset = clsHelperClass::CalcOffsetForModule((PTCHAR)m_bookmarkData.at(i).bookmarkModule.toStdWString().c_str(),
																		m_bookmarkData.at(i).bookmarkOffset,
																		processID);

			if(newBaseOffset != m_bookmarkData.at(i).bookmarkBaseOffset)
			{
				DWORD64 tempOffset = m_bookmarkData.at(i).bookmarkOffset - m_bookmarkData.at(i).bookmarkBaseOffset;

				m_bookmarkData[i].bookmarkBaseOffset = newBaseOffset;
				m_bookmarkData[i].bookmarkOffset = newBaseOffset + tempOffset;

				if(m_bookmarkData.at(i).bookmarkPID == NULL)
					m_bookmarkData[i].bookmarkPID = processID;
			}
		}
	}

	UpdateDisplay();
}

void qtDLGBookmark::BookmarkInsertFromProjectFile(BookmarkData newBookmark)
{
	m_bookmarkData.append(newBookmark);
}