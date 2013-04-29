#ifndef QTDLGWINDOWVIEW_H
#define QTDLGWINDOWVIEW_H

#include "ui_qtDLGWindowView.h"

#include "qtDLGNanomite.h"

class qtDLGWindowView : public QWidget, public Ui_qtDLGWindowViewClass
{
	Q_OBJECT

public:
	static qtDLGWindowView *pThis;
	
	qtDLGWindowView(QWidget *parent = 0, Qt::WFlags flags = 0,qint32 iPID = 0);
	~qtDLGWindowView();

private:
	qint32 _iPID;
	int _iForEntry;
	int _iForEnd;

	qtDLGNanomite *myMainWindow;

	private slots:
		void EnumWindow();
};
#endif