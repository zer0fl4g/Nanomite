/********************************************************************************
** Form generated from reading UI file 'qtDLGHeapView.ui'
**
** Created: Sat 19. Jan 02:12:58 2013
**      by: Qt User Interface Compiler version 4.8.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QTDLGHEAPVIEW_H
#define UI_QTDLGHEAPVIEW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QTableWidget>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_qtDLGHeapViewClass
{
public:
    QTableWidget *tblHeapView;

    void setupUi(QWidget *qtDLGHeapViewClass)
    {
        if (qtDLGHeapViewClass->objectName().isEmpty())
            qtDLGHeapViewClass->setObjectName(QString::fromUtf8("qtDLGHeapViewClass"));
        qtDLGHeapViewClass->resize(770, 579);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/qtDLGNanomite/Icons/IDI_MAIN.ico"), QSize(), QIcon::Normal, QIcon::Off);
        qtDLGHeapViewClass->setWindowIcon(icon);
        tblHeapView = new QTableWidget(qtDLGHeapViewClass);
        if (tblHeapView->columnCount() < 6)
            tblHeapView->setColumnCount(6);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tblHeapView->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tblHeapView->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        tblHeapView->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        tblHeapView->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        tblHeapView->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        tblHeapView->setHorizontalHeaderItem(5, __qtablewidgetitem5);
        tblHeapView->setObjectName(QString::fromUtf8("tblHeapView"));
        tblHeapView->setGeometry(QRect(0, 0, 770, 580));
        tblHeapView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tblHeapView->setSelectionBehavior(QAbstractItemView::SelectRows);
        tblHeapView->horizontalHeader()->setStretchLastSection(true);
        tblHeapView->verticalHeader()->setVisible(false);
        tblHeapView->verticalHeader()->setDefaultSectionSize(15);
        tblHeapView->verticalHeader()->setMinimumSectionSize(15);

        retranslateUi(qtDLGHeapViewClass);

        QMetaObject::connectSlotsByName(qtDLGHeapViewClass);
    } // setupUi

    void retranslateUi(QWidget *qtDLGHeapViewClass)
    {
        qtDLGHeapViewClass->setWindowTitle(QApplication::translate("qtDLGHeapViewClass", "[ Nanomite ] - Heap View", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem = tblHeapView->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("qtDLGHeapViewClass", "PID", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem1 = tblHeapView->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("qtDLGHeapViewClass", "Heap ID", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem2 = tblHeapView->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QApplication::translate("qtDLGHeapViewClass", "Base Address", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem3 = tblHeapView->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QApplication::translate("qtDLGHeapViewClass", "Size", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem4 = tblHeapView->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QApplication::translate("qtDLGHeapViewClass", "Block Count", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem5 = tblHeapView->horizontalHeaderItem(5);
        ___qtablewidgetitem5->setText(QApplication::translate("qtDLGHeapViewClass", "Flags", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class qtDLGHeapViewClass: public Ui_qtDLGHeapViewClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QTDLGHEAPVIEW_H
