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

	for(int i = 0; i < myMainWindow->coreDebugger->PIDs.size(); i++)
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

	for(int i = _iForEntry; i < _iForEnd;i++)
	{
		EnumWindows((WNDENUMPROC)clsCallbacks::EnumWindowCallBack,(LPARAM)myMainWindow->coreDebugger->PIDs[i].dwPID);
	}
}