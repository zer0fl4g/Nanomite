#include "qtDLGStringView.h"
#include "qtDLGNanomite.h"

#include <fstream>
#include <sstream>

using namespace std;

qtDLGStringView::qtDLGStringView(QWidget *parent, Qt::WFlags flags,qint32 iPID)
	: QWidget(parent, flags)
{
	setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose,true);
	_iPID = iPID;
	this->setFixedSize(this->width(),this->height());

	// Init List
	tblStringView->horizontalHeader()->resizeSection(0,75);
	tblStringView->horizontalHeader()->resizeSection(1,135);

	// Display
	qtDLGNanomite *myMainWindow = qtDLGNanomite::GetInstance();

	int iForEntry = 0;
	int iForEnd = myMainWindow->coreDebugger->PIDs.size();

	for(int i = 0; i < myMainWindow->coreDebugger->PIDs.size(); i++)
	{
		if(myMainWindow->coreDebugger->PIDs[i].dwPID == _iPID)
			iForEntry = i; iForEnd = i +1;
	}

	PTCHAR sTemp = (PTCHAR)malloc(255 * sizeof(WCHAR));
	for(int i = iForEntry; i < iForEnd;i++)
	{
		bool bNotEndOfFile = true;
		wifstream inputFile;
				
		inputFile.open(myMainWindow->coreDebugger->PIDs[i].sFileName,ifstream::binary);

		if(!inputFile.is_open())
		{
			MessageBox(NULL,myMainWindow->coreDebugger->PIDs[i].sFileName,L"Error opening File!",MB_OKCANCEL);
			bNotEndOfFile = false;
			close();
		}

		while(bNotEndOfFile && inputFile.good())
		{
			wstringstream sTempString;
			TCHAR sT;
			inputFile.get(sT);

			while(inputFile.good())
			{
				if(((int)sT >= 0x41 && (int)sT <= 0x5a)		||
					((int)sT >= 0x61 && (int)sT <= 0x7a)	||
					((int)sT >= 0x30 && (int)sT <= 0x39)	|| 
					((int)sT == 0x20))
					sTempString << sT;
				else
					break;
				inputFile.get(sT);
			}

			if(sTempString.str().length() > 3)
			{
				tblStringView->insertRow(tblStringView->rowCount());
						
				// PID
				tblStringView->setItem(tblStringView->rowCount() - 1,0,
					new QTableWidgetItem(QString().sprintf("%08X",myMainWindow->coreDebugger->PIDs[i].dwPID)));

				// Offset
				tblStringView->setItem(tblStringView->rowCount() - 1,1,
					new QTableWidgetItem(QString().sprintf("%08X",inputFile.tellg())));

				// String
				tblStringView->setItem(tblStringView->rowCount() - 1,2,
					new QTableWidgetItem(QString::fromStdWString(sTempString.str())));
			}
		}
	}
	free(sTemp);
}

qtDLGStringView::~qtDLGStringView()
{

}