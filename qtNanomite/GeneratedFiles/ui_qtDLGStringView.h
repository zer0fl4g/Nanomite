/********************************************************************************
** Form generated from reading UI file 'qtDLGStringView.ui'
**
** Created: Sat 19. Jan 02:12:58 2013
**      by: Qt User Interface Compiler version 4.8.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QTDLGSTRINGVIEW_H
#define UI_QTDLGSTRINGVIEW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QTableWidget>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_qtDLGStringViewClass
{
public:
    QTableWidget *tblStringView;

    void setupUi(QWidget *qtDLGStringViewClass)
    {
        if (qtDLGStringViewClass->objectName().isEmpty())
            qtDLGStringViewClass->setObjectName(QString::fromUtf8("qtDLGStringViewClass"));
        qtDLGStringViewClass->resize(569, 659);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/qtDLGNanomite/Icons/IDI_MAIN.ico"), QSize(), QIcon::Normal, QIcon::Off);
        qtDLGStringViewClass->setWindowIcon(icon);
        tblStringView = new QTableWidget(qtDLGStringViewClass);
        if (tblStringView->columnCount() < 3)
            tblStringView->setColumnCount(3);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tblStringView->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tblStringView->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        tblStringView->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        tblStringView->setObjectName(QString::fromUtf8("tblStringView"));
        tblStringView->setGeometry(QRect(0, 0, 571, 661));
        tblStringView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tblStringView->setSelectionBehavior(QAbstractItemView::SelectRows);
        tblStringView->horizontalHeader()->setStretchLastSection(true);
        tblStringView->verticalHeader()->setVisible(false);
        tblStringView->verticalHeader()->setDefaultSectionSize(15);
        tblStringView->verticalHeader()->setMinimumSectionSize(15);

        retranslateUi(qtDLGStringViewClass);

        QMetaObject::connectSlotsByName(qtDLGStringViewClass);
    } // setupUi

    void retranslateUi(QWidget *qtDLGStringViewClass)
    {
        qtDLGStringViewClass->setWindowTitle(QApplication::translate("qtDLGStringViewClass", "[ Nanomite ] String View", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem = tblStringView->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("qtDLGStringViewClass", "PID", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem1 = tblStringView->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("qtDLGStringViewClass", "Offset", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem2 = tblStringView->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QApplication::translate("qtDLGStringViewClass", "String", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class qtDLGStringViewClass: public Ui_qtDLGStringViewClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QTDLGSTRINGVIEW_H
