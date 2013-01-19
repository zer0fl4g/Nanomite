/********************************************************************************
** Form generated from reading UI file 'qtDLGOption.ui'
**
** Created: Sat 19. Jan 02:12:58 2013
**      by: Qt User Interface Compiler version 4.8.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QTDLGOPTION_H
#define UI_QTDLGOPTION_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDialog>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QTableWidget>

QT_BEGIN_NAMESPACE

class Ui_qtDLGOptionClass
{
public:
    QGroupBox *groupBox;
    QRadioButton *rbSystemEP;
    QRadioButton *rbModuleEP;
    QRadioButton *rbTlsCallback;
    QRadioButton *rbDirect;
    QPushButton *btnSave;
    QPushButton *btnClose;
    QPushButton *btnReload;
    QGroupBox *groupBox_2;
    QTableWidget *tblCustomExceptions;
    QCheckBox *cbInvPriv;
    QCheckBox *cbDivZero;
    QCheckBox *cbIG_AVIOL;
    QCheckBox *cbIgEx;
    QGroupBox *groupBox_3;
    QCheckBox *cbLoadSym;
    QCheckBox *cbDebugChild;
    QCheckBox *cbSuspendThread;

    void setupUi(QDialog *qtDLGOptionClass)
    {
        if (qtDLGOptionClass->objectName().isEmpty())
            qtDLGOptionClass->setObjectName(QString::fromUtf8("qtDLGOptionClass"));
        qtDLGOptionClass->setWindowModality(Qt::WindowModal);
        qtDLGOptionClass->resize(341, 411);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/qtDLGNanomite/IDI_MAIN.ico"), QSize(), QIcon::Normal, QIcon::Off);
        qtDLGOptionClass->setWindowIcon(icon);
        groupBox = new QGroupBox(qtDLGOptionClass);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(10, 10, 161, 111));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
        groupBox->setSizePolicy(sizePolicy);
        rbSystemEP = new QRadioButton(groupBox);
        rbSystemEP->setObjectName(QString::fromUtf8("rbSystemEP"));
        rbSystemEP->setGeometry(QRect(10, 20, 141, 17));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(rbSystemEP->sizePolicy().hasHeightForWidth());
        rbSystemEP->setSizePolicy(sizePolicy1);
        rbModuleEP = new QRadioButton(groupBox);
        rbModuleEP->setObjectName(QString::fromUtf8("rbModuleEP"));
        rbModuleEP->setGeometry(QRect(10, 40, 140, 17));
        sizePolicy1.setHeightForWidth(rbModuleEP->sizePolicy().hasHeightForWidth());
        rbModuleEP->setSizePolicy(sizePolicy1);
        rbTlsCallback = new QRadioButton(groupBox);
        rbTlsCallback->setObjectName(QString::fromUtf8("rbTlsCallback"));
        rbTlsCallback->setGeometry(QRect(10, 60, 140, 17));
        sizePolicy1.setHeightForWidth(rbTlsCallback->sizePolicy().hasHeightForWidth());
        rbTlsCallback->setSizePolicy(sizePolicy1);
        rbDirect = new QRadioButton(groupBox);
        rbDirect->setObjectName(QString::fromUtf8("rbDirect"));
        rbDirect->setGeometry(QRect(10, 80, 131, 17));
        sizePolicy1.setHeightForWidth(rbDirect->sizePolicy().hasHeightForWidth());
        rbDirect->setSizePolicy(sizePolicy1);
        btnSave = new QPushButton(qtDLGOptionClass);
        btnSave->setObjectName(QString::fromUtf8("btnSave"));
        btnSave->setGeometry(QRect(70, 380, 80, 23));
        sizePolicy1.setHeightForWidth(btnSave->sizePolicy().hasHeightForWidth());
        btnSave->setSizePolicy(sizePolicy1);
        btnClose = new QPushButton(qtDLGOptionClass);
        btnClose->setObjectName(QString::fromUtf8("btnClose"));
        btnClose->setGeometry(QRect(250, 380, 80, 23));
        sizePolicy1.setHeightForWidth(btnClose->sizePolicy().hasHeightForWidth());
        btnClose->setSizePolicy(sizePolicy1);
        btnReload = new QPushButton(qtDLGOptionClass);
        btnReload->setObjectName(QString::fromUtf8("btnReload"));
        btnReload->setGeometry(QRect(160, 380, 80, 23));
        sizePolicy1.setHeightForWidth(btnReload->sizePolicy().hasHeightForWidth());
        btnReload->setSizePolicy(sizePolicy1);
        groupBox_2 = new QGroupBox(qtDLGOptionClass);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        groupBox_2->setGeometry(QRect(10, 130, 321, 241));
        sizePolicy.setHeightForWidth(groupBox_2->sizePolicy().hasHeightForWidth());
        groupBox_2->setSizePolicy(sizePolicy);
        tblCustomExceptions = new QTableWidget(groupBox_2);
        if (tblCustomExceptions->columnCount() < 2)
            tblCustomExceptions->setColumnCount(2);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tblCustomExceptions->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tblCustomExceptions->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        tblCustomExceptions->setObjectName(QString::fromUtf8("tblCustomExceptions"));
        tblCustomExceptions->setGeometry(QRect(10, 100, 301, 131));
        QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Expanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(tblCustomExceptions->sizePolicy().hasHeightForWidth());
        tblCustomExceptions->setSizePolicy(sizePolicy2);
        tblCustomExceptions->setContextMenuPolicy(Qt::CustomContextMenu);
        tblCustomExceptions->horizontalHeader()->setVisible(true);
        tblCustomExceptions->horizontalHeader()->setDefaultSectionSize(100);
        tblCustomExceptions->horizontalHeader()->setStretchLastSection(true);
        tblCustomExceptions->verticalHeader()->setVisible(false);
        tblCustomExceptions->verticalHeader()->setMinimumSectionSize(15);
        cbInvPriv = new QCheckBox(groupBox_2);
        cbInvPriv->setObjectName(QString::fromUtf8("cbInvPriv"));
        cbInvPriv->setGeometry(QRect(10, 60, 171, 17));
        sizePolicy1.setHeightForWidth(cbInvPriv->sizePolicy().hasHeightForWidth());
        cbInvPriv->setSizePolicy(sizePolicy1);
        cbDivZero = new QCheckBox(groupBox_2);
        cbDivZero->setObjectName(QString::fromUtf8("cbDivZero"));
        cbDivZero->setGeometry(QRect(10, 40, 101, 17));
        sizePolicy1.setHeightForWidth(cbDivZero->sizePolicy().hasHeightForWidth());
        cbDivZero->setSizePolicy(sizePolicy1);
        cbIG_AVIOL = new QCheckBox(groupBox_2);
        cbIG_AVIOL->setObjectName(QString::fromUtf8("cbIG_AVIOL"));
        cbIG_AVIOL->setGeometry(QRect(10, 20, 110, 17));
        sizePolicy1.setHeightForWidth(cbIG_AVIOL->sizePolicy().hasHeightForWidth());
        cbIG_AVIOL->setSizePolicy(sizePolicy1);
        cbIgEx = new QCheckBox(groupBox_2);
        cbIgEx->setObjectName(QString::fromUtf8("cbIgEx"));
        cbIgEx->setGeometry(QRect(10, 80, 131, 17));
        sizePolicy1.setHeightForWidth(cbIgEx->sizePolicy().hasHeightForWidth());
        cbIgEx->setSizePolicy(sizePolicy1);
        groupBox_3 = new QGroupBox(qtDLGOptionClass);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        groupBox_3->setGeometry(QRect(180, 10, 151, 91));
        sizePolicy.setHeightForWidth(groupBox_3->sizePolicy().hasHeightForWidth());
        groupBox_3->setSizePolicy(sizePolicy);
        cbLoadSym = new QCheckBox(groupBox_3);
        cbLoadSym->setObjectName(QString::fromUtf8("cbLoadSym"));
        cbLoadSym->setGeometry(QRect(20, 20, 91, 17));
        sizePolicy1.setHeightForWidth(cbLoadSym->sizePolicy().hasHeightForWidth());
        cbLoadSym->setSizePolicy(sizePolicy1);
        cbDebugChild = new QCheckBox(groupBox_3);
        cbDebugChild->setObjectName(QString::fromUtf8("cbDebugChild"));
        cbDebugChild->setGeometry(QRect(20, 40, 131, 17));
        sizePolicy1.setHeightForWidth(cbDebugChild->sizePolicy().hasHeightForWidth());
        cbDebugChild->setSizePolicy(sizePolicy1);
        cbSuspendThread = new QCheckBox(groupBox_3);
        cbSuspendThread->setObjectName(QString::fromUtf8("cbSuspendThread"));
        cbSuspendThread->setGeometry(QRect(20, 60, 121, 17));
        sizePolicy1.setHeightForWidth(cbSuspendThread->sizePolicy().hasHeightForWidth());
        cbSuspendThread->setSizePolicy(sizePolicy1);

        retranslateUi(qtDLGOptionClass);

        QMetaObject::connectSlotsByName(qtDLGOptionClass);
    } // setupUi

    void retranslateUi(QDialog *qtDLGOptionClass)
    {
        qtDLGOptionClass->setWindowTitle(QApplication::translate("qtDLGOptionClass", "[ Nanomite ] - Option", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("qtDLGOptionClass", "Entry - BreakOn", 0, QApplication::UnicodeUTF8));
        rbSystemEP->setText(QApplication::translate("qtDLGOptionClass", "System EP", 0, QApplication::UnicodeUTF8));
        rbModuleEP->setText(QApplication::translate("qtDLGOptionClass", "Module EP", 0, QApplication::UnicodeUTF8));
        rbTlsCallback->setText(QApplication::translate("qtDLGOptionClass", "TLS - Callback", 0, QApplication::UnicodeUTF8));
        rbDirect->setText(QApplication::translate("qtDLGOptionClass", "Direct Run", 0, QApplication::UnicodeUTF8));
        btnSave->setText(QApplication::translate("qtDLGOptionClass", "Save", 0, QApplication::UnicodeUTF8));
        btnClose->setText(QApplication::translate("qtDLGOptionClass", "Close", 0, QApplication::UnicodeUTF8));
        btnReload->setText(QApplication::translate("qtDLGOptionClass", "Reload Default", 0, QApplication::UnicodeUTF8));
        groupBox_2->setTitle(QApplication::translate("qtDLGOptionClass", "Exception Handling", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem = tblCustomExceptions->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("qtDLGOptionClass", "Custom Exception", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem1 = tblCustomExceptions->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("qtDLGOptionClass", "Handle", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        tblCustomExceptions->setToolTip(QString());
#endif // QT_NO_TOOLTIP
        cbInvPriv->setText(QApplication::translate("qtDLGOptionClass", "Invalid or Privileged Instruction", 0, QApplication::UnicodeUTF8));
        cbDivZero->setText(QApplication::translate("qtDLGOptionClass", "Division by zero", 0, QApplication::UnicodeUTF8));
        cbIG_AVIOL->setText(QApplication::translate("qtDLGOptionClass", "Access Violation", 0, QApplication::UnicodeUTF8));
        cbIgEx->setText(QApplication::translate("qtDLGOptionClass", "Ignore all Exceptions", 0, QApplication::UnicodeUTF8));
        groupBox_3->setTitle(QApplication::translate("qtDLGOptionClass", "GroupBox", 0, QApplication::UnicodeUTF8));
        cbLoadSym->setText(QApplication::translate("qtDLGOptionClass", "Load Symbols", 0, QApplication::UnicodeUTF8));
        cbDebugChild->setText(QApplication::translate("qtDLGOptionClass", "Debug Child Processes", 0, QApplication::UnicodeUTF8));
        cbSuspendThread->setText(QApplication::translate("qtDLGOptionClass", "Use SuspendThread", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class qtDLGOptionClass: public Ui_qtDLGOptionClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QTDLGOPTION_H
