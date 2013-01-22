#ifndef QTDLGHEXVIEW_H
#define QTDLGHEXVIEW_H

#include "ui_qtDLGHexView.h"

class qtDLGHexView : public QWidget, public Ui_qtDLGHexViewClass
{
	Q_OBJECT

public:
	qtDLGHexView(QWidget *parent, Qt::WFlags flags,unsigned long dwPID, unsigned long long StartOffset,unsigned long long Size);
	~qtDLGHexView();
};

#endif
