#include "qtDLGSourceViewer.h"

#include <QFile>
#include <Windows.h>

qtDLGSourceViewer::qtDLGSourceViewer(QWidget *parent, Qt::WFlags flags)
	: QWidget(parent, flags)
{
	setupUi(this);
	this->setLayout(verticalLayout);
	IsSourceAvailable = false;
}

qtDLGSourceViewer::~qtDLGSourceViewer()
{

}

void qtDLGSourceViewer::OnDisplaySource(QString sourceFile,int LineNumber)
{
	int LineCounter = 1;
	QFile file(sourceFile);
	
	if(file.open(QIODevice::ReadOnly))
	{
		listSource->clear();
		while(file.bytesAvailable() > 0)
		{
			QListWidgetItem *pItem = new QListWidgetItem();
			pItem->setText(file.readLine().replace("\r","").replace("\n",""));
			if(LineCounter == LineNumber)
				pItem->setBackground(QBrush(QColor("Blue"),Qt::SolidPattern));
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