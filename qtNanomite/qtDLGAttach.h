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
#ifndef QTDLGATTACH_H
#define QTDLGATTACH_H

#include "ui_qtDLGAttach.h"

class qtDLGAttach : public QDialog, public Ui_qtDLGAttachClass
{
	Q_OBJECT

public:
	qtDLGAttach(QWidget *parent = 0, Qt::WFlags flags = 0);
	~qtDLGAttach();

signals:
	void StartAttachedDebug(int iPID,QString sFile);

private:
	void FillProcessList();

private slots:
		void OnProcessDoubleClick(int iRow,int iColumn);

};

#endif
