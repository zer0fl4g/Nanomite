#ifndef QTDLGOPTION_H
#define QTDLGOPTION_H

#include "ui_qtDLGOption.h"

class qtDLGOption : public QDialog, public Ui_qtDLGOptionClass
{
	Q_OBJECT

public:
	qtDLGOption(QWidget *parent = 0, Qt::WFlags flags = 0);
	~qtDLGOption();

	private slots:
		void OnRightClickCustomException(const QPoint &);
		void OnClose();
		void OnReload();
		void OnSave();

private:
	void OnLoad();
};

#endif
