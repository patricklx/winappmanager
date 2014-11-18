/********************************************************************************
** Form generated from reading UI file 'updaterdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.3.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UPDATERDIALOG_H
#define UI_UPDATERDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_UpdaterDialog
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QProgressBar *progressBar;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *btCancel;
    QSpacerItem *horizontalSpacer;

    void setupUi(QDialog *UpdaterDialog)
    {
        if (UpdaterDialog->objectName().isEmpty())
            UpdaterDialog->setObjectName(QStringLiteral("UpdaterDialog"));
        UpdaterDialog->resize(398, 101);
        verticalLayout = new QVBoxLayout(UpdaterDialog);
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        label = new QLabel(UpdaterDialog);
        label->setObjectName(QStringLiteral("label"));
        QFont font;
        font.setFamily(QStringLiteral("Courier New"));
        font.setPointSize(10);
        font.setBold(true);
        font.setItalic(false);
        font.setWeight(75);
        label->setFont(font);

        verticalLayout->addWidget(label);

        progressBar = new QProgressBar(UpdaterDialog);
        progressBar->setObjectName(QStringLiteral("progressBar"));
        progressBar->setMinimumSize(QSize(0, 30));
        progressBar->setValue(0);
        progressBar->setTextVisible(false);
        progressBar->setInvertedAppearance(false);

        verticalLayout->addWidget(progressBar);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        btCancel = new QPushButton(UpdaterDialog);
        btCancel->setObjectName(QStringLiteral("btCancel"));

        horizontalLayout->addWidget(btCancel);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(UpdaterDialog);

        QMetaObject::connectSlotsByName(UpdaterDialog);
    } // setupUi

    void retranslateUi(QDialog *UpdaterDialog)
    {
        UpdaterDialog->setWindowTitle(QApplication::translate("UpdaterDialog", "WinApp_Manager update checker", 0));
        label->setText(QString());
        progressBar->setFormat(QString());
        btCancel->setText(QApplication::translate("UpdaterDialog", "Cancel", 0));
    } // retranslateUi

};

namespace Ui {
    class UpdaterDialog: public Ui_UpdaterDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_UPDATERDIALOG_H
