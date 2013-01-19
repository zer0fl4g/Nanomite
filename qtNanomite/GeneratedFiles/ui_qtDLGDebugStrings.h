/********************************************************************************
** Form generated from reading UI file 'qtDLGDebugStrings.ui'
**
** Created: Sat 19. Jan 02:12:58 2013
**      by: Qt User Interface Compiler version 4.8.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QTDLGDEBUGSTRINGS_H
#define UI_QTDLGDEBUGSTRINGS_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QTableWidget>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_qtDLGDebugStringsClass
{
public:
    QTableWidget *tblDebugStrings;

    void setupUi(QWidget *qtDLGDebugStringsClass)
    {
        if (qtDLGDebugStringsClass->objectName().isEmpty())
            qtDLGDebugStringsClass->setObjectName(QString::fromUtf8("qtDLGDebugStringsClass"));
        qtDLGDebugStringsClass->resize(627, 317);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/qtDLGNanomite/IDI_MAIN.ico"), QSize(), QIcon::Normal, QIcon::Off);
        qtDLGDebugStringsClass->setWindowIcon(icon);
        tblDebugStrings = new QTableWidget(qtDLGDebugStringsClass);
        if (tblDebugStrings->columnCount() < 2)
            tblDebugStrings->setColumnCount(2);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tblDebugStrings->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tblDebugStrings->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        tblDebugStrings->setObjectName(QString::fromUtf8("tblDebugStrings"));
        tblDebugStrings->setGeometry(QRect(0, 0, 629, 319));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(1);
        sizePolicy.setVerticalStretch(1);
        sizePolicy.setHeightForWidth(tblDebugStrings->sizePolicy().hasHeightForWidth());
        tblDebugStrings->setSizePolicy(sizePolicy);
        tblDebugStrings->setLayoutDirection(Qt::LeftToRight);
        tblDebugStrings->horizontalHeader()->setStretchLastSection(true);
        tblDebugStrings->verticalHeader()->setVisible(false);
        tblDebugStrings->verticalHeader()->setDefaultSectionSize(15);

        retranslateUi(qtDLGDebugStringsClass);

        QMetaObject::connectSlotsByName(qtDLGDebugStringsClass);
    } // setupUi

    void retranslateUi(QWidget *qtDLGDebugStringsClass)
    {
        qtDLGDebugStringsClass->setWindowTitle(QApplication::translate("qtDLGDebugStringsClass", "[ Nanomite ] - Debug Strings", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem = tblDebugStrings->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("qtDLGDebugStringsClass", "PID", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem1 = tblDebugStrings->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("qtDLGDebugStringsClass", "Debug String", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class qtDLGDebugStringsClass: public Ui_qtDLGDebugStringsClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QTDLGDEBUGSTRINGS_H
