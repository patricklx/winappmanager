/********************************************************************************
** Form generated from reading UI file 'settingsdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.3.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SETTINGSDIALOG_H
#define UI_SETTINGSDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SettingsDialog
{
public:
    QVBoxLayout *verticalLayout;
    QTabWidget *tabWidget;
    QWidget *tab;
    QVBoxLayout *verticalLayout_2;
    QCheckBox *checklatestversion;
    QCheckBox *checkstartwithsystem;
    QGroupBox *checkupdates;
    QVBoxLayout *verticalLayout_8;
    QCheckBox *ckdlPackage;
    QGroupBox *groupBox_2;
    QHBoxLayout *horizontalLayout_4;
    QRadioButton *checkAskClose;
    QRadioButton *checkCloseClsoe;
    QRadioButton *checkCloseMinimize;
    QWidget *tab_4;
    QVBoxLayout *verticalLayout_3;
    QCheckBox *saveDownloadedCheck;
    QCheckBox *ckShowAllApps;
    QHBoxLayout *horizontalLayout;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_11;
    QRadioButton *checkAskInstall;
    QRadioButton *checkSilentInstall;
    QRadioButton *checkAttendeInstall;
    QGroupBox *groupBox_6;
    QVBoxLayout *verticalLayout_4;
    QRadioButton *checkUpgradeAsk;
    QRadioButton *checkSilentUpgrade;
    QRadioButton *checkAttendedUpgrade;
    QGroupBox *groupBox_7;
    QVBoxLayout *verticalLayout_9;
    QRadioButton *checkUninstallAsk;
    QRadioButton *checkSilentUninstall;
    QRadioButton *checkAttendedUninstall;
    QSpacerItem *verticalSpacer_5;
    QWidget *tab_2;
    QVBoxLayout *verticalLayout_5;
    QHBoxLayout *horizontalLayout_5;
    QSpinBox *maxtaskCount;
    QLabel *label;
    QSpacerItem *horizontalSpacer_3;
    QGroupBox *groupBox_4;
    QHBoxLayout *horizontalLayout_6;
    QRadioButton *installmodeAttended;
    QRadioButton *installModeSilent;
    QSpacerItem *verticalSpacer;
    QWidget *tab_3;
    QVBoxLayout *verticalLayout_6;
    QGroupBox *proxy_settings;
    QVBoxLayout *verticalLayout_7;
    QHBoxLayout *horizontalLayout_9;
    QLabel *label_5;
    QLineEdit *proxy_hostname;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_2;
    QLineEdit *proxy_port;
    QHBoxLayout *horizontalLayout_8;
    QLabel *label_4;
    QLineEdit *proxy_username;
    QHBoxLayout *horizontalLayout_7;
    QLabel *label_3;
    QLineEdit *proxy_password;
    QHBoxLayout *horizontalLayout_10;
    QLabel *label_6;
    QComboBox *proxy_type;
    QSpacerItem *verticalSpacer_2;
    QHBoxLayout *horizontalLayout_3;
    QSpacerItem *horizontalSpacer;
    QPushButton *btApply;
    QPushButton *btCancel;
    QSpacerItem *horizontalSpacer_2;

    void setupUi(QDialog *SettingsDialog)
    {
        if (SettingsDialog->objectName().isEmpty())
            SettingsDialog->setObjectName(QStringLiteral("SettingsDialog"));
        SettingsDialog->resize(358, 303);
        QPalette palette;
        QBrush brush(QColor(255, 255, 255, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Base, brush);
        palette.setBrush(QPalette::Active, QPalette::Window, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Base, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Window, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Base, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Window, brush);
        SettingsDialog->setPalette(palette);
        verticalLayout = new QVBoxLayout(SettingsDialog);
        verticalLayout->setSpacing(0);
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        tabWidget = new QTabWidget(SettingsDialog);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        tabWidget->setEnabled(true);
        tabWidget->setTabPosition(QTabWidget::North);
        tabWidget->setTabShape(QTabWidget::Rounded);
        tab = new QWidget();
        tab->setObjectName(QStringLiteral("tab"));
        verticalLayout_2 = new QVBoxLayout(tab);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        checklatestversion = new QCheckBox(tab);
        checklatestversion->setObjectName(QStringLiteral("checklatestversion"));

        verticalLayout_2->addWidget(checklatestversion);

        checkstartwithsystem = new QCheckBox(tab);
        checkstartwithsystem->setObjectName(QStringLiteral("checkstartwithsystem"));

        verticalLayout_2->addWidget(checkstartwithsystem);

        checkupdates = new QGroupBox(tab);
        checkupdates->setObjectName(QStringLiteral("checkupdates"));
        checkupdates->setCheckable(true);
        verticalLayout_8 = new QVBoxLayout(checkupdates);
        verticalLayout_8->setObjectName(QStringLiteral("verticalLayout_8"));
        ckdlPackage = new QCheckBox(checkupdates);
        ckdlPackage->setObjectName(QStringLiteral("ckdlPackage"));

        verticalLayout_8->addWidget(ckdlPackage);


        verticalLayout_2->addWidget(checkupdates);

        groupBox_2 = new QGroupBox(tab);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        horizontalLayout_4 = new QHBoxLayout(groupBox_2);
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        checkAskClose = new QRadioButton(groupBox_2);
        checkAskClose->setObjectName(QStringLiteral("checkAskClose"));

        horizontalLayout_4->addWidget(checkAskClose);

        checkCloseClsoe = new QRadioButton(groupBox_2);
        checkCloseClsoe->setObjectName(QStringLiteral("checkCloseClsoe"));

        horizontalLayout_4->addWidget(checkCloseClsoe);

        checkCloseMinimize = new QRadioButton(groupBox_2);
        checkCloseMinimize->setObjectName(QStringLiteral("checkCloseMinimize"));

        horizontalLayout_4->addWidget(checkCloseMinimize);


        verticalLayout_2->addWidget(groupBox_2);

        tabWidget->addTab(tab, QString());
        tab_4 = new QWidget();
        tab_4->setObjectName(QStringLiteral("tab_4"));
        verticalLayout_3 = new QVBoxLayout(tab_4);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        saveDownloadedCheck = new QCheckBox(tab_4);
        saveDownloadedCheck->setObjectName(QStringLiteral("saveDownloadedCheck"));

        verticalLayout_3->addWidget(saveDownloadedCheck);

        ckShowAllApps = new QCheckBox(tab_4);
        ckShowAllApps->setObjectName(QStringLiteral("ckShowAllApps"));

        verticalLayout_3->addWidget(ckShowAllApps);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        groupBox = new QGroupBox(tab_4);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        verticalLayout_11 = new QVBoxLayout(groupBox);
        verticalLayout_11->setObjectName(QStringLiteral("verticalLayout_11"));
        checkAskInstall = new QRadioButton(groupBox);
        checkAskInstall->setObjectName(QStringLiteral("checkAskInstall"));

        verticalLayout_11->addWidget(checkAskInstall);

        checkSilentInstall = new QRadioButton(groupBox);
        checkSilentInstall->setObjectName(QStringLiteral("checkSilentInstall"));

        verticalLayout_11->addWidget(checkSilentInstall);

        checkAttendeInstall = new QRadioButton(groupBox);
        checkAttendeInstall->setObjectName(QStringLiteral("checkAttendeInstall"));

        verticalLayout_11->addWidget(checkAttendeInstall);


        horizontalLayout->addWidget(groupBox);

        groupBox_6 = new QGroupBox(tab_4);
        groupBox_6->setObjectName(QStringLiteral("groupBox_6"));
        verticalLayout_4 = new QVBoxLayout(groupBox_6);
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        checkUpgradeAsk = new QRadioButton(groupBox_6);
        checkUpgradeAsk->setObjectName(QStringLiteral("checkUpgradeAsk"));

        verticalLayout_4->addWidget(checkUpgradeAsk);

        checkSilentUpgrade = new QRadioButton(groupBox_6);
        checkSilentUpgrade->setObjectName(QStringLiteral("checkSilentUpgrade"));

        verticalLayout_4->addWidget(checkSilentUpgrade);

        checkAttendedUpgrade = new QRadioButton(groupBox_6);
        checkAttendedUpgrade->setObjectName(QStringLiteral("checkAttendedUpgrade"));

        verticalLayout_4->addWidget(checkAttendedUpgrade);


        horizontalLayout->addWidget(groupBox_6);

        groupBox_7 = new QGroupBox(tab_4);
        groupBox_7->setObjectName(QStringLiteral("groupBox_7"));
        verticalLayout_9 = new QVBoxLayout(groupBox_7);
        verticalLayout_9->setObjectName(QStringLiteral("verticalLayout_9"));
        checkUninstallAsk = new QRadioButton(groupBox_7);
        checkUninstallAsk->setObjectName(QStringLiteral("checkUninstallAsk"));

        verticalLayout_9->addWidget(checkUninstallAsk);

        checkSilentUninstall = new QRadioButton(groupBox_7);
        checkSilentUninstall->setObjectName(QStringLiteral("checkSilentUninstall"));

        verticalLayout_9->addWidget(checkSilentUninstall);

        checkAttendedUninstall = new QRadioButton(groupBox_7);
        checkAttendedUninstall->setObjectName(QStringLiteral("checkAttendedUninstall"));

        verticalLayout_9->addWidget(checkAttendedUninstall);


        horizontalLayout->addWidget(groupBox_7);


        verticalLayout_3->addLayout(horizontalLayout);

        verticalSpacer_5 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_3->addItem(verticalSpacer_5);

        tabWidget->addTab(tab_4, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QStringLiteral("tab_2"));
        verticalLayout_5 = new QVBoxLayout(tab_2);
        verticalLayout_5->setObjectName(QStringLiteral("verticalLayout_5"));
        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        maxtaskCount = new QSpinBox(tab_2);
        maxtaskCount->setObjectName(QStringLiteral("maxtaskCount"));
        maxtaskCount->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
        maxtaskCount->setCorrectionMode(QAbstractSpinBox::CorrectToPreviousValue);
        maxtaskCount->setMinimum(1);
        maxtaskCount->setMaximum(10);
        maxtaskCount->setValue(2);

        horizontalLayout_5->addWidget(maxtaskCount);

        label = new QLabel(tab_2);
        label->setObjectName(QStringLiteral("label"));

        horizontalLayout_5->addWidget(label);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_3);


        verticalLayout_5->addLayout(horizontalLayout_5);

        groupBox_4 = new QGroupBox(tab_2);
        groupBox_4->setObjectName(QStringLiteral("groupBox_4"));
        horizontalLayout_6 = new QHBoxLayout(groupBox_4);
        horizontalLayout_6->setObjectName(QStringLiteral("horizontalLayout_6"));
        installmodeAttended = new QRadioButton(groupBox_4);
        installmodeAttended->setObjectName(QStringLiteral("installmodeAttended"));

        horizontalLayout_6->addWidget(installmodeAttended);

        installModeSilent = new QRadioButton(groupBox_4);
        installModeSilent->setObjectName(QStringLiteral("installModeSilent"));

        horizontalLayout_6->addWidget(installModeSilent);


        verticalLayout_5->addWidget(groupBox_4);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_5->addItem(verticalSpacer);

        tabWidget->addTab(tab_2, QString());
        tab_3 = new QWidget();
        tab_3->setObjectName(QStringLiteral("tab_3"));
        verticalLayout_6 = new QVBoxLayout(tab_3);
        verticalLayout_6->setObjectName(QStringLiteral("verticalLayout_6"));
        proxy_settings = new QGroupBox(tab_3);
        proxy_settings->setObjectName(QStringLiteral("proxy_settings"));
        proxy_settings->setCheckable(true);
        proxy_settings->setChecked(false);
        verticalLayout_7 = new QVBoxLayout(proxy_settings);
        verticalLayout_7->setObjectName(QStringLiteral("verticalLayout_7"));
        horizontalLayout_9 = new QHBoxLayout();
        horizontalLayout_9->setObjectName(QStringLiteral("horizontalLayout_9"));
        label_5 = new QLabel(proxy_settings);
        label_5->setObjectName(QStringLiteral("label_5"));

        horizontalLayout_9->addWidget(label_5);

        proxy_hostname = new QLineEdit(proxy_settings);
        proxy_hostname->setObjectName(QStringLiteral("proxy_hostname"));

        horizontalLayout_9->addWidget(proxy_hostname);


        verticalLayout_7->addLayout(horizontalLayout_9);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        label_2 = new QLabel(proxy_settings);
        label_2->setObjectName(QStringLiteral("label_2"));

        horizontalLayout_2->addWidget(label_2);

        proxy_port = new QLineEdit(proxy_settings);
        proxy_port->setObjectName(QStringLiteral("proxy_port"));

        horizontalLayout_2->addWidget(proxy_port);


        verticalLayout_7->addLayout(horizontalLayout_2);

        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setObjectName(QStringLiteral("horizontalLayout_8"));
        label_4 = new QLabel(proxy_settings);
        label_4->setObjectName(QStringLiteral("label_4"));

        horizontalLayout_8->addWidget(label_4);

        proxy_username = new QLineEdit(proxy_settings);
        proxy_username->setObjectName(QStringLiteral("proxy_username"));

        horizontalLayout_8->addWidget(proxy_username);


        verticalLayout_7->addLayout(horizontalLayout_8);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QStringLiteral("horizontalLayout_7"));
        label_3 = new QLabel(proxy_settings);
        label_3->setObjectName(QStringLiteral("label_3"));

        horizontalLayout_7->addWidget(label_3);

        proxy_password = new QLineEdit(proxy_settings);
        proxy_password->setObjectName(QStringLiteral("proxy_password"));

        horizontalLayout_7->addWidget(proxy_password);


        verticalLayout_7->addLayout(horizontalLayout_7);

        horizontalLayout_10 = new QHBoxLayout();
        horizontalLayout_10->setObjectName(QStringLiteral("horizontalLayout_10"));
        label_6 = new QLabel(proxy_settings);
        label_6->setObjectName(QStringLiteral("label_6"));
        QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label_6->sizePolicy().hasHeightForWidth());
        label_6->setSizePolicy(sizePolicy);

        horizontalLayout_10->addWidget(label_6);

        proxy_type = new QComboBox(proxy_settings);
        proxy_type->setObjectName(QStringLiteral("proxy_type"));

        horizontalLayout_10->addWidget(proxy_type);


        verticalLayout_7->addLayout(horizontalLayout_10);


        verticalLayout_6->addWidget(proxy_settings);

        verticalSpacer_2 = new QSpacerItem(20, 48, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_6->addItem(verticalSpacer_2);

        tabWidget->addTab(tab_3, QString());

        verticalLayout->addWidget(tabWidget);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer);

        btApply = new QPushButton(SettingsDialog);
        btApply->setObjectName(QStringLiteral("btApply"));

        horizontalLayout_3->addWidget(btApply);

        btCancel = new QPushButton(SettingsDialog);
        btCancel->setObjectName(QStringLiteral("btCancel"));

        horizontalLayout_3->addWidget(btCancel);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_2);


        verticalLayout->addLayout(horizontalLayout_3);

        QWidget::setTabOrder(tabWidget, checklatestversion);
        QWidget::setTabOrder(checklatestversion, checkstartwithsystem);
        QWidget::setTabOrder(checkstartwithsystem, checkAskClose);
        QWidget::setTabOrder(checkAskClose, checkCloseClsoe);
        QWidget::setTabOrder(checkCloseClsoe, checkCloseMinimize);
        QWidget::setTabOrder(checkCloseMinimize, maxtaskCount);
        QWidget::setTabOrder(maxtaskCount, installmodeAttended);
        QWidget::setTabOrder(installmodeAttended, installModeSilent);
        QWidget::setTabOrder(installModeSilent, proxy_settings);
        QWidget::setTabOrder(proxy_settings, proxy_hostname);
        QWidget::setTabOrder(proxy_hostname, proxy_port);
        QWidget::setTabOrder(proxy_port, proxy_username);
        QWidget::setTabOrder(proxy_username, proxy_password);
        QWidget::setTabOrder(proxy_password, proxy_type);
        QWidget::setTabOrder(proxy_type, btApply);
        QWidget::setTabOrder(btApply, btCancel);
        QWidget::setTabOrder(btCancel, ckdlPackage);

        retranslateUi(SettingsDialog);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(SettingsDialog);
    } // setupUi

    void retranslateUi(QDialog *SettingsDialog)
    {
        SettingsDialog->setWindowTitle(QApplication::translate("SettingsDialog", "Dialog", 0));
        checklatestversion->setText(QApplication::translate("SettingsDialog", "Check for latest version on program start", 0));
        checkstartwithsystem->setText(QApplication::translate("SettingsDialog", "Start with System", 0));
        checkupdates->setTitle(QApplication::translate("SettingsDialog", "Check for package updates on program start", 0));
        ckdlPackage->setText(QApplication::translate("SettingsDialog", "also download new and updated packages", 0));
        groupBox_2->setTitle(QApplication::translate("SettingsDialog", "On Closing window", 0));
        checkAskClose->setText(QApplication::translate("SettingsDialog", "Ask", 0));
        checkCloseClsoe->setText(QApplication::translate("SettingsDialog", "Close", 0));
        checkCloseMinimize->setText(QApplication::translate("SettingsDialog", "Minimize to tray", 0));
        tabWidget->setTabText(tabWidget->indexOf(tab), QApplication::translate("SettingsDialog", "Main", 0));
        saveDownloadedCheck->setText(QApplication::translate("SettingsDialog", "Save downloaded Files", 0));
        ckShowAllApps->setText(QApplication::translate("SettingsDialog", "show all installed applications", 0));
        groupBox->setTitle(QApplication::translate("SettingsDialog", "Install Mode", 0));
        checkAskInstall->setText(QApplication::translate("SettingsDialog", "Ask", 0));
        checkSilentInstall->setText(QApplication::translate("SettingsDialog", "Silent", 0));
        checkAttendeInstall->setText(QApplication::translate("SettingsDialog", "Attended", 0));
        groupBox_6->setTitle(QApplication::translate("SettingsDialog", "Upgrade Mode", 0));
        checkUpgradeAsk->setText(QApplication::translate("SettingsDialog", "Ask", 0));
        checkSilentUpgrade->setText(QApplication::translate("SettingsDialog", "Silent", 0));
        checkAttendedUpgrade->setText(QApplication::translate("SettingsDialog", "Attended", 0));
        groupBox_7->setTitle(QApplication::translate("SettingsDialog", "Uninstall Mode", 0));
        checkUninstallAsk->setText(QApplication::translate("SettingsDialog", "Ask", 0));
        checkSilentUninstall->setText(QApplication::translate("SettingsDialog", "Silent", 0));
        checkAttendedUninstall->setText(QApplication::translate("SettingsDialog", "Attended", 0));
        tabWidget->setTabText(tabWidget->indexOf(tab_4), QApplication::translate("SettingsDialog", "Software", 0));
        maxtaskCount->setSuffix(QString());
        maxtaskCount->setPrefix(QString());
        label->setText(QApplication::translate("SettingsDialog", "Simultaneous downloads", 0));
        groupBox_4->setTitle(QApplication::translate("SettingsDialog", "Default install", 0));
        installmodeAttended->setText(QApplication::translate("SettingsDialog", "Attended Install", 0));
        installModeSilent->setText(QApplication::translate("SettingsDialog", "Silent Install", 0));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QApplication::translate("SettingsDialog", "Tasks", 0));
        proxy_settings->setTitle(QApplication::translate("SettingsDialog", "Proxy Settings", 0));
        label_5->setText(QApplication::translate("SettingsDialog", "HostName: ", 0));
        label_2->setText(QApplication::translate("SettingsDialog", "         Port:  ", 0));
        label_4->setText(QApplication::translate("SettingsDialog", "         User: ", 0));
        label_3->setText(QApplication::translate("SettingsDialog", " Password: ", 0));
        label_6->setText(QApplication::translate("SettingsDialog", "        Type: ", 0));
        proxy_type->clear();
        proxy_type->insertItems(0, QStringList()
         << QApplication::translate("SettingsDialog", "Socks5Proxy", 0)
         << QApplication::translate("SettingsDialog", "HttpProxy", 0)
         << QApplication::translate("SettingsDialog", "HttpCachingProxy", 0)
         << QApplication::translate("SettingsDialog", "FtpCachingProxy", 0)
        );
        tabWidget->setTabText(tabWidget->indexOf(tab_3), QApplication::translate("SettingsDialog", "Proxy", 0));
        btApply->setText(QApplication::translate("SettingsDialog", "Apply", 0));
        btCancel->setText(QApplication::translate("SettingsDialog", "Cancel", 0));
    } // retranslateUi

};

namespace Ui {
    class SettingsDialog: public Ui_SettingsDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SETTINGSDIALOG_H
