#ifndef QTDLGHEAPVIEW_H
#define QTDLGHEAPVIEW_H

#include "ui_qtDLGHeapView.h"

class qtDLGHeapView : public QWidget, public Ui_qtDLGHeapViewClass
{
	Q_OBJECT

public:
	qtDLGHeapView(QWidget *parent = 0, Qt::WFlags flags = 0,int iPID = 0);
	~qtDLGHeapView();

private:
	int _iPID;
};

#endif
