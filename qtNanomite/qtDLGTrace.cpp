#include "qtDLGTrace.h"

#include "clsHelperClass.h"
#include "clsMemManager.h"

#include <QWheelEvent>

using namespace std;

qtDLGTrace *qtDLGTrace::pThis = NULL;

qtDLGTrace::qtDLGTrace(QWidget *parent, Qt::WFlags flags)
	: QWidget(parent, flags)
{
	setupUi(this);
	this->setLayout(horizontalLayout);
	this->setStyleSheet(clsHelperClass::LoadStyleSheet());
	pThis = this;

	tblTraceLog->horizontalHeader()->resizeSection(0,80); //PID
	tblTraceLog->horizontalHeader()->resizeSection(1,80); //TID
	tblTraceLog->horizontalHeader()->resizeSection(2,135); //OFFSET
	tblTraceLog->horizontalHeader()->resizeSection(3,300); //INST.
	//tblTraceLog->horizontalHeader()->resizeSection(4,300); //REG

	connect(tblTraceLog,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnCustomContextMenu(QPoint)));
	//connect(scrollTrace,SIGNAL(valueChanged(int)),this,SLOT(OnShow(int)));
}

qtDLGTrace::~qtDLGTrace()
{
	pThis = NULL;
}

void qtDLGTrace::addTraceData(DWORD64 dwOffset,DWORD PID,DWORD TID)
{
	if(pThis == NULL) return;

	TraceInfoRow newTraceRow;
	newTraceRow.PID = PID;
	newTraceRow.TID = TID;
	newTraceRow.asmInstruction = QString("");
	newTraceRow.cpuReg = QString("");
	newTraceRow.dwOffset = dwOffset;

	pThis->traceData.insert(dwOffset,newTraceRow);
}

void qtDLGTrace::clearTraceData()
{
	if(pThis == NULL) return;

	pThis->traceData.clear();
	pThis->scrollTrace->setValue(0);
	pThis->scrollTrace->setMaximum(0);
}

void qtDLGTrace::showEvent(QShowEvent * event)
{
	OnShow(0);
}

void qtDLGTrace::OnShow(int delta)
{
	int iLines = NULL,
		iPossibleRowCount = ((tblTraceLog->verticalHeader()->height() + 4) / 12) - 1;
	QMap<DWORD64,TraceInfoRow>::const_iterator i;

	scrollTrace->setMaximum(traceData.count());

	if(delta < 0 && tblTraceLog->rowCount() > 0)
	{
		i = traceData.constFind(tblTraceLog->item(0,2)->text().toULongLong(0,16));
		--i;
		scrollTrace->setValue(scrollTrace->value() - 1);
	}
	else if(delta > 0 && tblTraceLog->rowCount() > 1)
	{
		i = traceData.constFind(tblTraceLog->item(1,2)->text().toULongLong(0,16));
		scrollTrace->setValue(scrollTrace->value() + 1);
	}
	else if(delta == 0)
	{
		i = traceData.constBegin();
		scrollTrace->setValue(0);
	}
	else
		return;

	if((QMapData::Node *)i == (QMapData::Node *)traceData.constEnd())
		return;

	tblTraceLog->setRowCount(0);
	while(iLines <= iPossibleRowCount)
	{
		if((QMapData::Node *)i == (QMapData::Node *)traceData.constEnd())
			return;

		tblTraceLog->insertRow(tblTraceLog->rowCount());
		
		tblTraceLog->setItem(tblTraceLog->rowCount() - 1,0,
			new QTableWidgetItem(QString("%1").arg(i.value().PID,8,16,QChar('0'))));
		
		tblTraceLog->setItem(tblTraceLog->rowCount() - 1,1,
			new QTableWidgetItem(QString("%1").arg(i.value().TID,8,16,QChar('0'))));
	
		tblTraceLog->setItem(tblTraceLog->rowCount() - 1,2,
			new QTableWidgetItem(QString("%1").arg(i.value().dwOffset,16,16,QChar('0'))));

		tblTraceLog->setItem(tblTraceLog->rowCount() - 1,3,
			new QTableWidgetItem(i.value().asmInstruction));

		tblTraceLog->setItem(tblTraceLog->rowCount() - 1,4,
			new QTableWidgetItem(i.value().cpuReg));

		++i;iLines++;
	}
}

void qtDLGTrace::wheelEvent(QWheelEvent *event)
{
	QWheelEvent *pWheel = (QWheelEvent*)event;
		
	OnShow(pWheel->delta() * -1);
}

void qtDLGTrace::resizeEvent(QResizeEvent *event)
{
	OnShow(0);
}

void qtDLGTrace::OnCustomContextMenu(QPoint qPoint)
{
	QMenu menu;

	_iSelectedRow = tblTraceLog->indexAt(qPoint).row();

	menu.addAction(new QAction("Send to Disassembler",this));
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));

	menu.exec(QCursor::pos());
}

void qtDLGTrace::MenuCallback(QAction* pAction)
{
	if(QString().compare(pAction->text(),"Send to Disassembler") == 0)
		emit OnDisplayDisassembly(tblTraceLog->item(_iSelectedRow,2)->text().toULongLong(0,16));
}