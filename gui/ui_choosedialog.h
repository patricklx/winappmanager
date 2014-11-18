/********************************************************************************
** Form generated from reading UI file 'choosedialog.ui'
**
** Created by: Qt User Interface Compiler version 5.3.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CHOOSEDIALOG_H
#define UI_CHOOSEDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_ChooseDialog
{
public:
    QVBoxLayout *verticalLayout;
    QListWidget *listWidget;
    QHBoxLayout *horizontalLayout;
    QPushButton *btOk;
    QPushButton *btCancel;
    QCheckBox *checkInstall;
    QCheckBox *checkSilent;

    void setupUi(QDialog *ChooseDialog)
    {
        if (ChooseDialog->objectName().isEmpty())
            ChooseDialog->setObjectName(QStringLiteral("ChooseDialog"));
        ChooseDialog->resize(302, 133);
        verticalLayout = new QVBoxLayout(ChooseDialog);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        listWidget = new QListWidget(ChooseDialog);
        listWidget->setObjectName(QStringLiteral("listWidget"));

        verticalLayout->addWidget(listWidget);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        btOk = new QPushButton(ChooseDialog);
        btOk->setObjectName(QStringLiteral("btOk"));

        horizontalLayout->addWidget(btOk);

        btCancel = new QPushButton(ChooseDialog);
        btCancel->setObjectName(QStringLiteral("btCancel"));

        horizontalLayout->addWidget(btCancel);

        checkInstall = new QCheckBox(ChooseDialog);
        checkInstall->setObjectName(QStringLiteral("checkInstall"));

        horizontalLayout->addWidget(checkInstall);

        checkSilent = new QCheckBox(ChooseDialog);
        checkSilent->setObjectName(QStringLiteral("checkSilent"));

        horizontalLayout->addWidget(checkSilent);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(ChooseDialog);

        QMetaObject::connectSlotsByName(ChooseDialog);
    } // setupUi

    void retranslateUi(QDialog *ChooseDialog)
    {
        ChooseDialog->setWindowTitle(QApplication::translate("ChooseDialog", "Choose", 0));
        btOk->setText(QApplication::translate("ChooseDialog", "OK", 0));
        btCancel->setText(QApplication::translate("ChooseDialog", "Cancel", 0));
        checkInstall->setText(QApplication::translate("ChooseDialog", "+install", 0));
        checkSilent->setText(QApplication::translate("ChooseDialog", "+silent", 0));
    } // retranslateUi

};

namespace Ui {
    class ChooseDialog: public Ui_ChooseDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CHOOSEDIALOG_H
