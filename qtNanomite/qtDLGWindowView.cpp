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
#include "qtDLGWindowView.h"

#include "clsCallbacks.h"

qtDLGWindowView* qtDLGWindowView::pThis = NULL;

qtDLGWindowView::qtDLGWindowView(QWidget *parent, Qt::WFlags flags,qint32 iPID)
	: QWidget(parent, flags)
{
	setupUi(this);
	pThis = this;
	this->setAttribute(Qt::WA_DeleteOnClose,true);
	this->setLayout(verticalLayout);

	_iPID = iPID;

	// Init List
	tblWindowView->horizontalHeader()->resizeSection(0,75);
	tblWindowView->horizontalHeader()->resizeSection(1,300);
	tblWindowView->horizontalHeader()->resizeSection(2,75);
	tblWindowView->horizontalHeader()->resizeSection(3,135);

	// Display
	myMainWindow = qtDLGNanomite::GetInstance();

	_iForEntry = 0;
	_iForEnd = myMainWindow->coreDebugger->PIDs.size();

	for(size_t i = 0; i < myMainWindow->coreDebugger->PIDs.size(); i++)
	{
		if(myMainWindow->coreDebugger->PIDs[i].dwPID == _iPID)
			_iForEntry = i; _iForEnd = i +1;
	}

	connect(new QShortcut(QKeySequence("F5"),this),SIGNAL(activated()),this,SLOT(EnumWindow()));

	EnumWindow();
}

qtDLGWindowView::~qtDLGWindowView()
{

}

void qtDLGWindowView::EnumWindow()
{
	tblWindowView->setRowCount(0);

	for(size_t i = _iForEntry; i < _iForEnd;i++)
	{
		EnumWindows((WNDENUMPROC)clsCallbacks::EnumWindowCallBack,(LPARAM)myMainWindow->coreDebugger->PIDs[i].dwPID);
	}
}