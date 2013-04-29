#include "qtDLGStringView.h"
#include "clsHelperClass.h"
#include "clsMemManager.h"

#include <fstream>
#include <sstream>

using namespace std;

qtDLGStringView::qtDLGStringView(QWidget *parent, Qt::WFlags flags,qint32 iPID)
	: QWidget(parent, flags)
{
	setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose,true);
	this->setLayout(verticalLayout);
	
	_iPID = iPID;

	// Init List
	tblStringView->horizontalHeader()->resizeSection(0,75);
	tblStringView->horizontalHeader()->resizeSection(1,135);

	// Display
	myMainWindow = qtDLGNanomite::GetInstance();

	_iForEntry = 0;
	_iForEnd = myMainWindow->coreDebugger->PIDs.size();

	for(int i = 0; i < myMainWindow->coreDebugger->PIDs.size(); i++)
	{
		if(myMainWindow->coreDebugger->PIDs[i].dwPID == _iPID)
			_iForEntry = i; _iForEnd = i +1;
	}
	connect(new QShortcut(QKeySequence("F5"),this),SIGNAL(activated()),this,SLOT(DisplayStrings()));

	DisplayStrings();
}

qtDLGStringView::~qtDLGStringView()
{

}

void qtDLGStringView::DisplayStrings()
{
	tblStringView->setRowCount(0);

	PTCHAR sTemp = (PTCHAR)clsMemManager::CAlloc(MAX_PATH * sizeof(WCHAR));
	quint64 StartOffset = NULL,EndOffset = NULL;

	for(int i = _iForEntry; i < _iForEnd;i++)
	{
		//clsHelperClass::getStartAndEndOffsetOfPID(myMainWindow->coreDebugger->PIDs[i].dwPID,StartOffset,EndOffset);
		//if(StartOffset == 0 || EndOffset == 0)
		//	return;
		//
		//quint64 dwSize = EndOffset - StartOffset;
		//DWORD	dwOldProtection = 0,
		//		dwNewProtection = PAGE_EXECUTE_READWRITE;
		//LPVOID pBuffer = malloc(dwSize);

		//if(VirtualProtectEx(myMainWindow->coreDebugger->PIDs[i].hProc,(LPVOID)StartOffset,dwSize,dwNewProtection,&dwOldProtection) &&
		//	ReadProcessMemory(myMainWindow->coreDebugger->PIDs[i].hProc,(LPVOID)StartOffset,pBuffer,dwSize,NULL))
		//{	
		//	quint64 currentOffset = StartOffset,a = NULL;
		//	QString strTemp;
		//	
		//	while(currentOffset >= StartOffset && currentOffset <= EndOffset)
		//	{
		//		char sT = (char)((PCHAR)pBuffer)[a];
		//		while(true)
		//		{
		//			if(((int)sT >= 0x41 && (int)sT <= 0x5a)		||
		//				((int)sT >= 0x61 && (int)sT <= 0x7a)	||
		//				((int)sT >= 0x30 && (int)sT <= 0x39)	|| 
		//				((int)sT == 0x20))
		//			{
		//				strTemp.append((char*)&sT);
		//				a++;sT = (char)((PCHAR)pBuffer)[a];
		//			}
		//			else
		//			{
		//				a++;sT = (char)((PCHAR)pBuffer)[a];
		//				break;
		//			}
		//		}

		//		if(strTemp.length() > 3)
		//		{
		//			tblStringView->insertRow(tblStringView->rowCount());
		//					
		//			// PID
		//			tblStringView->setItem(tblStringView->rowCount() - 1,0,
		//				new QTableWidgetItem(QString().sprintf("%08X",myMainWindow->coreDebugger->PIDs[i].dwPID)));

		//			// Offset
		//			tblStringView->setItem(tblStringView->rowCount() - 1,1,
		//				new QTableWidgetItem(QString("%1").arg(currentOffset,16,16,QChar('0'))));

		//			// String
		//			tblStringView->setItem(tblStringView->rowCount() - 1,2,
		//				new QTableWidgetItem(strTemp));
		//			strTemp.clear();
		//		}
		//		currentOffset += a;
		//	}
		//}
		//else
		//{
		//	free(pBuffer);
		//	MessageBox(NULL,L"Access Denied! Can´t read this buffer :(",L"Nanomite",MB_OK);
		//	return;
		//}

		//bool bProtect = VirtualProtectEx(myMainWindow->coreDebugger->PIDs[i].hProc,(LPVOID)StartOffset,dwSize,dwOldProtection,NULL);
		//free(pBuffer);

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
				//tblStringView->setItem(tblStringView->rowCount() - 1,1,
				//	new QTableWidgetItem(QString().sprintf("%08X",inputFile.tellg())));

				// String
				tblStringView->setItem(tblStringView->rowCount() - 1,2,
					new QTableWidgetItem(QString::fromStdWString(sTempString.str())));
			}
		}
	}
	clsMemManager::CFree(sTemp);
}