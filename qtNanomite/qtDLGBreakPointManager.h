#ifndef QTDLGBPMANAGER_H
#define QTDLGBPMANAGER_H

#include "clsDebugger\clsDebugger.h"

#include "ui_qtDLGBreakPointManager.h"

class qtDLGBreakPointManager : public QWidget, public Ui_qtDLGBreakPointManagerClass
{
	Q_OBJECT

public:
	qtDLGBreakPointManager(QWidget *parent = 0, Qt::WFlags flags = 0);
	~qtDLGBreakPointManager();

	void DeleteCompleterContent();

	public slots:
		void OnUpdate(BPStruct newBP,int iType);
		void OnBPRemove();
		void UpdateCompleter(std::wstring,int iPID);

private:
	QStringList completerList;

	private slots:
		void OnClose();
		void OnAddUpdate();
		void OnSelectedBPChanged(int iRow,int iCol);
};

#endif
