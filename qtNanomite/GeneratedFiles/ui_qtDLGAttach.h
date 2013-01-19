/********************************************************************************
** Form generated from reading UI file 'qtDLGAttach.ui'
**
** Created: Sat 19. Jan 02:12:58 2013
**      by: Qt User Interface Compiler version 4.8.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QTDLGATTACH_H
#define UI_QTDLGATTACH_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QTableWidget>

QT_BEGIN_NAMESPACE

class Ui_qtDLGAttachClass
{
public:
    QTableWidget *tblProcList;

    void setupUi(QDialog *qtDLGAttachClass)
    {
        if (qtDLGAttachClass->objectName().isEmpty())
            qtDLGAttachClass->setObjectName(QString::fromUtf8("qtDLGAttachClass"));
        qtDLGAttachClass->resize(471, 468);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/qtDLGNanomite/IDI_MAIN.ico"), QSize(), QIcon::Normal, QIcon::Off);
        qtDLGAttachClass->setWindowIcon(icon);
        tblProcList = new QTableWidget(qtDLGAttachClass);
        if (tblProcList->columnCount() < 3)
            tblProcList->setColumnCount(3);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tblProcList->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tblProcList->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        tblProcList->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        tblProcList->setObjectName(QString::fromUtf8("tblProcList"));
        tblProcList->setGeometry(QRect(10, 10, 450, 450));
        tblProcList->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tblProcList->setSelectionMode(QAbstractItemView::ExtendedSelection);
        tblProcList->setSelectionBehavior(QAbstractItemView::SelectRows);
        tblProcList->setSortingEnabled(true);
        tblProcList->horizontalHeader()->setStretchLastSection(true);
        tblProcList->verticalHeader()->setVisible(false);
        tblProcList->verticalHeader()->setDefaultSectionSize(15);
        tblProcList->verticalHeader()->setMinimumSectionSize(15);

        retranslateUi(qtDLGAttachClass);

        QMetaObject::connectSlotsByName(qtDLGAttachClass);
    } // setupUi

    void retranslateUi(QDialog *qtDLGAttachClass)
    {
        qtDLGAttachClass->setWindowTitle(QApplication::translate("qtDLGAttachClass", "[ Nanomite ] - AttachTo", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem = tblProcList->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("qtDLGAttachClass", "Process Name", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem1 = tblProcList->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("qtDLGAttachClass", "PID", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem2 = tblProcList->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QApplication::translate("qtDLGAttachClass", "Path", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class qtDLGAttachClass: public Ui_qtDLGAttachClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QTDLGATTACH_H
