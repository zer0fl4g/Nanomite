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
#ifndef QTDLGPROCESSPRIVILEGE_H
#define QTDLGPROCESSPRIVILEGE_H

#include "ui_qtDLGProcessPrivilege.h"

#include "qtDLGNanomite.h"

class qtDLGProcessPrivilege : public QWidget, public Ui_qtDLGProcessPrivilegeClass
{
	Q_OBJECT

public:
	qtDLGProcessPrivilege(QWidget *parent, Qt::WFlags flags,qint32 iPID);
	~qtDLGProcessPrivilege();

private:
	int _PID,
		_SelectedRow;
	size_t	_ForEntry,
			_ForEnd;

	qtDLGNanomite *myMainWindow;

private slots:
	void DisplayPrivileges();
	void MenuCallback(QAction*);
	void OnCustomContextMenuRequested(QPoint qPoint);
};

#endif // QTDLGREGISTERS_H
