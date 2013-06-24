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
	int retVal = 2;

	if(cbAlways->isChecked())
		emit ContinueException(retVal + 10);
	else
		emit ContinueException(retVal);

	close();
}

void qtDLGExceptionAsk::ExceptionSendToApp()
{
	int retVal = 1;

	if(cbAlways->isChecked())
		emit ContinueException(retVal + 10);
	else
		emit ContinueException(retVal);

	close();
}

void qtDLGExceptionAsk::ExceptionBreak()
{
	int retVal = 0;

	if(cbAlways->isChecked())
		emit ContinueException(retVal + 10);
	else
		emit ContinueException(retVal);

	close();
}

void qtDLGExceptionAsk::closeEvent(QCloseEvent *event)
{
	//emit ContinueException(0);
	//close();
}