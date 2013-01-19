#ifndef QTDLGSTRINGVIEW_H
#define QTDLGSTRINGVIEW_H

#include "ui_qtDLGStringView.h"

class qtDLGStringView : public QWidget, public Ui_qtDLGStringViewClass
{
	Q_OBJECT

public:
	qtDLGStringView(QWidget *parent = 0, Qt::WFlags flags = 0,qint32 iPID = 0);
	~qtDLGStringView();

private:
	qint32 _iPID;
};

#endif
