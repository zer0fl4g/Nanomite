/********************************************************************************
** Form generated from reading UI file 'qtDLGDetailInfo.ui'
**
** Created: Sat 19. Jan 02:12:58 2013
**      by: Qt User Interface Compiler version 4.8.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QTDLGDETAILINFO_H
#define UI_QTDLGDETAILINFO_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QTableWidget>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_qtDLGDetailInfoClass
{
public:
    QTableWidget *tblPIDs;
    QTableWidget *tblTIDs;
    QTableWidget *tblExceptions;
    QTableWidget *tblModules;

    void setupUi(QWidget *qtDLGDetailInfoClass)
    {
        if (qtDLGDetailInfoClass->objectName().isEmpty())
            qtDLGDetailInfoClass->setObjectName(QString::fromUtf8("qtDLGDetailInfoClass"));
        qtDLGDetailInfoClass->resize(799, 610);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(qtDLGDetailInfoClass->sizePolicy().hasHeightForWidth());
        qtDLGDetailInfoClass->setSizePolicy(sizePolicy);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/qtDLGNanomite/IDI_MAIN.ico"), QSize(), QIcon::Normal, QIcon::Off);
        qtDLGDetailInfoClass->setWindowIcon(icon);
        tblPIDs = new QTableWidget(qtDLGDetailInfoClass);
        if (tblPIDs->columnCount() < 4)
            tblPIDs->setColumnCount(4);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tblPIDs->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tblPIDs->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        tblPIDs->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        tblPIDs->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        tblPIDs->setObjectName(QString::fromUtf8("tblPIDs"));
        tblPIDs->setGeometry(QRect(0, 0, 800, 110));
        sizePolicy.setHeightForWidth(tblPIDs->sizePolicy().hasHeightForWidth());
        tblPIDs->setSizePolicy(sizePolicy);
        QFont font;
        font.setFamily(QString::fromUtf8("Lucida Console"));
        tblPIDs->setFont(font);
        tblPIDs->horizontalHeader()->setMinimumSectionSize(15);
        tblPIDs->horizontalHeader()->setStretchLastSection(true);
        tblPIDs->verticalHeader()->setVisible(false);
        tblPIDs->verticalHeader()->setDefaultSectionSize(15);
        tblTIDs = new QTableWidget(qtDLGDetailInfoClass);
        if (tblTIDs->columnCount() < 5)
            tblTIDs->setColumnCount(5);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        tblTIDs->setHorizontalHeaderItem(0, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        tblTIDs->setHorizontalHeaderItem(1, __qtablewidgetitem5);
        QTableWidgetItem *__qtablewidgetitem6 = new QTableWidgetItem();
        tblTIDs->setHorizontalHeaderItem(2, __qtablewidgetitem6);
        QTableWidgetItem *__qtablewidgetitem7 = new QTableWidgetItem();
        tblTIDs->setHorizontalHeaderItem(3, __qtablewidgetitem7);
        QTableWidgetItem *__qtablewidgetitem8 = new QTableWidgetItem();
        tblTIDs->setHorizontalHeaderItem(4, __qtablewidgetitem8);
        tblTIDs->setObjectName(QString::fromUtf8("tblTIDs"));
        tblTIDs->setGeometry(QRect(0, 110, 800, 150));
        sizePolicy.setHeightForWidth(tblTIDs->sizePolicy().hasHeightForWidth());
        tblTIDs->setSizePolicy(sizePolicy);
        tblTIDs->setFont(font);
        tblTIDs->horizontalHeader()->setMinimumSectionSize(15);
        tblTIDs->horizontalHeader()->setStretchLastSection(true);
        tblTIDs->verticalHeader()->setVisible(false);
        tblTIDs->verticalHeader()->setDefaultSectionSize(15);
        tblExceptions = new QTableWidget(qtDLGDetailInfoClass);
        if (tblExceptions->columnCount() < 4)
            tblExceptions->setColumnCount(4);
        QTableWidgetItem *__qtablewidgetitem9 = new QTableWidgetItem();
        tblExceptions->setHorizontalHeaderItem(0, __qtablewidgetitem9);
        QTableWidgetItem *__qtablewidgetitem10 = new QTableWidgetItem();
        tblExceptions->setHorizontalHeaderItem(1, __qtablewidgetitem10);
        QTableWidgetItem *__qtablewidgetitem11 = new QTableWidgetItem();
        tblExceptions->setHorizontalHeaderItem(2, __qtablewidgetitem11);
        QTableWidgetItem *__qtablewidgetitem12 = new QTableWidgetItem();
        tblExceptions->setHorizontalHeaderItem(3, __qtablewidgetitem12);
        tblExceptions->setObjectName(QString::fromUtf8("tblExceptions"));
        tblExceptions->setGeometry(QRect(0, 260, 800, 150));
        sizePolicy.setHeightForWidth(tblExceptions->sizePolicy().hasHeightForWidth());
        tblExceptions->setSizePolicy(sizePolicy);
        tblExceptions->setFont(font);
        tblExceptions->horizontalHeader()->setMinimumSectionSize(15);
        tblExceptions->horizontalHeader()->setStretchLastSection(true);
        tblExceptions->verticalHeader()->setVisible(false);
        tblExceptions->verticalHeader()->setDefaultSectionSize(15);
        tblModules = new QTableWidget(qtDLGDetailInfoClass);
        if (tblModules->columnCount() < 4)
            tblModules->setColumnCount(4);
        QTableWidgetItem *__qtablewidgetitem13 = new QTableWidgetItem();
        tblModules->setHorizontalHeaderItem(0, __qtablewidgetitem13);
        QTableWidgetItem *__qtablewidgetitem14 = new QTableWidgetItem();
        tblModules->setHorizontalHeaderItem(1, __qtablewidgetitem14);
        QTableWidgetItem *__qtablewidgetitem15 = new QTableWidgetItem();
        tblModules->setHorizontalHeaderItem(2, __qtablewidgetitem15);
        QTableWidgetItem *__qtablewidgetitem16 = new QTableWidgetItem();
        tblModules->setHorizontalHeaderItem(3, __qtablewidgetitem16);
        tblModules->setObjectName(QString::fromUtf8("tblModules"));
        tblModules->setGeometry(QRect(0, 410, 800, 200));
        sizePolicy.setHeightForWidth(tblModules->sizePolicy().hasHeightForWidth());
        tblModules->setSizePolicy(sizePolicy);
        tblModules->setFont(font);
        tblModules->horizontalHeader()->setMinimumSectionSize(15);
        tblModules->horizontalHeader()->setStretchLastSection(true);
        tblModules->verticalHeader()->setVisible(false);
        tblModules->verticalHeader()->setDefaultSectionSize(15);

        retranslateUi(qtDLGDetailInfoClass);

        QMetaObject::connectSlotsByName(qtDLGDetailInfoClass);
    } // setupUi

    void retranslateUi(QWidget *qtDLGDetailInfoClass)
    {
        qtDLGDetailInfoClass->setWindowTitle(QApplication::translate("qtDLGDetailInfoClass", "[ Nanomite ] - Detail Information - Processes / Threads / Exceptions / Modules", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem = tblPIDs->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("qtDLGDetailInfoClass", "PID", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem1 = tblPIDs->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("qtDLGDetailInfoClass", "EntryPoint", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem2 = tblPIDs->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QApplication::translate("qtDLGDetailInfoClass", "ExitCode", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem3 = tblPIDs->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QApplication::translate("qtDLGDetailInfoClass", "FilePath", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem4 = tblTIDs->horizontalHeaderItem(0);
        ___qtablewidgetitem4->setText(QApplication::translate("qtDLGDetailInfoClass", "PID", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem5 = tblTIDs->horizontalHeaderItem(1);
        ___qtablewidgetitem5->setText(QApplication::translate("qtDLGDetailInfoClass", "TID", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem6 = tblTIDs->horizontalHeaderItem(2);
        ___qtablewidgetitem6->setText(QApplication::translate("qtDLGDetailInfoClass", "EntryPoint", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem7 = tblTIDs->horizontalHeaderItem(3);
        ___qtablewidgetitem7->setText(QApplication::translate("qtDLGDetailInfoClass", "ExitCode", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem8 = tblTIDs->horizontalHeaderItem(4);
        ___qtablewidgetitem8->setText(QApplication::translate("qtDLGDetailInfoClass", "State", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem9 = tblExceptions->horizontalHeaderItem(0);
        ___qtablewidgetitem9->setText(QApplication::translate("qtDLGDetailInfoClass", "Offset", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem10 = tblExceptions->horizontalHeaderItem(1);
        ___qtablewidgetitem10->setText(QApplication::translate("qtDLGDetailInfoClass", "Exc. Code", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem11 = tblExceptions->horizontalHeaderItem(2);
        ___qtablewidgetitem11->setText(QApplication::translate("qtDLGDetailInfoClass", "PID / TID", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem12 = tblExceptions->horizontalHeaderItem(3);
        ___qtablewidgetitem12->setText(QApplication::translate("qtDLGDetailInfoClass", "Module.Function", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem13 = tblModules->horizontalHeaderItem(0);
        ___qtablewidgetitem13->setText(QApplication::translate("qtDLGDetailInfoClass", "PID", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem14 = tblModules->horizontalHeaderItem(1);
        ___qtablewidgetitem14->setText(QApplication::translate("qtDLGDetailInfoClass", "EntryPoint", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem15 = tblModules->horizontalHeaderItem(2);
        ___qtablewidgetitem15->setText(QApplication::translate("qtDLGDetailInfoClass", "State", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem16 = tblModules->horizontalHeaderItem(3);
        ___qtablewidgetitem16->setText(QApplication::translate("qtDLGDetailInfoClass", "Path", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class qtDLGDetailInfoClass: public Ui_qtDLGDetailInfoClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QTDLGDETAILINFO_H
