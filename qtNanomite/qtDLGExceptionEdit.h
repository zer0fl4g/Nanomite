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
#ifndef QTDLGEXCEPTIONEDIT_H
#define QTDLGEXCEPTIONEDIT_H

#include "ui_qtDLGExceptionEdit.h"

#include <Windows.h>

class qtDLGExceptionEdit : public QDialog, public Ui_qtDLGExceptionEditClass
{
	Q_OBJECT

public:
	qtDLGExceptionEdit(QWidget *parent = 0, Qt::WFlags flags = 0, DWORD exceptionCode = 0, int handleException = -1);
	~qtDLGExceptionEdit();

signals:
	void OnInsertNewException(DWORD exceptionCode, int handleCode);

private:
	QStringList LoadExceptionList();

private slots:
	void OnSave();
	void OnCancel();
};

#endif
