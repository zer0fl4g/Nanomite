/********************************************************************************
** Form generated from reading UI file 'qtDLGAbout.ui'
**
** Created: Sat 19. Jan 02:12:58 2013
**      by: Qt User Interface Compiler version 4.8.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QTDLGABOUT_H
#define UI_QTDLGABOUT_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>

QT_BEGIN_NAMESPACE

class Ui_qtDLGAboutUI
{
public:
    QGroupBox *groupBox;
    QLabel *label;
    QLabel *lblAboutLogo;

    void setupUi(QDialog *qtDLGAboutUI)
    {
        if (qtDLGAboutUI->objectName().isEmpty())
            qtDLGAboutUI->setObjectName(QString::fromUtf8("qtDLGAboutUI"));
        qtDLGAboutUI->resize(451, 321);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(qtDLGAboutUI->sizePolicy().hasHeightForWidth());
        qtDLGAboutUI->setSizePolicy(sizePolicy);
        groupBox = new QGroupBox(qtDLGAboutUI);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(10, 160, 430, 151));
        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(10, 10, 410, 131));
        lblAboutLogo = new QLabel(qtDLGAboutUI);
        lblAboutLogo->setObjectName(QString::fromUtf8("lblAboutLogo"));
        lblAboutLogo->setGeometry(QRect(0, 0, 450, 150));
        lblAboutLogo->setPixmap(QPixmap(QString::fromUtf8(":/qtDLGNanomite/Icons/450x150.bmp")));
        lblAboutLogo->setScaledContents(true);

        retranslateUi(qtDLGAboutUI);

        QMetaObject::connectSlotsByName(qtDLGAboutUI);
    } // setupUi

    void retranslateUi(QDialog *qtDLGAboutUI)
    {
        qtDLGAboutUI->setWindowTitle(QApplication::translate("qtDLGAboutUI", "[ Nanomite ] - About", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("qtDLGAboutUI", "<html><head/><body><p align=\"center\"><span style=\" font-size:14pt;\">Nanomite v 0.1</span></p><p>Coded by : Zer0Flag<br/></p><p>Coded in : C++ and QT<br/></p><p>Contact : zer0fl4g@gmail.com</p></body></html>", 0, QApplication::UnicodeUTF8));
        lblAboutLogo->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class qtDLGAboutUI: public Ui_qtDLGAboutUI {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QTDLGABOUT_H
