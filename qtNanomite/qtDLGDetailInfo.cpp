#include "qtDLGDetailInfo.h"

qtDLGDetailInfo::qtDLGDetailInfo(QWidget *parent, Qt::WFlags flags)
	: QWidget(parent, flags)
{
	setupUi(this);
	this->setFixedSize(this->width(),this->height());
}

qtDLGDetailInfo::~qtDLGDetailInfo()
{

}