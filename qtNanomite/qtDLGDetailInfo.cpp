#include "qtDLGDetailInfo.h"

qtDLGDetailInfo::qtDLGDetailInfo(QWidget *parent, Qt::WFlags flags)
	: QWidget(parent, flags)
{
	setupUi(this);
	this->setLayout(verticalLayout);
}

qtDLGDetailInfo::~qtDLGDetailInfo()
{

}