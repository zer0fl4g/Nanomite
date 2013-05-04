#ifndef QTDLGSTACK_H
#define QTDLGSTACK_H

#include <QDockWidget>
#include "ui_qtDLGStack.h"

class qtDLGStack : public QDockWidget, public Ui_qtDLGStack
{
	Q_OBJECT

public:
	qtDLGStack(QWidget *parent = 0);
	~qtDLGStack();

private:
};

#endif // QTDLGSTACK_H
