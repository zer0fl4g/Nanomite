#ifndef QTDLGSOURCEVIEWER_H
#define QTDLGSOURCEVIEWER_H

#include "ui_qtDLGSourceViewer.h"

class qtDLGSourceViewer : public QWidget, public Ui_qtDLGSourceViewerClass
{
	Q_OBJECT

public:
	bool IsSourceAvailable;

	qtDLGSourceViewer(QWidget *parent = 0, Qt::WFlags flags = 0);
	~qtDLGSourceViewer();

public slots:
	void OnDisplaySource(QString,int);
};

#endif
