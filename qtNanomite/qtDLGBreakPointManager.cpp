#include "qtDLGBreakPointManager.h"
#include "qtDLGNanomite.h"

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

void qtDLGBreakPointManager::OnAddUpdate()
{
	int iUpdateLine = -1;
	qtDLGNanomite *myMainWindow = qtDLGNanomite::GetInstance();

	if(leOffset->text().contains("::"))
	{
		// Resolve API
		DWORD64 dwOffset = (DWORD64)GetProcAddress(GetModuleHandle(leOffset->text().split("::")[0].toStdWString().c_str()),
			leOffset->text().split("::")[1].toStdString().c_str());

		if(dwOffset <= 0)
		{
			MessageBoxW(NULL,L"Please use a correct API Name!",L"Nanomite - Breakpoint Manager",MB_OK);
			return;
		}
		else
			leOffset->setText(QString().sprintf("%016X",dwOffset));
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

		myMainWindow->coreDebugger->RemoveBPFromList(tblBPs->item(iUpdateLine,0)->text().toULongLong(0,16),dwType,
			tblBPs->item(iUpdateLine,0)->text().toLong());

		tblBPs->item(iUpdateLine,0)->setText(lePID->text());
		tblBPs->item(iUpdateLine,1)->setText(leOffset->text());
		tblBPs->item(iUpdateLine,2)->setText(cbType->currentText());
		tblBPs->item(iUpdateLine,3)->setText(leSize->text());
		tblBPs->item(iUpdateLine,4)->setText(cbBreakOn->currentText());

		tblBPs->removeRow(iUpdateLine);
	}

	tblBPs->insertRow(tblBPs->rowCount());
	tblBPs->setItem(tblBPs->rowCount() - 1,0,new QTableWidgetItem(lePID->text()));
	tblBPs->setItem(tblBPs->rowCount() - 1,1,new QTableWidgetItem(leOffset->text()));
	tblBPs->setItem(tblBPs->rowCount() - 1,2,new QTableWidgetItem(cbType->currentText()));
	tblBPs->setItem(tblBPs->rowCount() - 1,3,new QTableWidgetItem(leSize->text()));
	tblBPs->setItem(tblBPs->rowCount() - 1,4,new QTableWidgetItem(cbBreakOn->currentText()));

	for(int i = 0; i < tblBPs->rowCount(); i++)
	{
		DWORD dwType = 0,
			dwBreakOn = 0;

		if(QString().compare(tblBPs->item(i,2)->text(),"Software BP - int3") == 0)
			dwType = 0;
		else if(QString().compare(tblBPs->item(i,2)->text(),"Hardware BP - Dr[0-3]") == 0)
			dwType = 2;
		else if(QString().compare(tblBPs->item(i,2)->text(),"Memory BP - Page Guard") == 0)
			dwType = 1;

		if(QString().compare(tblBPs->item(i,4)->text(),"Execute") == 0)
			dwBreakOn = DR_EXECUTE;
		else if(QString().compare(tblBPs->item(i,4)->text(),"Read") == 0)
			dwBreakOn = DR_READ;
		else if(QString().compare(tblBPs->item(i,4)->text(),"Write") == 0)
			dwBreakOn = DR_WRITE;

		myMainWindow->coreDebugger->AddBreakpointToList(dwType,dwBreakOn,tblBPs->item(i,0)->text().toLong(),
			tblBPs->item(i,1)->text().toULongLong(0,16),0,true);
	}
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
		cbBreakOn->setCurrentIndex(1);
	else if(QString().compare(tblBPs->item(iRow,4)->text(),"Write") == 0)
		cbBreakOn->setCurrentIndex(2);
}