/********************************************************************************
** Form generated from reading UI file 'qtDLGMemoryView.ui'
**
** Created: Sat 19. Jan 02:12:58 2013
**      by: Qt User Interface Compiler version 4.8.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QTDLGMEMORYVIEW_H
#define UI_QTDLGMEMORYVIEW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QTableWidget>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_qtDLGMemoryViewClass
{
public:
    QTableWidget *tblMemoryView;

    void setupUi(QWidget *qtDLGMemoryViewClass)
    {
        if (qtDLGMemoryViewClass->objectName().isEmpty())
            qtDLGMemoryViewClass->setObjectName(QString::fromUtf8("qtDLGMemoryViewClass"));
        qtDLGMemoryViewClass->resize(861, 541);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/qtDLGNanomite/Icons/IDI_MAIN.ico"), QSize(), QIcon::Normal, QIcon::Off);
        qtDLGMemoryViewClass->setWindowIcon(icon);
        tblMemoryView = new QTableWidget(qtDLGMemoryViewClass);
        if (tblMemoryView->columnCount() < 6)
            tblMemoryView->setColumnCount(6);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tblMemoryView->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tblMemoryView->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        tblMemoryView->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        tblMemoryView->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        tblMemoryView->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        tblMemoryView->setHorizontalHeaderItem(5, __qtablewidgetitem5);
        tblMemoryView->setObjectName(QString::fromUtf8("tblMemoryView"));
        tblMemoryView->setGeometry(QRect(0, 0, 861, 541));
        tblMemoryView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tblMemoryView->setSelectionBehavior(QAbstractItemView::SelectRows);
        tblMemoryView->horizontalHeader()->setStretchLastSection(true);
        tblMemoryView->verticalHeader()->setVisible(false);
        tblMemoryView->verticalHeader()->setDefaultSectionSize(15);
        tblMemoryView->verticalHeader()->setMinimumSectionSize(15);

        retranslateUi(qtDLGMemoryViewClass);

        QMetaObject::connectSlotsByName(qtDLGMemoryViewClass);
    } // setupUi

    void retranslateUi(QWidget *qtDLGMemoryViewClass)
    {
        qtDLGMemoryViewClass->setWindowTitle(QApplication::translate("qtDLGMemoryViewClass", "[ Nanomite ] - Memory View", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem = tblMemoryView->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("qtDLGMemoryViewClass", "PID", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem1 = tblMemoryView->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("qtDLGMemoryViewClass", "Base Address", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem2 = tblMemoryView->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QApplication::translate("qtDLGMemoryViewClass", "Size", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem3 = tblMemoryView->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QApplication::translate("qtDLGMemoryViewClass", "Module", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem4 = tblMemoryView->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QApplication::translate("qtDLGMemoryViewClass", "Type", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem5 = tblMemoryView->horizontalHeaderItem(5);
        ___qtablewidgetitem5->setText(QApplication::translate("qtDLGMemoryViewClass", "Access", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class qtDLGMemoryViewClass: public Ui_qtDLGMemoryViewClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QTDLGMEMORYVIEW_H
