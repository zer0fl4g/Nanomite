#ifndef QTDLGREGEDIT_H
#define QTDLGREGEDIT_H

#include "ui_qtDLGRegEdit86.h"
#include "ui_qtDLGRegEdit64.h"

#include <Windows.h>

class qtDLGRegEdit : public QDialog
{
	Q_OBJECT

public:
	qtDLGRegEdit(QWidget *parent = 0, Qt::WFlags flags = 0,LPVOID pProcessContext = NULL,bool bIs64 = false);
	~qtDLGRegEdit();

signals:
	void OnUpdateRegView();

private:
	bool _bIs64;
	LPVOID _pProcessContext;

	Ui::qtDLGRegEdit86Class ui86;
	Ui::qtDLGRegEdit64Class ui64;

	void FillGUI();

	private slots:
		void OnExit();
		void OnSaveAndExit();
};

#endif
