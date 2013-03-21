#include "qtDLGAbout.h"
#include "clsHelperClass.h"

qtDLGAbout::qtDLGAbout(QWidget *parent, Qt::WFlags flags)
	: QDialog(parent, flags)
{
	ui.setupUi(this);
	this->setFixedSize(this->width(),this->height());
	this->setStyleSheet(clsHelperClass::LoadStyleSheet());
}

qtDLGAbout::~qtDLGAbout()
{

}