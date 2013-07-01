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

#include "qtDLGExceptionEdit.h"

#include <QStringList>

#include "clsMemManager.h"
#include "clsHelperClass.h"

qtDLGExceptionEdit::qtDLGExceptionEdit(QWidget *parent, Qt::WFlags flags, DWORD exceptionCode, int handleException)
	: QDialog(parent, flags)
{
	setupUi(this);
	this->setFixedSize(this->width(),this->height());
	this->setStyleSheet(clsHelperClass::LoadStyleSheet());

	connect(pbSave,SIGNAL(clicked()),this,SLOT(OnSave()));
	connect(pbCancel,SIGNAL(clicked()),this,SLOT(OnCancel()));

	if(exceptionCode != NULL && handleException != -1)
	{
		leExceptionCode->setText(QString("%1").arg(exceptionCode,8,16,QChar('0')));
		leExceptionCode->setEnabled(false);

		switch(handleException)
		{
		case 0:
			cbHandleException->setCurrentIndex(0);
			break;
		case 1:
			cbHandleException->setCurrentIndex(1);
			break;
		case 2:
			cbHandleException->setCurrentIndex(2);
			break;
		default:
			cbHandleException->setCurrentIndex(0);
			break;
		}		
	}
	else
	{
		QStringList exceptionList = LoadExceptionList();
		leExceptionCode->setCompleter(new QCompleter(exceptionList,this));
	}
}

qtDLGExceptionEdit::~qtDLGExceptionEdit()
{

}

void qtDLGExceptionEdit::OnSave()
{
	if(leExceptionCode->text().contains("::"))
		emit OnInsertNewException(leExceptionCode->text().split("::")[0].toULong(0,16),cbHandleException->currentIndex());
	else
		emit OnInsertNewException(leExceptionCode->text().toULong(0,16),cbHandleException->currentIndex());
	close();
}

void qtDLGExceptionEdit::OnCancel()
{
	close();
}

QStringList qtDLGExceptionEdit::LoadExceptionList()
{
	QStringList newExceptionList;
	newExceptionList.append("00000000::STATUS_WAIT_0");
	newExceptionList.append("00000080::STATUS_ABANDONED_WAIT_0");
	newExceptionList.append("000000C0::STATUS_USER_APC");
	newExceptionList.append("00000102::STATUS_TIMEOUT");
	newExceptionList.append("00000103::STATUS_PENDING");
	newExceptionList.append("40000005::STATUS_SEGMENT_NOTIFICATION");
	newExceptionList.append("40010005::DBG_CONTROL_C");
	newExceptionList.append("40010006::DBG_PRINTEXCEPTION_C");
	newExceptionList.append("40010007::DBG_RIPEXCEPTION");
	newExceptionList.append("40010008::DBG_CONTROL_BREAK");
	newExceptionList.append("40010009::DBG_COMMAND_EXCEPTION");
	newExceptionList.append("80000001::STATUS_GUARD_PAGE_VIOLATION");
	newExceptionList.append("80000002::STATUS_DATATYPE_MISALIGNMENT");
	newExceptionList.append("80000003::STATUS_BREAKPOINT");
	newExceptionList.append("80000004::STATUS_SINGLE_STEP");
	newExceptionList.append("80000026::STATUS_LONGJUMP");
	newExceptionList.append("80000029::STATUS_UNWIND_CONSOLIDATE");
	newExceptionList.append("C0000005::STATUS_ACCESS_VIOLATION");
	newExceptionList.append("C0000006::STATUS_IN_PAGE_ERROR");
	newExceptionList.append("C0000008::STATUS_INVALID_HANDLE");
	newExceptionList.append("C000000D::STATUS_INVALID_PARAMETER");
	newExceptionList.append("C0000017::STATUS_NO_MEMORY");
	newExceptionList.append("C000001D::STATUS_ILLEGAL_INSTRUCTION");
	newExceptionList.append("C0000025::STATUS_NONCONTINUABLE_EXCEPTION");
	newExceptionList.append("C0000026::STATUS_INVALID_DISPOSITION");
	newExceptionList.append("C000008C::STATUS_ARRAY_BOUNDS_EXCEEDED");
	newExceptionList.append("C000008D::STATUS_FLOAT_DENORMAL_OPERAND");
	newExceptionList.append("C000008E::STATUS_FLOAT_DIVIDE_BY_ZERO");
	newExceptionList.append("C000008F::STATUS_FLOAT_INEXACT_RESULT");
	newExceptionList.append("C0000090::STATUS_FLOAT_INVALID_OPERATION");
	newExceptionList.append("C0000091::STATUS_FLOAT_OVERFLOW");
	newExceptionList.append("C0000092::STATUS_FLOAT_STACK_CHECK");
	newExceptionList.append("C0000093::STATUS_FLOAT_UNDERFLOW");
	newExceptionList.append("C0000094::STATUS_INTEGER_DIVIDE_BY_ZERO");
	newExceptionList.append("C0000095::STATUS_INTEGER_OVERFLOW");
	newExceptionList.append("C0000096::STATUS_PRIVILEGED_INSTRUCTION");
	newExceptionList.append("C00000FD::STATUS_STACK_OVERFLOW");
	newExceptionList.append("C0000135::STATUS_DLL_NOT_FOUND");
	newExceptionList.append("C0000138::STATUS_ORDINAL_NOT_FOUND");
	newExceptionList.append("C0000139::STATUS_ENTRYPOINT_NOT_FOUND");
	newExceptionList.append("C000013A::STATUS_CONTROL_C_EXIT");
	newExceptionList.append("C0000142::STATUS_DLL_INIT_FAILED");
	newExceptionList.append("C00002B4::STATUS_FLOAT_MULTIPLE_FAULTS");
	newExceptionList.append("C00002B5::STATUS_FLOAT_MULTIPLE_TRAPS");
	newExceptionList.append("C00002C9::STATUS_REG_NAT_CONSUMPTION");
	newExceptionList.append("C0000409::STATUS_STACK_BUFFER_OVERRUN");
	newExceptionList.append("C000041::STATUS_INVALID_CRUNTIME_PARAMETER");
	newExceptionList.append("C0000420::STATUS_ASSERTION_FAILURE");
           
	return newExceptionList;
}