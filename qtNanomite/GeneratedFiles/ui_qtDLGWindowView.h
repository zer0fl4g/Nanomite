/********************************************************************************
** Form generated from reading UI file 'qtDLGWindowView.ui'
**
** Created: Sat 19. Jan 02:12:58 2013
**      by: Qt User Interface Compiler version 4.8.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QTDLGWINDOWVIEW_H
#define UI_QTDLGWINDOWVIEW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QTableWidget>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_qtDLGWindowViewClass
{
public:
    QTableWidget *tblWindowView;

    void setupUi(QWidget *qtDLGWindowViewClass)
    {
        if (qtDLGWindowViewClass->objectName().isEmpty())
            qtDLGWindowViewClass->setObjectName(QString::fromUtf8("qtDLGWindowViewClass"));
        qtDLGWindowViewClass->resize(880, 310);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/qtDLGNanomite/Icons/IDI_MAIN.ico"), QSize(), QIcon::Normal, QIcon::Off);
        qtDLGWindowViewClass->setWindowIcon(icon);
        tblWindowView = new QTableWidget(qtDLGWindowViewClass);
        if (tblWindowView->columnCount() < 5)
            tblWindowView->setColumnCount(5);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tblWindowView->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tblWindowView->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        tblWindowView->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        tblWindowView->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        tblWindowView->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        tblWindowView->setObjectName(QString::fromUtf8("tblWindowView"));
        tblWindowView->setGeometry(QRect(0, 0, 880, 310));
        tblWindowView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tblWindowView->setSelectionBehavior(QAbstractItemView::SelectRows);
        tblWindowView->horizontalHeader()->setStretchLastSection(true);
        tblWindowView->verticalHeader()->setVisible(false);
        tblWindowView->verticalHeader()->setDefaultSectionSize(15);
        tblWindowView->verticalHeader()->setMinimumSectionSize(15);

        retranslateUi(qtDLGWindowViewClass);

        QMetaObject::connectSlotsByName(qtDLGWindowViewClass);
    } // setupUi

    void retranslateUi(QWidget *qtDLGWindowViewClass)
    {
        qtDLGWindowViewClass->setWindowTitle(QApplication::translate("qtDLGWindowViewClass", "[ Nanomite ] - Window View", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem = tblWindowView->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("qtDLGWindowViewClass", "PID", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem1 = tblWindowView->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("qtDLGWindowViewClass", "Window Name", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem2 = tblWindowView->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QApplication::translate("qtDLGWindowViewClass", "Is Visible", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem3 = tblWindowView->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QApplication::translate("qtDLGWindowViewClass", "hWnd", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem4 = tblWindowView->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QApplication::translate("qtDLGWindowViewClass", "File Path", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class qtDLGWindowViewClass: public Ui_qtDLGWindowViewClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QTDLGWINDOWVIEW_H
