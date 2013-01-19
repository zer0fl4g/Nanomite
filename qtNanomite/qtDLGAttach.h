#ifndef QTDLGATTACH_H
#define QTDLGATTACH_H

#include "ui_qtDLGAttach.h"

class qtDLGAttach : public QDialog, public Ui_qtDLGAttachClass
{
	Q_OBJECT

public:
	qtDLGAttach(QWidget *parent = 0, Qt::WFlags flags = 0);
	~qtDLGAttach();

signals:
	void StartAttachedDebug(int iPID);

private:
	void FillProcessList();

private slots:
		void OnProcessDoubleClick(int iRow,int iColumn);

};

#endif
