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
#ifndef QTDLGBPMANAGER_H
#define QTDLGBPMANAGER_H

#include "clsDebugger\clsDebugger.h"

#include "ui_qtDLGBreakPointManager.h"

class qtDLGBreakPointManager : public QWidget, public Ui_qtDLGBreakPointManagerClass
{
	Q_OBJECT

public:
	qtDLGBreakPointManager(QWidget *parent = 0, Qt::WFlags flags = 0);
	~qtDLGBreakPointManager();

	void DeleteCompleterContent();

	public slots:
		void OnUpdate(BPStruct newBP,int iType);
		void OnDelete(quint64 bpOffset);
		void OnBPRemove();
		void UpdateCompleter(std::wstring,int iPID);

private:
	QStringList completerList;

	private slots:
		void OnClose();
		void OnAddUpdate();
		void OnSelectedBPChanged(int iRow,int iCol);
};

#endif
