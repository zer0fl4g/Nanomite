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

#include "qtDLGExceptionAsk.h"

#include "clsHelperClass.h"
#include "clsMemManager.h"

qtDLGExceptionAsk::qtDLGExceptionAsk(DWORD exceptionCode, QWidget *parent, Qt::WFlags flags)
	: QDialog(parent, flags)
{
	this->setupUi(this);
	this->setFixedSize(this->width(),this->height());
	this->setStyleSheet(clsHelperClass::LoadStyleSheet());
	this->setAttribute(Qt::WA_DeleteOnClose,true);

	m_retValue = 0;

	lableState->setText(QString("ExceptionCode: %1").arg(exceptionCode,8,16,QChar('0')));

	connect(pbIgnore,SIGNAL(clicked()),this,SLOT(ExceptionIgnore()));
	connect(pbApp,SIGNAL(clicked()),this,SLOT(ExceptionSendToApp()));
	connect(pbBreak,SIGNAL(clicked()),this,SLOT(ExceptionBreak()));
}

qtDLGExceptionAsk::~qtDLGExceptionAsk()
{

}

void qtDLGExceptionAsk::ExceptionIgnore()
{
	if(cbAlways->isChecked())
		m_retValue = 12;
	else
		m_retValue = 2;

	close();
}

void qtDLGExceptionAsk::ExceptionSendToApp()
{
	if(cbAlways->isChecked())
		m_retValue = 11;
	else
		m_retValue = 1;

	close();
}

void qtDLGExceptionAsk::ExceptionBreak()
{
	if(cbAlways->isChecked())
		m_retValue = 10;
	else
		m_retValue = 0;

	close();
}

void qtDLGExceptionAsk::closeEvent(QCloseEvent *event)
{
	emit ContinueException(m_retValue);
}