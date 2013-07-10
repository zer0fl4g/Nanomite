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
#include "qtDLGSourceViewer.h"

#include <QFile>
#include <Windows.h>

qtDLGSourceViewer::qtDLGSourceViewer(QWidget *parent, Qt::WFlags flags)
	: QWidget(parent, flags),
	IsSourceAvailable(false)
{
	setupUi(this);
	this->setLayout(verticalLayout);
}

qtDLGSourceViewer::~qtDLGSourceViewer()
{

}

void qtDLGSourceViewer::OnDisplaySource(QString sourceFile,int LineNumber)
{
	QFile file(sourceFile);
	
	if(file.open(QIODevice::ReadOnly))
	{
		int LineCounter = 1;
		listSource->clear();
		while(file.bytesAvailable() > 0)
		{
			QListWidgetItem *pItem = new QListWidgetItem();
			pItem->setText(file.readLine().replace("\r","").replace("\n",""));
			if(LineCounter == LineNumber)
			{
				pItem->setBackground(QBrush(QColor("Blue"),Qt::SolidPattern));
				pItem->setForeground(QBrush(QColor("White"),Qt::SolidPattern));
			}

			listSource->addItem(pItem);
			LineCounter++;
		}
		IsSourceAvailable = true;
		file.close();
	}
	else
	{
		IsSourceAvailable = false;
		//MessageBoxW(NULL,L"Sorry could not open this file!",L"Nanomite",MB_OK);
		//close();
	}
	return;
}