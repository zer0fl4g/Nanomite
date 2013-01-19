#include "qtDLGWindowView.h"
#include "qtDLGNanomite.h"

#include "clsCallbacks.h"

qtDLGWindowView* qtDLGWindowView::pThis = NULL;

qtDLGWindowView::qtDLGWindowView(QWidget *parent, Qt::WFlags flags,qint32 iPID)
	: QWidget(parent, flags)
{
	setupUi(this);
	pThis = this;
	this->setAttribute(Qt::WA_DeleteOnClose,true);
	_iPID = iPID;
	this->setFixedSize(this->width(),this->height());

	// Init List
	tblWindowView->horizontalHeader()->resizeSection(0,75);
	tblWindowView->horizontalHeader()->resizeSection(1,300);
	tblWindowView->horizontalHeader()->resizeSection(2,75);
	tblWindowView->horizontalHeader()->resizeSection(3,135);

	// Display
	qtDLGNanomite *myMainWindow = qtDLGNanomite::GetInstance();

	int iForEntry = 0;
	int iForEnd = myMainWindow->coreDebugger->PIDs.size();

	for(int i = 0; i < myMainWindow->coreDebugger->PIDs.size(); i++)
	{
		if(myMainWindow->coreDebugger->PIDs[i].dwPID == _iPID)
			iForEntry = i; iForEnd = i +1;
	}

	for(int i = iForEntry; i < iForEnd;i++)
	{
		EnumWindows((WNDENUMPROC)clsCallbacks::EnumWindowCallBack,(LPARAM)myMainWindow->coreDebugger->PIDs[i].dwPID);
	}
}

qtDLGWindowView::~qtDLGWindowView()
{

}