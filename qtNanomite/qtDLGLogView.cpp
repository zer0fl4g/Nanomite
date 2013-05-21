/*
 * 	This file is part of Nanomite.
 *
 *    Nanomite is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    Nanomite is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with Nanomite.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "qtDLGLogView.h"

#include "clsMemManager.h"

#include <Windows.h>
#include <time.h>

#include <QClipboard>
#include <QTextStream>
#include <QFileDialog>
#include <QMenu>

using namespace std;

qtDLGLogView::qtDLGLogView(QWidget *parent)
	: QDockWidget(parent)
{
	setupUi(this);
	
	// List LogBox
	tblLogBox->horizontalHeader()->resizeSection(0,85);
	tblLogBox->horizontalHeader()->resizeSection(1,300);

	connect(tblLogBox,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(OnContextMenu(QPoint)));
}

qtDLGLogView::~qtDLGLogView()
{

}

void qtDLGLogView::OnContextMenu(QPoint qPoint)
{
	QMenu menu;

	_iSelectedRow = tblLogBox->indexAt(qPoint).row();
	if(_iSelectedRow < 0) return;


	menu.addAction(new QAction("Clear Log",this));
	menu.addAction(new QAction("Write Log to File",this));
	QMenu *submenu = menu.addMenu("Copy to Clipboard");
	submenu->addAction(new QAction("Line",this));
	submenu->addAction(new QAction("Time",this));
	submenu->addAction(new QAction("Text",this));

	connect(submenu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));

	connect(&menu,SIGNAL(triggered(QAction*)),this,SLOT(MenuCallback(QAction*)));

	menu.exec(QCursor::pos());
}

void qtDLGLogView::MenuCallback(QAction* pAction)
{
	if(QString().compare(pAction->text(),"Clear Log") == 0)
		tblLogBox->setRowCount(0);	
	else if(QString().compare(pAction->text(),"Write Log to File") == 0)
	{
		QString fileName = QFileDialog::getSaveFileName(NULL,"Please select a place to save the Logfile",QDir::currentPath(),"Log Files (*.log)");
		QFile tempOutput(fileName);
		tempOutput.open(QIODevice::WriteOnly | QIODevice::Text);
		QTextStream out(&tempOutput);

		for(int i = 0; i < tblLogBox->rowCount(); i++)
			out << tblLogBox->item(i,0)->text() << "\t" << tblLogBox->item(i,1)->text() << "\n";

		tempOutput.close();
		MessageBoxW(NULL,L"The log file has been written to disk!",L"Nanomite",MB_OK);
	}
	else if(QString().compare(pAction->text(),"Line") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(QString("%1:%2").arg(tblLogBox->item(_iSelectedRow,0)->text()).arg(tblLogBox->item(_iSelectedRow,1)->text()));
	}
	else if(QString().compare(pAction->text(),"Time") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblLogBox->item(_iSelectedRow,0)->text());
	}
	else if(QString().compare(pAction->text(),"Text") == 0)
	{
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(tblLogBox->item(_iSelectedRow,1)->text());
	}
}

int qtDLGLogView::OnLog(wstring sLog)
{
	time_t tTime;
	tm timeInfo;
	time(&tTime);
	localtime_s(&timeInfo,&tTime);

	tblLogBox->insertRow(tblLogBox->rowCount());
	
	tblLogBox->setItem(tblLogBox->rowCount() - 1,0,
		new QTableWidgetItem(QString().sprintf("[%i:%i:%i]",timeInfo.tm_hour,timeInfo.tm_min,timeInfo.tm_sec)));
	
	tblLogBox->setItem(tblLogBox->rowCount() - 1,1,
		new QTableWidgetItem(QString::fromStdWString(sLog)));
	
	tblLogBox->scrollToBottom();
	return 0;
}