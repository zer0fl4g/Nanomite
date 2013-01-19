#ifndef QTDLGDETINFO_H
#define QTDLGDETINFO_H

#include "ui_qtDLGDetailInfo.h"

class qtDLGDetailInfo : public QWidget, public Ui_qtDLGDetailInfoClass
{
	Q_OBJECT

public:
	qtDLGDetailInfo(QWidget *parent = 0, Qt::WFlags flags = 0);
	~qtDLGDetailInfo();
};

#endif
