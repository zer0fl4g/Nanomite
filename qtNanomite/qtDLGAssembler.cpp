#include "qtDLGAssembler.h"

#include "clsMemManager.h"

qtDLGAssembler::qtDLGAssembler(QWidget *parent, Qt::WFlags flags,HANDLE hProc)
	: QWidget(parent, flags)
{
	setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose,true);
	this->setLayout(verticalLayout);

	_hProc = hProc;
}

qtDLGAssembler::~qtDLGAssembler()
{

}