#include "qtDLGDetailInfo.h"

#include <QtCore>
#include <QMenu>

qtDLGDetailInfo::qtDLGDetailInfo(QWidget *parent, Qt::WFlags flags)
	: QWidget(parent, flags)
{
	setupUi(this);
	this->setLayout(verticalLayout);

	connect(tblTIDs,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnCustomTIDContextMenu(QPoint)));
	connect(tblPIDs,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnCustomPIDContextMenu(QPoint)));
	connect(tblExceptions,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnCustomExceptionContextMenu(QPoint)));
	connect(tblModules,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnCustomModuleContextMenu(QPoint)));
}

qtDLGDetailInfo::~qtDLGDetailInfo()
{

}

void qtDLGDetailInfo::OnCustomPIDContextMenu(QPoint qPoint)
{
	if(tblPIDs->rowCount() <= 0) return;

	QMenu menu;

	_iSelectedRow = tblPIDs->indexAt(qPoint).row();
	_SelectedOffset = tblPIDs->item(_iSelectedRow,1)->text().toULongLong(0,16);

	menu.addAction(new QAction("Show Offset in disassembler",this));
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));

	menu.exec(QCursor::pos());
}

void qtDLGDetailInfo::OnCustomTIDContextMenu(QPoint qPoint)
{
	if(tblTIDs->rowCount() <= 0) return;

	QMenu menu;

	_iSelectedRow = tblTIDs->indexAt(qPoint).row();
	_SelectedOffset = tblTIDs->item(_iSelectedRow,2)->text().toULongLong(0,16);

	menu.addAction(new QAction("Show Offset in disassembler",this));
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));

	menu.exec(QCursor::pos());
}

void qtDLGDetailInfo::OnCustomExceptionContextMenu(QPoint qPoint)
{
	if(tblExceptions->rowCount() <= 0) return;

	QMenu menu;

	_iSelectedRow = tblExceptions->indexAt(qPoint).row();
	_SelectedOffset = tblExceptions->item(_iSelectedRow,0)->text().toULongLong(0,16);

	menu.addAction(new QAction("Show Offset in disassembler",this));
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));

	menu.exec(QCursor::pos());
}

void qtDLGDetailInfo::OnCustomModuleContextMenu(QPoint qPoint)
{
	if(tblModules->rowCount() <= 0) return;

	QMenu menu;

	_iSelectedRow = tblModules->indexAt(qPoint).row();
	_SelectedOffset = tblModules->item(_iSelectedRow,1)->text().toULongLong(0,16);

	menu.addAction(new QAction("Show Offset in disassembler",this));
	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));

	menu.exec(QCursor::pos());
}

void qtDLGDetailInfo::MenuCallback(QAction* pAction)
{
	if(QString().compare(pAction->text(),"Show Offset in disassembler") == 0)
	{
		if(_SelectedOffset >= 0)
		{
			emit ShowInDisassembler(_SelectedOffset);
			_SelectedOffset = NULL;
		}
	}
}