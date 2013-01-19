/********************************************************************************
** Form generated from reading UI file 'qtDLGBreakPointManager.ui'
**
** Created: Sat 19. Jan 02:12:58 2013
**      by: Qt User Interface Compiler version 4.8.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QTDLGBREAKPOINTMANAGER_H
#define UI_QTDLGBREAKPOINTMANAGER_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QTableWidget>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_qtDLGBreakPointManagerClass
{
public:
    QTableWidget *tblBPs;
    QGroupBox *groupBox;
    QComboBox *cbType;
    QLabel *label;
    QComboBox *cbBreakOn;
    QLabel *label_2;
    QLineEdit *leSize;
    QLabel *label_3;
    QLineEdit *lePID;
    QLabel *label_4;
    QPushButton *pbAddUpdate;
    QPushButton *pbClose;
    QLineEdit *leOffset;
    QLabel *label_5;

    void setupUi(QWidget *qtDLGBreakPointManagerClass)
    {
        if (qtDLGBreakPointManagerClass->objectName().isEmpty())
            qtDLGBreakPointManagerClass->setObjectName(QString::fromUtf8("qtDLGBreakPointManagerClass"));
        qtDLGBreakPointManagerClass->resize(688, 264);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(qtDLGBreakPointManagerClass->sizePolicy().hasHeightForWidth());
        qtDLGBreakPointManagerClass->setSizePolicy(sizePolicy);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/qtDLGNanomite/IDI_MAIN.ico"), QSize(), QIcon::Normal, QIcon::Off);
        qtDLGBreakPointManagerClass->setWindowIcon(icon);
        tblBPs = new QTableWidget(qtDLGBreakPointManagerClass);
        if (tblBPs->columnCount() < 5)
            tblBPs->setColumnCount(5);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tblBPs->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tblBPs->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        tblBPs->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        tblBPs->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        tblBPs->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        tblBPs->setObjectName(QString::fromUtf8("tblBPs"));
        tblBPs->setGeometry(QRect(3, 4, 681, 170));
        sizePolicy.setHeightForWidth(tblBPs->sizePolicy().hasHeightForWidth());
        tblBPs->setSizePolicy(sizePolicy);
        tblBPs->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tblBPs->setSelectionBehavior(QAbstractItemView::SelectRows);
        tblBPs->horizontalHeader()->setStretchLastSection(true);
        tblBPs->verticalHeader()->setVisible(false);
        tblBPs->verticalHeader()->setDefaultSectionSize(15);
        tblBPs->verticalHeader()->setMinimumSectionSize(15);
        groupBox = new QGroupBox(qtDLGBreakPointManagerClass);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(4, 178, 681, 81));
        sizePolicy.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
        groupBox->setSizePolicy(sizePolicy);
        cbType = new QComboBox(groupBox);
        cbType->setObjectName(QString::fromUtf8("cbType"));
        cbType->setGeometry(QRect(60, 20, 260, 22));
        sizePolicy.setHeightForWidth(cbType->sizePolicy().hasHeightForWidth());
        cbType->setSizePolicy(sizePolicy);
        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(19, 20, 40, 20));
        cbBreakOn = new QComboBox(groupBox);
        cbBreakOn->setObjectName(QString::fromUtf8("cbBreakOn"));
        cbBreakOn->setGeometry(QRect(400, 20, 150, 22));
        sizePolicy.setHeightForWidth(cbBreakOn->sizePolicy().hasHeightForWidth());
        cbBreakOn->setSizePolicy(sizePolicy);
        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(338, 20, 60, 20));
        leSize = new QLineEdit(groupBox);
        leSize->setObjectName(QString::fromUtf8("leSize"));
        leSize->setGeometry(QRect(400, 50, 30, 20));
        sizePolicy.setHeightForWidth(leSize->sizePolicy().hasHeightForWidth());
        leSize->setSizePolicy(sizePolicy);
        leSize->setAlignment(Qt::AlignCenter);
        leSize->setReadOnly(true);
        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(362, 50, 30, 20));
        lePID = new QLineEdit(groupBox);
        lePID->setObjectName(QString::fromUtf8("lePID"));
        lePID->setGeometry(QRect(490, 50, 60, 20));
        sizePolicy.setHeightForWidth(lePID->sizePolicy().hasHeightForWidth());
        lePID->setSizePolicy(sizePolicy);
        lePID->setAlignment(Qt::AlignCenter);
        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(450, 50, 30, 20));
        pbAddUpdate = new QPushButton(groupBox);
        pbAddUpdate->setObjectName(QString::fromUtf8("pbAddUpdate"));
        pbAddUpdate->setGeometry(QRect(560, 20, 110, 21));
        sizePolicy.setHeightForWidth(pbAddUpdate->sizePolicy().hasHeightForWidth());
        pbAddUpdate->setSizePolicy(sizePolicy);
        pbClose = new QPushButton(groupBox);
        pbClose->setObjectName(QString::fromUtf8("pbClose"));
        pbClose->setGeometry(QRect(560, 50, 111, 21));
        sizePolicy.setHeightForWidth(pbClose->sizePolicy().hasHeightForWidth());
        pbClose->setSizePolicy(sizePolicy);
        leOffset = new QLineEdit(groupBox);
        leOffset->setObjectName(QString::fromUtf8("leOffset"));
        leOffset->setGeometry(QRect(60, 50, 260, 20));
        label_5 = new QLabel(groupBox);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(10, 50, 40, 20));

        retranslateUi(qtDLGBreakPointManagerClass);

        QMetaObject::connectSlotsByName(qtDLGBreakPointManagerClass);
    } // setupUi

    void retranslateUi(QWidget *qtDLGBreakPointManagerClass)
    {
        qtDLGBreakPointManagerClass->setWindowTitle(QApplication::translate("qtDLGBreakPointManagerClass", "[ Nanomite ] - Breakpoint Manager", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem = tblBPs->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("qtDLGBreakPointManagerClass", "PID", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem1 = tblBPs->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("qtDLGBreakPointManagerClass", "Offset", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem2 = tblBPs->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QApplication::translate("qtDLGBreakPointManagerClass", "Type", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem3 = tblBPs->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QApplication::translate("qtDLGBreakPointManagerClass", "Size", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem4 = tblBPs->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QApplication::translate("qtDLGBreakPointManagerClass", "Break On", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("qtDLGBreakPointManagerClass", "Breakpoint Settings :", 0, QApplication::UnicodeUTF8));
        cbType->clear();
        cbType->insertItems(0, QStringList()
         << QApplication::translate("qtDLGBreakPointManagerClass", "Software BP - int3", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("qtDLGBreakPointManagerClass", "Hardware BP - Dr[0-3]", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("qtDLGBreakPointManagerClass", "Memory BP - Page Guard", 0, QApplication::UnicodeUTF8)
        );
        label->setText(QApplication::translate("qtDLGBreakPointManagerClass", "Type :", 0, QApplication::UnicodeUTF8));
        cbBreakOn->clear();
        cbBreakOn->insertItems(0, QStringList()
         << QApplication::translate("qtDLGBreakPointManagerClass", "Execute", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("qtDLGBreakPointManagerClass", "Write", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("qtDLGBreakPointManagerClass", "Read", 0, QApplication::UnicodeUTF8)
        );
        label_2->setText(QApplication::translate("qtDLGBreakPointManagerClass", "Break On :", 0, QApplication::UnicodeUTF8));
        leSize->setText(QApplication::translate("qtDLGBreakPointManagerClass", "0x1", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("qtDLGBreakPointManagerClass", "Size :", 0, QApplication::UnicodeUTF8));
        lePID->setText(QApplication::translate("qtDLGBreakPointManagerClass", "-1", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("qtDLGBreakPointManagerClass", "PID :", 0, QApplication::UnicodeUTF8));
        pbAddUpdate->setText(QApplication::translate("qtDLGBreakPointManagerClass", "Add / Update", 0, QApplication::UnicodeUTF8));
        pbClose->setText(QApplication::translate("qtDLGBreakPointManagerClass", "Close", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("qtDLGBreakPointManagerClass", "Offset :", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class qtDLGBreakPointManagerClass: public Ui_qtDLGBreakPointManagerClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QTDLGBREAKPOINTMANAGER_H
