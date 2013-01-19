#ifndef QTDLGMEMORY_H
#define QTDLGMEMORY_H

#include "ui_qtDLGMemoryView.h"

class qtDLGMemoryView : public QWidget, public Ui_qtDLGMemoryViewClass
{
	Q_OBJECT

public:
	qtDLGMemoryView(QWidget *parent = 0, Qt::WFlags flags = 0,qint32 iPID = 0);
	~qtDLGMemoryView();

private:
	qint32 _iPID;
};

#endif
