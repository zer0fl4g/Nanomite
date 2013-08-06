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
#include "qtDLGGoToDialog.h"
#include "qtDLGBreakPointManager.h"

#include "clsMemManager.h"

#include <QCompleter>
#include <QShortcut>

qtDLGGoToDialog::qtDLGGoToDialog(QString *pData, QStringList completerList, QWidget *parent, Qt::WFlags flags)
	: QDialog(parent, flags),
	m_pDataString(pData)
{
	setupUi(this);
	this->setFixedSize(this->width(),this->height());

	completerList.append(qtDLGBreakPointManager::ReturnCompleterList());
	m_pLineCompleter = new QCompleter(completerList);
	m_pLineCompleter->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
	lineEdit->setCompleter(m_pLineCompleter);

	connect(new QShortcut(QKeySequence(Qt::Key_Escape),this),SIGNAL(activated()),this,SLOT(close()));
	connect(lineEdit,SIGNAL(returnPressed()),this,SLOT(OpenNewData()));
}

qtDLGGoToDialog::~qtDLGGoToDialog()
{
	delete m_pLineCompleter;
}

void qtDLGGoToDialog::OpenNewData()
{
	if(lineEdit->text().length() >= 0)
	{
		*m_pDataString = lineEdit->text();
		close();
	}
}