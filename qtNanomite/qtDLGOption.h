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
#ifndef QTDLGOPTION_H
#define QTDLGOPTION_H

#include "ui_qtDLGOption.h"

#include "qtDLGNanomite.h"

class qtDLGOption : public QDialog, public Ui_qtDLGOptionClass
{
	Q_OBJECT

public:
	qtDLGOption(QWidget *parent = 0, Qt::WFlags flags = 0);
	~qtDLGOption();

	private slots:
		void OnRightClickCustomException(const QPoint &);
		void OnClose();
		void OnReload();
		void OnSave();

private:
	void OnLoad();

	int getIndex(QString itemColor);
};

#endif
