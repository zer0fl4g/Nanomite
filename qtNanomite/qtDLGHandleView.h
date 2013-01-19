#ifndef QTDLGHANDLEVIEW_H
#define QTDLGHANDLEVIEW_H

#include "ui_qtDLGHandleView.h"

#include <Windows.h>

class qtDLGHandleView : public QWidget, public Ui_qtDLGHandleViewClass
{
	Q_OBJECT

public:
	qtDLGHandleView(QWidget *parent = 0, Qt::WFlags flags = 0,qint32 iPID = 0);
	~qtDLGHandleView();

private:
	qint32 _iPID;

	void InsertDataIntoTable(DWORD dwPID,DWORD dwHandle,PTCHAR ptType,PTCHAR ptName);
};

#endif
