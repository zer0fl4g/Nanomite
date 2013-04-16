#ifndef QTDLGTRACE_H
#define QTDLGTRACE_H

#include "ui_qtDLGTrace.h"

#include <Windows.h>
#include <string>
#include <QMap>

struct TraceInfoRow
{
	DWORD64 dwOffset;
	DWORD	PID;
	DWORD	TID;
	QString asmInstruction;
	QString cpuReg;
};

class qtDLGTrace : public QWidget, public Ui_qtDLGTraceClass
{
	Q_OBJECT

public:
	qtDLGTrace(QWidget *parent = 0, Qt::WFlags flags = 0);
	~qtDLGTrace();
	
	static void addTraceData(DWORD64 dwOffset,DWORD PID,DWORD TID);
	static void clearTraceData();

private:
	static qtDLGTrace *pThis;
	QMap<DWORD64,TraceInfoRow> traceData;

private slots:
	void OnShow(int Offset);

protected:
	void showEvent(QShowEvent * event);
	void wheelEvent(QWheelEvent * event);
	void resizeEvent(QResizeEvent *event);
};

#endif