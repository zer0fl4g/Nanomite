#ifndef QTDLGABOUT_H
#define QTDLGABOUT_H

#include "ui_qtDLGAbout.h"

class qtDLGAbout : public QDialog
{
	Q_OBJECT

public:
	qtDLGAbout(QWidget *parent = 0, Qt::WFlags flags = 0);
	~qtDLGAbout();
	
private:
	Ui::qtDLGAboutUI ui;
};

#endif
