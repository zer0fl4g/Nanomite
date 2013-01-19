#ifndef QTDLGDBGSTR_H
#define QTDLGDBGSTR_H

#include "Ui_qtDLGDebugStrings.h"

class qtDLGDebugStrings : public QWidget, public Ui_qtDLGDebugStringsClass
{
	Q_OBJECT

public:
	qtDLGDebugStrings(QWidget *parent = 0, Qt::WFlags flags = 0);
	~qtDLGDebugStrings();
};

#endif
