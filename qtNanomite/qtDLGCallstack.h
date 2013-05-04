#ifndef QTDLGCALLSTACK_H
#define QTDLGCALLSTACK_H

#include <QDockWidget>
#include "ui_qtDLGCallstack.h"

class qtDLGCallstack : public QDockWidget, public Ui_qtDLGCallstack
{
	Q_OBJECT

public:
	qtDLGCallstack(QWidget *parent = 0);
	~qtDLGCallstack();

private:
};

#endif // QTDLGCALLSTACK_H
