/********************************************************************************
** Form generated from reading UI file 'qtDLGNanomite.ui'
**
** Created: Sat 19. Jan 02:12:58 2013
**      by: Qt User Interface Compiler version 4.8.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QTDLGNANOMITE_H
#define UI_QTDLGNANOMITE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QScrollBar>
#include <QtGui/QStatusBar>
#include <QtGui/QTableWidget>
#include <QtGui/QToolBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_qtDLGNanomiteClass
{
public:
    QAction *actionFile_OpenNew;
    QAction *actionFile_AttachTo;
    QAction *actionFile_Detach;
    QAction *actionFile_Exit;
    QAction *actionDebug_Start;
    QAction *actionDebug_Stop;
    QAction *actionDebug_Suspend;
    QAction *actionDebug_Restart;
    QAction *actionDebug_Step_In;
    QAction *actionDebug_Step_Over;
    QAction *actionOptions_Options;
    QAction *actionOptions_About;
    QAction *actionWindow_Detail_Information;
    QAction *actionWindow_Breakpoint_Manager;
    QAction *actionWindow_Show_Memory;
    QAction *actionWindow_Show_Heap;
    QAction *actionWindow_Show_Strings;
    QAction *actionWindow_Show_Debug_Output;
    QAction *actionWindow_Show_Handles;
    QAction *actionWindow_Show_Windows;
    QWidget *centralWidget;
    QTableWidget *tblLogBox;
    QTableWidget *tblStack;
    QTableWidget *tblCallstack;
    QTableWidget *tblDisAs;
    QTableWidget *tblRegView;
    QScrollBar *scrollStackView;
    QScrollBar *scrollDisAs;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuDebug;
    QMenu *menuOptions;
    QMenu *menuWindows;
    QToolBar *mainToolBar;
    QStatusBar *stateBar;

    void setupUi(QMainWindow *qtDLGNanomiteClass)
    {
        if (qtDLGNanomiteClass->objectName().isEmpty())
            qtDLGNanomiteClass->setObjectName(QString::fromUtf8("qtDLGNanomiteClass"));
        qtDLGNanomiteClass->setWindowModality(Qt::NonModal);
        qtDLGNanomiteClass->resize(1194, 798);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(qtDLGNanomiteClass->sizePolicy().hasHeightForWidth());
        qtDLGNanomiteClass->setSizePolicy(sizePolicy);
        qtDLGNanomiteClass->setMinimumSize(QSize(990, 775));
        qtDLGNanomiteClass->setSizeIncrement(QSize(10, 10));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/qtDLGNanomite/Icons/IDI_MAIN.ico"), QSize(), QIcon::Normal, QIcon::Off);
        qtDLGNanomiteClass->setWindowIcon(icon);
        actionFile_OpenNew = new QAction(qtDLGNanomiteClass);
        actionFile_OpenNew->setObjectName(QString::fromUtf8("actionFile_OpenNew"));
        actionFile_AttachTo = new QAction(qtDLGNanomiteClass);
        actionFile_AttachTo->setObjectName(QString::fromUtf8("actionFile_AttachTo"));
        actionFile_Detach = new QAction(qtDLGNanomiteClass);
        actionFile_Detach->setObjectName(QString::fromUtf8("actionFile_Detach"));
        actionFile_Exit = new QAction(qtDLGNanomiteClass);
        actionFile_Exit->setObjectName(QString::fromUtf8("actionFile_Exit"));
        actionDebug_Start = new QAction(qtDLGNanomiteClass);
        actionDebug_Start->setObjectName(QString::fromUtf8("actionDebug_Start"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/qtDLGNanomite/IDI_START.ico"), QSize(), QIcon::Normal, QIcon::Off);
        icon1.addFile(QString::fromUtf8(":/qtDLGNanomite/Icons/IDI_START.ico"), QSize(), QIcon::Normal, QIcon::On);
        actionDebug_Start->setIcon(icon1);
        actionDebug_Stop = new QAction(qtDLGNanomiteClass);
        actionDebug_Stop->setObjectName(QString::fromUtf8("actionDebug_Stop"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/qtDLGNanomite/IDI_STOP.ico"), QSize(), QIcon::Normal, QIcon::Off);
        icon2.addFile(QString::fromUtf8(":/qtDLGNanomite/Icons/IDI_STOP.ico"), QSize(), QIcon::Normal, QIcon::On);
        actionDebug_Stop->setIcon(icon2);
        actionDebug_Suspend = new QAction(qtDLGNanomiteClass);
        actionDebug_Suspend->setObjectName(QString::fromUtf8("actionDebug_Suspend"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/qtDLGNanomite/IDI_SUSPEND.ico"), QSize(), QIcon::Normal, QIcon::Off);
        icon3.addFile(QString::fromUtf8(":/qtDLGNanomite/Icons/IDI_SUSPEND.ico"), QSize(), QIcon::Normal, QIcon::On);
        actionDebug_Suspend->setIcon(icon3);
        actionDebug_Restart = new QAction(qtDLGNanomiteClass);
        actionDebug_Restart->setObjectName(QString::fromUtf8("actionDebug_Restart"));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/qtDLGNanomite/IDI_RESTART.ico"), QSize(), QIcon::Normal, QIcon::Off);
        icon4.addFile(QString::fromUtf8(":/qtDLGNanomite/Icons/IDI_RESTART.ico"), QSize(), QIcon::Normal, QIcon::On);
        actionDebug_Restart->setIcon(icon4);
        actionDebug_Step_In = new QAction(qtDLGNanomiteClass);
        actionDebug_Step_In->setObjectName(QString::fromUtf8("actionDebug_Step_In"));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/qtDLGNanomite/IDI_STEPIN.ico"), QSize(), QIcon::Normal, QIcon::Off);
        icon5.addFile(QString::fromUtf8(":/qtDLGNanomite/Icons/IDI_STEPIN.ico"), QSize(), QIcon::Normal, QIcon::On);
        actionDebug_Step_In->setIcon(icon5);
        actionDebug_Step_Over = new QAction(qtDLGNanomiteClass);
        actionDebug_Step_Over->setObjectName(QString::fromUtf8("actionDebug_Step_Over"));
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/qtDLGNanomite/IDI_STEPOVER.ico"), QSize(), QIcon::Normal, QIcon::Off);
        icon6.addFile(QString::fromUtf8(":/qtDLGNanomite/Icons/IDI_STEPOVER.ico"), QSize(), QIcon::Normal, QIcon::On);
        actionDebug_Step_Over->setIcon(icon6);
        actionOptions_Options = new QAction(qtDLGNanomiteClass);
        actionOptions_Options->setObjectName(QString::fromUtf8("actionOptions_Options"));
        actionOptions_About = new QAction(qtDLGNanomiteClass);
        actionOptions_About->setObjectName(QString::fromUtf8("actionOptions_About"));
        actionWindow_Detail_Information = new QAction(qtDLGNanomiteClass);
        actionWindow_Detail_Information->setObjectName(QString::fromUtf8("actionWindow_Detail_Information"));
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/qtDLGNanomite/ID_showInfo.ico"), QSize(), QIcon::Normal, QIcon::Off);
        icon7.addFile(QString::fromUtf8(":/qtDLGNanomite/Icons/ID_showInfo.ico"), QSize(), QIcon::Normal, QIcon::On);
        actionWindow_Detail_Information->setIcon(icon7);
        actionWindow_Breakpoint_Manager = new QAction(qtDLGNanomiteClass);
        actionWindow_Breakpoint_Manager->setObjectName(QString::fromUtf8("actionWindow_Breakpoint_Manager"));
        QIcon icon8;
        icon8.addFile(QString::fromUtf8(":/qtDLGNanomite/Icons/ID_BPManager.ico"), QSize(), QIcon::Normal, QIcon::On);
        actionWindow_Breakpoint_Manager->setIcon(icon8);
        actionWindow_Show_Memory = new QAction(qtDLGNanomiteClass);
        actionWindow_Show_Memory->setObjectName(QString::fromUtf8("actionWindow_Show_Memory"));
        QIcon icon9;
        icon9.addFile(QString::fromUtf8(":/qtDLGNanomite/Icons/ID_Memory.ico"), QSize(), QIcon::Normal, QIcon::On);
        actionWindow_Show_Memory->setIcon(icon9);
        actionWindow_Show_Heap = new QAction(qtDLGNanomiteClass);
        actionWindow_Show_Heap->setObjectName(QString::fromUtf8("actionWindow_Show_Heap"));
        QIcon icon10;
        icon10.addFile(QString::fromUtf8(":/qtDLGNanomite/Icons/ID_Heap.ico"), QSize(), QIcon::Normal, QIcon::On);
        actionWindow_Show_Heap->setIcon(icon10);
        actionWindow_Show_Strings = new QAction(qtDLGNanomiteClass);
        actionWindow_Show_Strings->setObjectName(QString::fromUtf8("actionWindow_Show_Strings"));
        QIcon icon11;
        icon11.addFile(QString::fromUtf8(":/qtDLGNanomite/Icons/ID_Strings.ico"), QSize(), QIcon::Normal, QIcon::On);
        actionWindow_Show_Strings->setIcon(icon11);
        actionWindow_Show_Debug_Output = new QAction(qtDLGNanomiteClass);
        actionWindow_Show_Debug_Output->setObjectName(QString::fromUtf8("actionWindow_Show_Debug_Output"));
        QIcon icon12;
        icon12.addFile(QString::fromUtf8(":/qtDLGNanomite/Icons/ID_DebugString.ico"), QSize(), QIcon::Normal, QIcon::On);
        actionWindow_Show_Debug_Output->setIcon(icon12);
        actionWindow_Show_Handles = new QAction(qtDLGNanomiteClass);
        actionWindow_Show_Handles->setObjectName(QString::fromUtf8("actionWindow_Show_Handles"));
        QIcon icon13;
        icon13.addFile(QString::fromUtf8(":/qtDLGNanomite/Icons/ID_Handles.ico"), QSize(), QIcon::Normal, QIcon::On);
        actionWindow_Show_Handles->setIcon(icon13);
        actionWindow_Show_Windows = new QAction(qtDLGNanomiteClass);
        actionWindow_Show_Windows->setObjectName(QString::fromUtf8("actionWindow_Show_Windows"));
        QIcon icon14;
        icon14.addFile(QString::fromUtf8(":/qtDLGNanomite/Icons/ID_Windows.ico"), QSize(), QIcon::Normal, QIcon::On);
        actionWindow_Show_Windows->setIcon(icon14);
        centralWidget = new QWidget(qtDLGNanomiteClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        tblLogBox = new QTableWidget(centralWidget);
        if (tblLogBox->columnCount() < 2)
            tblLogBox->setColumnCount(2);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tblLogBox->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tblLogBox->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        tblLogBox->setObjectName(QString::fromUtf8("tblLogBox"));
        tblLogBox->setGeometry(QRect(410, 540, 770, 180));
        QFont font;
        font.setFamily(QString::fromUtf8("Lucida Console"));
        tblLogBox->setFont(font);
        tblLogBox->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tblLogBox->setSelectionBehavior(QAbstractItemView::SelectRows);
        tblLogBox->horizontalHeader()->setStretchLastSection(true);
        tblLogBox->verticalHeader()->setVisible(false);
        tblLogBox->verticalHeader()->setDefaultSectionSize(14);
        tblLogBox->verticalHeader()->setMinimumSectionSize(14);
        tblStack = new QTableWidget(centralWidget);
        if (tblStack->columnCount() < 3)
            tblStack->setColumnCount(3);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        tblStack->setHorizontalHeaderItem(0, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        tblStack->setHorizontalHeaderItem(1, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        tblStack->setHorizontalHeaderItem(2, __qtablewidgetitem4);
        tblStack->setObjectName(QString::fromUtf8("tblStack"));
        tblStack->setGeometry(QRect(10, 540, 380, 182));
        tblStack->setFont(font);
        tblStack->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        tblStack->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        tblStack->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tblStack->setSelectionBehavior(QAbstractItemView::SelectRows);
        tblStack->horizontalHeader()->setStretchLastSection(true);
        tblStack->verticalHeader()->setVisible(false);
        tblStack->verticalHeader()->setDefaultSectionSize(14);
        tblStack->verticalHeader()->setMinimumSectionSize(14);
        tblCallstack = new QTableWidget(centralWidget);
        if (tblCallstack->columnCount() < 7)
            tblCallstack->setColumnCount(7);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        tblCallstack->setHorizontalHeaderItem(0, __qtablewidgetitem5);
        QTableWidgetItem *__qtablewidgetitem6 = new QTableWidgetItem();
        tblCallstack->setHorizontalHeaderItem(1, __qtablewidgetitem6);
        QTableWidgetItem *__qtablewidgetitem7 = new QTableWidgetItem();
        tblCallstack->setHorizontalHeaderItem(2, __qtablewidgetitem7);
        QTableWidgetItem *__qtablewidgetitem8 = new QTableWidgetItem();
        tblCallstack->setHorizontalHeaderItem(3, __qtablewidgetitem8);
        QTableWidgetItem *__qtablewidgetitem9 = new QTableWidgetItem();
        tblCallstack->setHorizontalHeaderItem(4, __qtablewidgetitem9);
        QTableWidgetItem *__qtablewidgetitem10 = new QTableWidgetItem();
        tblCallstack->setHorizontalHeaderItem(5, __qtablewidgetitem10);
        QTableWidgetItem *__qtablewidgetitem11 = new QTableWidgetItem();
        tblCallstack->setHorizontalHeaderItem(6, __qtablewidgetitem11);
        tblCallstack->setObjectName(QString::fromUtf8("tblCallstack"));
        tblCallstack->setGeometry(QRect(10, 380, 1170, 160));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(tblCallstack->sizePolicy().hasHeightForWidth());
        tblCallstack->setSizePolicy(sizePolicy1);
        tblCallstack->setFont(font);
        tblCallstack->setFrameShadow(QFrame::Sunken);
        tblCallstack->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tblCallstack->setSelectionBehavior(QAbstractItemView::SelectRows);
        tblCallstack->horizontalHeader()->setStretchLastSection(true);
        tblCallstack->verticalHeader()->setVisible(false);
        tblCallstack->verticalHeader()->setDefaultSectionSize(14);
        tblCallstack->verticalHeader()->setMinimumSectionSize(14);
        tblDisAs = new QTableWidget(centralWidget);
        if (tblDisAs->columnCount() < 4)
            tblDisAs->setColumnCount(4);
        QTableWidgetItem *__qtablewidgetitem12 = new QTableWidgetItem();
        tblDisAs->setHorizontalHeaderItem(0, __qtablewidgetitem12);
        QTableWidgetItem *__qtablewidgetitem13 = new QTableWidgetItem();
        tblDisAs->setHorizontalHeaderItem(1, __qtablewidgetitem13);
        QTableWidgetItem *__qtablewidgetitem14 = new QTableWidgetItem();
        tblDisAs->setHorizontalHeaderItem(2, __qtablewidgetitem14);
        QTableWidgetItem *__qtablewidgetitem15 = new QTableWidgetItem();
        tblDisAs->setHorizontalHeaderItem(3, __qtablewidgetitem15);
        tblDisAs->setObjectName(QString::fromUtf8("tblDisAs"));
        tblDisAs->setGeometry(QRect(10, 0, 810, 380));
        tblDisAs->setFont(font);
        tblDisAs->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tblDisAs->setSelectionBehavior(QAbstractItemView::SelectRows);
        tblDisAs->horizontalHeader()->setStretchLastSection(true);
        tblDisAs->verticalHeader()->setVisible(false);
        tblDisAs->verticalHeader()->setDefaultSectionSize(14);
        tblDisAs->verticalHeader()->setMinimumSectionSize(14);
        tblRegView = new QTableWidget(centralWidget);
        if (tblRegView->columnCount() < 2)
            tblRegView->setColumnCount(2);
        QTableWidgetItem *__qtablewidgetitem16 = new QTableWidgetItem();
        tblRegView->setHorizontalHeaderItem(0, __qtablewidgetitem16);
        QTableWidgetItem *__qtablewidgetitem17 = new QTableWidgetItem();
        tblRegView->setHorizontalHeaderItem(1, __qtablewidgetitem17);
        tblRegView->setObjectName(QString::fromUtf8("tblRegView"));
        tblRegView->setGeometry(QRect(840, 0, 340, 380));
        tblRegView->setFont(font);
        tblRegView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tblRegView->setSelectionBehavior(QAbstractItemView::SelectRows);
        tblRegView->horizontalHeader()->setStretchLastSection(true);
        tblRegView->verticalHeader()->setMinimumSectionSize(15);
        scrollStackView = new QScrollBar(centralWidget);
        scrollStackView->setObjectName(QString::fromUtf8("scrollStackView"));
        scrollStackView->setGeometry(QRect(390, 541, 20, 180));
        sizePolicy.setHeightForWidth(scrollStackView->sizePolicy().hasHeightForWidth());
        scrollStackView->setSizePolicy(sizePolicy);
        scrollStackView->setMaximum(10);
        scrollStackView->setValue(5);
        scrollStackView->setOrientation(Qt::Vertical);
        scrollDisAs = new QScrollBar(centralWidget);
        scrollDisAs->setObjectName(QString::fromUtf8("scrollDisAs"));
        scrollDisAs->setGeometry(QRect(820, 0, 20, 380));
        scrollDisAs->setMaximum(10);
        scrollDisAs->setValue(5);
        scrollDisAs->setOrientation(Qt::Vertical);
        qtDLGNanomiteClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(qtDLGNanomiteClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1194, 20));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuDebug = new QMenu(menuBar);
        menuDebug->setObjectName(QString::fromUtf8("menuDebug"));
        menuOptions = new QMenu(menuBar);
        menuOptions->setObjectName(QString::fromUtf8("menuOptions"));
        menuWindows = new QMenu(menuBar);
        menuWindows->setObjectName(QString::fromUtf8("menuWindows"));
        qtDLGNanomiteClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(qtDLGNanomiteClass);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        qtDLGNanomiteClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        stateBar = new QStatusBar(qtDLGNanomiteClass);
        stateBar->setObjectName(QString::fromUtf8("stateBar"));
        qtDLGNanomiteClass->setStatusBar(stateBar);
        QWidget::setTabOrder(tblDisAs, tblRegView);
        QWidget::setTabOrder(tblRegView, tblCallstack);
        QWidget::setTabOrder(tblCallstack, tblStack);
        QWidget::setTabOrder(tblStack, tblLogBox);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuDebug->menuAction());
        menuBar->addAction(menuWindows->menuAction());
        menuBar->addAction(menuOptions->menuAction());
        menuFile->addAction(actionFile_OpenNew);
        menuFile->addAction(actionFile_AttachTo);
        menuFile->addAction(actionFile_Detach);
        menuFile->addSeparator();
        menuFile->addAction(actionFile_Exit);
        menuDebug->addAction(actionDebug_Start);
        menuDebug->addAction(actionDebug_Suspend);
        menuDebug->addAction(actionDebug_Stop);
        menuDebug->addAction(actionDebug_Restart);
        menuDebug->addSeparator();
        menuDebug->addAction(actionDebug_Step_In);
        menuDebug->addAction(actionDebug_Step_Over);
        menuOptions->addAction(actionOptions_Options);
        menuOptions->addSeparator();
        menuOptions->addAction(actionOptions_About);
        menuWindows->addAction(actionWindow_Detail_Information);
        menuWindows->addAction(actionWindow_Breakpoint_Manager);
        menuWindows->addSeparator();
        menuWindows->addAction(actionWindow_Show_Memory);
        menuWindows->addAction(actionWindow_Show_Heap);
        menuWindows->addAction(actionWindow_Show_Strings);
        menuWindows->addAction(actionWindow_Show_Debug_Output);
        menuWindows->addAction(actionWindow_Show_Handles);
        menuWindows->addAction(actionWindow_Show_Windows);
        mainToolBar->addAction(actionDebug_Start);
        mainToolBar->addAction(actionDebug_Stop);
        mainToolBar->addAction(actionDebug_Suspend);
        mainToolBar->addAction(actionDebug_Restart);
        mainToolBar->addAction(actionDebug_Step_In);
        mainToolBar->addAction(actionDebug_Step_Over);
        mainToolBar->addSeparator();
        mainToolBar->addAction(actionWindow_Detail_Information);
        mainToolBar->addAction(actionWindow_Breakpoint_Manager);
        mainToolBar->addSeparator();
        mainToolBar->addAction(actionWindow_Show_Memory);
        mainToolBar->addAction(actionWindow_Show_Heap);
        mainToolBar->addAction(actionWindow_Show_Strings);
        mainToolBar->addAction(actionWindow_Show_Debug_Output);
        mainToolBar->addAction(actionWindow_Show_Handles);
        mainToolBar->addAction(actionWindow_Show_Windows);

        retranslateUi(qtDLGNanomiteClass);

        QMetaObject::connectSlotsByName(qtDLGNanomiteClass);
    } // setupUi

    void retranslateUi(QMainWindow *qtDLGNanomiteClass)
    {
        qtDLGNanomiteClass->setWindowTitle(QApplication::translate("qtDLGNanomiteClass", "[Nanomite v 0.1] - MainWindow", 0, QApplication::UnicodeUTF8));
        actionFile_OpenNew->setText(QApplication::translate("qtDLGNanomiteClass", "Open new File", 0, QApplication::UnicodeUTF8));
        actionFile_AttachTo->setText(QApplication::translate("qtDLGNanomiteClass", "Attach To", 0, QApplication::UnicodeUTF8));
        actionFile_Detach->setText(QApplication::translate("qtDLGNanomiteClass", "Detach", 0, QApplication::UnicodeUTF8));
        actionFile_Exit->setText(QApplication::translate("qtDLGNanomiteClass", "Exit", 0, QApplication::UnicodeUTF8));
        actionDebug_Start->setText(QApplication::translate("qtDLGNanomiteClass", "Start", 0, QApplication::UnicodeUTF8));
        actionDebug_Stop->setText(QApplication::translate("qtDLGNanomiteClass", "Stop", 0, QApplication::UnicodeUTF8));
        actionDebug_Suspend->setText(QApplication::translate("qtDLGNanomiteClass", "Suspend", 0, QApplication::UnicodeUTF8));
        actionDebug_Restart->setText(QApplication::translate("qtDLGNanomiteClass", "Restart", 0, QApplication::UnicodeUTF8));
        actionDebug_Step_In->setText(QApplication::translate("qtDLGNanomiteClass", "Step In", 0, QApplication::UnicodeUTF8));
        actionDebug_Step_Over->setText(QApplication::translate("qtDLGNanomiteClass", "Step Over", 0, QApplication::UnicodeUTF8));
        actionOptions_Options->setText(QApplication::translate("qtDLGNanomiteClass", "Options", 0, QApplication::UnicodeUTF8));
        actionOptions_About->setText(QApplication::translate("qtDLGNanomiteClass", "About", 0, QApplication::UnicodeUTF8));
        actionWindow_Detail_Information->setText(QApplication::translate("qtDLGNanomiteClass", "Detail Information", 0, QApplication::UnicodeUTF8));
        actionWindow_Breakpoint_Manager->setText(QApplication::translate("qtDLGNanomiteClass", "Breakpoint Manager", 0, QApplication::UnicodeUTF8));
        actionWindow_Show_Memory->setText(QApplication::translate("qtDLGNanomiteClass", "Show Memory", 0, QApplication::UnicodeUTF8));
        actionWindow_Show_Heap->setText(QApplication::translate("qtDLGNanomiteClass", "Show Heap", 0, QApplication::UnicodeUTF8));
        actionWindow_Show_Strings->setText(QApplication::translate("qtDLGNanomiteClass", "Show Strings", 0, QApplication::UnicodeUTF8));
        actionWindow_Show_Debug_Output->setText(QApplication::translate("qtDLGNanomiteClass", "Show Debug Output", 0, QApplication::UnicodeUTF8));
        actionWindow_Show_Handles->setText(QApplication::translate("qtDLGNanomiteClass", "Show Handles", 0, QApplication::UnicodeUTF8));
        actionWindow_Show_Windows->setText(QApplication::translate("qtDLGNanomiteClass", "Show Windows", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem = tblLogBox->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("qtDLGNanomiteClass", "Time Stamp", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem1 = tblLogBox->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("qtDLGNanomiteClass", "Log String", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem2 = tblStack->horizontalHeaderItem(0);
        ___qtablewidgetitem2->setText(QApplication::translate("qtDLGNanomiteClass", "Offset", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem3 = tblStack->horizontalHeaderItem(1);
        ___qtablewidgetitem3->setText(QApplication::translate("qtDLGNanomiteClass", "Data", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem4 = tblStack->horizontalHeaderItem(2);
        ___qtablewidgetitem4->setText(QApplication::translate("qtDLGNanomiteClass", "Comment", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem5 = tblCallstack->horizontalHeaderItem(0);
        ___qtablewidgetitem5->setText(QApplication::translate("qtDLGNanomiteClass", "Stack Offset", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem6 = tblCallstack->horizontalHeaderItem(1);
        ___qtablewidgetitem6->setText(QApplication::translate("qtDLGNanomiteClass", "Func. Addr", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem7 = tblCallstack->horizontalHeaderItem(2);
        ___qtablewidgetitem7->setText(QApplication::translate("qtDLGNanomiteClass", "<mod.func>", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem8 = tblCallstack->horizontalHeaderItem(3);
        ___qtablewidgetitem8->setText(QApplication::translate("qtDLGNanomiteClass", "Return To", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem9 = tblCallstack->horizontalHeaderItem(4);
        ___qtablewidgetitem9->setText(QApplication::translate("qtDLGNanomiteClass", "Return To - <mod.func>", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem10 = tblCallstack->horizontalHeaderItem(5);
        ___qtablewidgetitem10->setText(QApplication::translate("qtDLGNanomiteClass", "Source Line", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem11 = tblCallstack->horizontalHeaderItem(6);
        ___qtablewidgetitem11->setText(QApplication::translate("qtDLGNanomiteClass", "Source File", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem12 = tblDisAs->horizontalHeaderItem(0);
        ___qtablewidgetitem12->setText(QApplication::translate("qtDLGNanomiteClass", "Offset", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem13 = tblDisAs->horizontalHeaderItem(1);
        ___qtablewidgetitem13->setText(QApplication::translate("qtDLGNanomiteClass", "OpCodes", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem14 = tblDisAs->horizontalHeaderItem(2);
        ___qtablewidgetitem14->setText(QApplication::translate("qtDLGNanomiteClass", "Mnemonics", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem15 = tblDisAs->horizontalHeaderItem(3);
        ___qtablewidgetitem15->setText(QApplication::translate("qtDLGNanomiteClass", "Comment", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem16 = tblRegView->horizontalHeaderItem(0);
        ___qtablewidgetitem16->setText(QApplication::translate("qtDLGNanomiteClass", "Register", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem17 = tblRegView->horizontalHeaderItem(1);
        ___qtablewidgetitem17->setText(QApplication::translate("qtDLGNanomiteClass", "Value", 0, QApplication::UnicodeUTF8));
        menuFile->setTitle(QApplication::translate("qtDLGNanomiteClass", "File", 0, QApplication::UnicodeUTF8));
        menuDebug->setTitle(QApplication::translate("qtDLGNanomiteClass", "Debug", 0, QApplication::UnicodeUTF8));
        menuOptions->setTitle(QApplication::translate("qtDLGNanomiteClass", "Options", 0, QApplication::UnicodeUTF8));
        menuWindows->setTitle(QApplication::translate("qtDLGNanomiteClass", "Windows", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class qtDLGNanomiteClass: public Ui_qtDLGNanomiteClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QTDLGNANOMITE_H
