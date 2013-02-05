#include "qtDLGBreakPointManager.h"
#include "qtDLGNanomite.h"

#include <string>

using namespace std;

qtDLGBreakPointManager::qtDLGBreakPointManager(QWidget *parent, Qt::WFlags flags)
	: QWidget(parent, flags)
{
	setupUi(this);
	this->setFixedSize(this->width(),this->height());

	connect(pbClose,SIGNAL(clicked()),this,SLOT(OnClose()));
	connect(pbAddUpdate,SIGNAL(clicked()),this,SLOT(OnAddUpdate()));
	connect(tblBPs,SIGNAL(cellClicked(int,int)),this,SLOT(OnSelectedBPChanged(int,int)));

}

qtDLGBreakPointManager::~qtDLGBreakPointManager()
{

}

void qtDLGBreakPointManager::OnClose()
{
	close();
}

void qtDLGBreakPointManager::OnUpdate(BPStruct newBP,int iType)
{
	if(newBP.dwHandle != 0x2)
	{
		tblBPs->insertRow(tblBPs->rowCount());
		tblBPs->setItem(tblBPs->rowCount() - 1,0,new QTableWidgetItem(QString("%1").arg(newBP.dwPID,8,16,QChar('0'))));
		tblBPs->setItem(tblBPs->rowCount() - 1,1,new QTableWidgetItem(QString("%1").arg(newBP.dwOffset,16,16,QChar('0'))));

		QString TempString;
		switch(iType)
		{
		case 0:
			TempString = "Software BP - int3";
			break;
		case 1:
			TempString = "Memory BP - Page Guard";
			break;
		case 2:
			TempString = "Hardware BP - Dr[0-3]";
			break;
		}
		tblBPs->setItem(tblBPs->rowCount() - 1,2,new QTableWidgetItem(TempString));

		tblBPs->setItem(tblBPs->rowCount() - 1,3,new QTableWidgetItem(QString("%1").arg(newBP.dwSize,2,16,QChar('0'))));

		switch(newBP.dwTypeFlag)
		{
		case DR_EXECUTE:
			TempString = "Execute";
			break;
		case DR_READ:
			TempString = "Read";
			break;
		case DR_WRITE:
			TempString = "Write";
			break;
		}
		tblBPs->setItem(tblBPs->rowCount() - 1,4,new QTableWidgetItem(TempString));
	}
}

void qtDLGBreakPointManager::OnAddUpdate()
{
	int iUpdateLine = -1;
	qtDLGNanomite *myMainWindow = qtDLGNanomite::GetInstance();

	if(leOffset->text().contains("::"))
	{
		QStringList SplitAPIList = leOffset->text().split("::");

		if(SplitAPIList.count() >= 2)
		{
	
			quint64 dwOffset = (quint64)GetProcAddress(GetModuleHandleA(SplitAPIList[0].toUtf8().data()),
				SplitAPIList[1].toUtf8().data());

			if(dwOffset <= 0)
			{
				MessageBoxW(NULL,L"Please use a correct API Name!",L"Nanomite - Breakpoint Manager",MB_OK);
				return;
			}
			else
				leOffset->setText(QString("%1").arg(dwOffset,16,16,QChar('0')));
		}
	}

	for(int i = 0; i < tblBPs->rowCount(); i++)
		if(QString().compare(tblBPs->item(i,0)->text(),lePID->text()) == 0 &&
			QString().compare(tblBPs->item(i,1)->text(),leOffset->text()) == 0)
			iUpdateLine = i;		

	if(iUpdateLine != -1)
	{
		DWORD dwType = 0;
		if(QString().compare(tblBPs->item(iUpdateLine,2)->text(),"Software BP - int3") == 0)
			dwType = 0;
		else if(QString().compare(tblBPs->item(iUpdateLine,2)->text(),"Hardware BP - Dr[0-3]") == 0)
			dwType = 2;
		else if(QString().compare(tblBPs->item(iUpdateLine,2)->text(),"Memory BP - Page Guard") == 0)
			dwType = 1;

		myMainWindow->coreDebugger->RemoveBPFromList(tblBPs->item(iUpdateLine,1)->text().toULongLong(0,16),dwType);
		tblBPs->removeRow(iUpdateLine);
	}

	DWORD dwType = 0,
		dwBreakOn = 0;

	if(cbType->currentText().compare("Software BP - int3") == 0)
		dwType = 0;
	else if(cbType->currentText().compare("Hardware BP - Dr[0-3]") == 0)
		dwType = 2;
	else if(cbType->currentText().compare("Memory BP - Page Guard") == 0)
		dwType = 1;

	if(cbBreakOn->currentText().compare("Execute") == 0)
		dwBreakOn = DR_EXECUTE;
	else if(cbBreakOn->currentText().compare("Read") == 0)
		dwBreakOn = DR_READ;
	else if(cbBreakOn->currentText().compare("Write") == 0)
		dwBreakOn = DR_WRITE;

	myMainWindow->coreDebugger->AddBreakpointToList(dwType,dwBreakOn,lePID->text().toLong(0,16),
		leOffset->text().toULongLong(0,16),0,true);
}

void qtDLGBreakPointManager::OnSelectedBPChanged(int iRow,int iCol)
{
	lePID->setText(tblBPs->item(iRow,0)->text());
	leOffset->setText(tblBPs->item(iRow,1)->text());
	leSize->setText(tblBPs->item(iRow,3)->text());

	if(QString().compare(tblBPs->item(iRow,2)->text(),"Software BP - int3") == 0)
		cbType->setCurrentIndex(0);
	else if(QString().compare(tblBPs->item(iRow,2)->text(),"Hardware BP - Dr[0-3]") == 0)
		cbType->setCurrentIndex(1);
	else if(QString().compare(tblBPs->item(iRow,2)->text(),"Memory BP - Page Guard") == 0)
		cbType->setCurrentIndex(2);

	if(QString().compare(tblBPs->item(iRow,4)->text(),"Execute") == 0)
		cbBreakOn->setCurrentIndex(0);
	else if(QString().compare(tblBPs->item(iRow,4)->text(),"Read") == 0)
		cbBreakOn->setCurrentIndex(2);
	else if(QString().compare(tblBPs->item(iRow,4)->text(),"Write") == 0)
		cbBreakOn->setCurrentIndex(1);
}