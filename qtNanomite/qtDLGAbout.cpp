#include "qtDLGAbout.h"

qtDLGAbout::qtDLGAbout(QWidget *parent, Qt::WFlags flags)
	: QDialog(parent, flags)
{
	ui.setupUi(this);
	this->setFixedSize(this->width(),this->height());
	this->setStyleSheet("background: rgb(230, 235, 230)");
}

qtDLGAbout::~qtDLGAbout()
{

}