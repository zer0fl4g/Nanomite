/********************************************************************************
** Form generated from reading UI file 'qtDLGHandleView.ui'
**
** Created: Sat 19. Jan 02:12:58 2013
**      by: Qt User Interface Compiler version 4.8.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QTDLGHANDLEVIEW_H
#define UI_QTDLGHANDLEVIEW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QTableWidget>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_qtDLGHandleViewClass
{
public:
    QTableWidget *tblHandleView;

    void setupUi(QWidget *qtDLGHandleViewClass)
    {
        if (qtDLGHandleViewClass->objectName().isEmpty())
            qtDLGHandleViewClass->setObjectName(QString::fromUtf8("qtDLGHandleViewClass"));
        qtDLGHandleViewClass->resize(880, 310);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/qtDLGNanomite/Icons/IDI_MAIN.ico"), QSize(), QIcon::Normal, QIcon::Off);
        qtDLGHandleViewClass->setWindowIcon(icon);
        tblHandleView = new QTableWidget(qtDLGHandleViewClass);
        if (tblHandleView->columnCount() < 4)
            tblHandleView->setColumnCount(4);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tblHandleView->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tblHandleView->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        tblHandleView->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        tblHandleView->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        tblHandleView->setObjectName(QString::fromUtf8("tblHandleView"));
        tblHandleView->setGeometry(QRect(0, 0, 880, 310));
        tblHandleView->horizontalHeader()->setStretchLastSection(true);
        tblHandleView->verticalHeader()->setVisible(false);
        tblHandleView->verticalHeader()->setDefaultSectionSize(15);
        tblHandleView->verticalHeader()->setMinimumSectionSize(15);

        retranslateUi(qtDLGHandleViewClass);

        QMetaObject::connectSlotsByName(qtDLGHandleViewClass);
    } // setupUi

    void retranslateUi(QWidget *qtDLGHandleViewClass)
    {
        qtDLGHandleViewClass->setWindowTitle(QApplication::translate("qtDLGHandleViewClass", "[ Nanomite ] - Handle View", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem = tblHandleView->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("qtDLGHandleViewClass", "PID", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem1 = tblHandleView->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("qtDLGHandleViewClass", "Handle", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem2 = tblHandleView->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QApplication::translate("qtDLGHandleViewClass", "Type", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem3 = tblHandleView->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QApplication::translate("qtDLGHandleViewClass", "Name", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class qtDLGHandleViewClass: public Ui_qtDLGHandleViewClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QTDLGHANDLEVIEW_H
