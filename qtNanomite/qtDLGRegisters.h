#ifndef QTDLGREGISTERS_H
#define QTDLGREGISTERS_H

#include <QDockWidget>
#include "ui_qtDLGRegisters.h"


class qtDLGRegisters : public QDockWidget, public Ui_qtDLGRegisters
{
	Q_OBJECT

public:
	qtDLGRegisters(QWidget *parent = 0);
	~qtDLGRegisters();

private:
};

#endif // QTDLGREGISTERS_H
