#ifndef QTDLGDETINFO_H
#define QTDLGDETINFO_H

#include "ui_qtDLGDetailInfo.h"

class qtDLGDetailInfo : public QWidget, public Ui_qtDLGDetailInfoClass
{
	Q_OBJECT

public:
	qtDLGDetailInfo(QWidget *parent = 0, Qt::WFlags flags = 0);
	~qtDLGDetailInfo();

signals:
	void ShowInDisassembler(quint64 Offset);

private:
	int _iSelectedRow;
	quint64 _SelectedOffset;

private slots:
	void MenuCallback(QAction* pAction);
	void OnCustomTIDContextMenu(QPoint qPoint);
	void OnCustomPIDContextMenu(QPoint qPoint);
	void OnCustomExceptionContextMenu(QPoint qPoint);
	void OnCustomModuleContextMenu(QPoint qPoint);

};

#endif
