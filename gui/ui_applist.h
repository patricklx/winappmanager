/********************************************************************************
** Form generated from reading UI file 'applist.ui'
**
** Created by: Qt User Interface Compiler version 5.3.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_APPLIST_H
#define UI_APPLIST_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCommandLinkButton>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "TaskList.h"
#include "mytreewidget.h"

QT_BEGIN_NAMESPACE

class Ui_AppList
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QPushButton *btReload;
    QPushButton *btUpdateInfo;
    QPushButton *btUpdate;
    QPushButton *btUpgrade;
    QPushButton *btnTask;
    QCommandLinkButton *startTasksBtn;
    QSpacerItem *horizontalSpacer;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout_2;
    QRadioButton *automaticRb;
    QRadioButton *manualRb;
    QLabel *lbLoad;
    QProgressBar *progressBar;
    QLineEdit *lbSearch;
    QSplitter *MainSplitter;
    QTreeWidget *TCategoryTree;
    QSplitter *splitter;
    QStackedWidget *stackedWidget;
    QWidget *page;
    QVBoxLayout *verticalLayout_4;
    TaskList *taskList;
    QWidget *page_2;
    QVBoxLayout *verticalLayout_3;
    QWidget *appListInfo;
    QVBoxLayout *verticalLayout_2;
    QSplitter *splitter_2;
    MyTreeWidget *LAppInfoList;
    QTextBrowser *AppInfoText;

    void setupUi(QWidget *AppList)
    {
        if (AppList->objectName().isEmpty())
            AppList->setObjectName(QStringLiteral("AppList"));
        AppList->resize(981, 454);
        QPalette palette;
        QBrush brush(QColor(255, 255, 255, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Base, brush);
        palette.setBrush(QPalette::Active, QPalette::Window, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Base, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Window, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Base, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Window, brush);
        AppList->setPalette(palette);
        verticalLayout = new QVBoxLayout(AppList);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        btReload = new QPushButton(AppList);
        btReload->setObjectName(QStringLiteral("btReload"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(btReload->sizePolicy().hasHeightForWidth());
        btReload->setSizePolicy(sizePolicy);
        QIcon icon;
        icon.addFile(QStringLiteral(":/icons/CheckNew.ico"), QSize(), QIcon::Normal, QIcon::Off);
        btReload->setIcon(icon);
        btReload->setIconSize(QSize(32, 32));

        horizontalLayout->addWidget(btReload);

        btUpdateInfo = new QPushButton(AppList);
        btUpdateInfo->setObjectName(QStringLiteral("btUpdateInfo"));
        sizePolicy.setHeightForWidth(btUpdateInfo->sizePolicy().hasHeightForWidth());
        btUpdateInfo->setSizePolicy(sizePolicy);
        btUpdateInfo->setLayoutDirection(Qt::RightToLeft);
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/icons/isnew.ico"), QSize(), QIcon::Normal, QIcon::Off);
        btUpdateInfo->setIcon(icon1);
        btUpdateInfo->setIconSize(QSize(32, 32));

        horizontalLayout->addWidget(btUpdateInfo);

        btUpdate = new QPushButton(AppList);
        btUpdate->setObjectName(QStringLiteral("btUpdate"));
        sizePolicy.setHeightForWidth(btUpdate->sizePolicy().hasHeightForWidth());
        btUpdate->setSizePolicy(sizePolicy);
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/icons/hasupdate.ico"), QSize(), QIcon::Normal, QIcon::Off);
        btUpdate->setIcon(icon2);
        btUpdate->setIconSize(QSize(32, 32));

        horizontalLayout->addWidget(btUpdate);

        btUpgrade = new QPushButton(AppList);
        btUpgrade->setObjectName(QStringLiteral("btUpgrade"));
        sizePolicy.setHeightForWidth(btUpgrade->sizePolicy().hasHeightForWidth());
        btUpgrade->setSizePolicy(sizePolicy);
        QIcon icon3;
        icon3.addFile(QStringLiteral(":/icons/Upgrade.ico"), QSize(), QIcon::Normal, QIcon::Off);
        btUpgrade->setIcon(icon3);
        btUpgrade->setIconSize(QSize(32, 32));

        horizontalLayout->addWidget(btUpgrade);

        btnTask = new QPushButton(AppList);
        btnTask->setObjectName(QStringLiteral("btnTask"));
        sizePolicy.setHeightForWidth(btnTask->sizePolicy().hasHeightForWidth());
        btnTask->setSizePolicy(sizePolicy);
        QIcon icon4;
        icon4.addFile(QStringLiteral(":/icons/WinApp_Manager.ico"), QSize(), QIcon::Normal, QIcon::Off);
        btnTask->setIcon(icon4);
        btnTask->setIconSize(QSize(32, 32));

        horizontalLayout->addWidget(btnTask);

        startTasksBtn = new QCommandLinkButton(AppList);
        startTasksBtn->setObjectName(QStringLiteral("startTasksBtn"));
        QSizePolicy sizePolicy1(QSizePolicy::Maximum, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(startTasksBtn->sizePolicy().hasHeightForWidth());
        startTasksBtn->setSizePolicy(sizePolicy1);

        horizontalLayout->addWidget(startTasksBtn);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        groupBox = new QGroupBox(AppList);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        horizontalLayout_2 = new QHBoxLayout(groupBox);
#ifndef Q_OS_MAC
        horizontalLayout_2->setSpacing(6);
#endif
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        automaticRb = new QRadioButton(groupBox);
        automaticRb->setObjectName(QStringLiteral("automaticRb"));
        automaticRb->setChecked(true);

        horizontalLayout_2->addWidget(automaticRb);

        manualRb = new QRadioButton(groupBox);
        manualRb->setObjectName(QStringLiteral("manualRb"));

        horizontalLayout_2->addWidget(manualRb);


        horizontalLayout->addWidget(groupBox);

        lbLoad = new QLabel(AppList);
        lbLoad->setObjectName(QStringLiteral("lbLoad"));
        QFont font;
        font.setFamily(QStringLiteral("Verdana"));
        font.setPointSize(10);
        font.setBold(true);
        font.setWeight(75);
        lbLoad->setFont(font);

        horizontalLayout->addWidget(lbLoad);

        progressBar = new QProgressBar(AppList);
        progressBar->setObjectName(QStringLiteral("progressBar"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(progressBar->sizePolicy().hasHeightForWidth());
        progressBar->setSizePolicy(sizePolicy2);
        progressBar->setMinimumSize(QSize(300, 0));
        progressBar->setMinimum(0);
        progressBar->setMaximum(0);
        progressBar->setValue(-1);
        progressBar->setTextVisible(true);

        horizontalLayout->addWidget(progressBar);

        lbSearch = new QLineEdit(AppList);
        lbSearch->setObjectName(QStringLiteral("lbSearch"));
        lbSearch->setMinimumSize(QSize(0, 25));
        lbSearch->setMaximumSize(QSize(200, 16777215));
        lbSearch->setBaseSize(QSize(150, 0));
        QFont font1;
        font1.setPointSize(10);
        lbSearch->setFont(font1);
        lbSearch->setFrame(true);

        horizontalLayout->addWidget(lbSearch);


        verticalLayout->addLayout(horizontalLayout);

        MainSplitter = new QSplitter(AppList);
        MainSplitter->setObjectName(QStringLiteral("MainSplitter"));
        QSizePolicy sizePolicy3(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(MainSplitter->sizePolicy().hasHeightForWidth());
        MainSplitter->setSizePolicy(sizePolicy3);
        MainSplitter->setFrameShape(QFrame::NoFrame);
        MainSplitter->setOrientation(Qt::Horizontal);
        MainSplitter->setOpaqueResize(true);
        MainSplitter->setHandleWidth(5);
        MainSplitter->setChildrenCollapsible(false);
        TCategoryTree = new QTreeWidget(MainSplitter);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QStringLiteral("1"));
        TCategoryTree->setHeaderItem(__qtreewidgetitem);
        TCategoryTree->setObjectName(QStringLiteral("TCategoryTree"));
        QSizePolicy sizePolicy4(QSizePolicy::Maximum, QSizePolicy::Maximum);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(TCategoryTree->sizePolicy().hasHeightForWidth());
        TCategoryTree->setSizePolicy(sizePolicy4);
        TCategoryTree->setMinimumSize(QSize(0, 0));
        TCategoryTree->setMaximumSize(QSize(175, 16777215));
        TCategoryTree->setBaseSize(QSize(0, 0));
        TCategoryTree->setFont(font1);
        TCategoryTree->setRootIsDecorated(true);
        TCategoryTree->setUniformRowHeights(true);
        MainSplitter->addWidget(TCategoryTree);
        TCategoryTree->header()->setVisible(false);
        TCategoryTree->header()->setCascadingSectionResizes(false);
        TCategoryTree->header()->setDefaultSectionSize(50);
        TCategoryTree->header()->setHighlightSections(true);
        TCategoryTree->header()->setMinimumSectionSize(0);
        splitter = new QSplitter(MainSplitter);
        splitter->setObjectName(QStringLiteral("splitter"));
        QSizePolicy sizePolicy5(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy5.setHorizontalStretch(0);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(splitter->sizePolicy().hasHeightForWidth());
        splitter->setSizePolicy(sizePolicy5);
        splitter->setOrientation(Qt::Vertical);
        splitter->setChildrenCollapsible(false);
        stackedWidget = new QStackedWidget(splitter);
        stackedWidget->setObjectName(QStringLiteral("stackedWidget"));
        page = new QWidget();
        page->setObjectName(QStringLiteral("page"));
        verticalLayout_4 = new QVBoxLayout(page);
        verticalLayout_4->setSpacing(0);
        verticalLayout_4->setContentsMargins(0, 0, 0, 0);
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        taskList = new TaskList(page);
        taskList->setObjectName(QStringLiteral("taskList"));

        verticalLayout_4->addWidget(taskList);

        stackedWidget->addWidget(page);
        page_2 = new QWidget();
        page_2->setObjectName(QStringLiteral("page_2"));
        verticalLayout_3 = new QVBoxLayout(page_2);
        verticalLayout_3->setSpacing(0);
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        appListInfo = new QWidget(page_2);
        appListInfo->setObjectName(QStringLiteral("appListInfo"));
        verticalLayout_2 = new QVBoxLayout(appListInfo);
        verticalLayout_2->setSpacing(0);
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        splitter_2 = new QSplitter(appListInfo);
        splitter_2->setObjectName(QStringLiteral("splitter_2"));
        splitter_2->setOrientation(Qt::Vertical);
        LAppInfoList = new MyTreeWidget(splitter_2);
        LAppInfoList->setObjectName(QStringLiteral("LAppInfoList"));
        LAppInfoList->setFont(font1);
        LAppInfoList->setContextMenuPolicy(Qt::CustomContextMenu);
        LAppInfoList->setEditTriggers(QAbstractItemView::NoEditTriggers);
        LAppInfoList->setSelectionMode(QAbstractItemView::ExtendedSelection);
        LAppInfoList->setIconSize(QSize(24, 24));
        LAppInfoList->setTextElideMode(Qt::ElideRight);
        LAppInfoList->setRootIsDecorated(false);
        LAppInfoList->setUniformRowHeights(true);
        LAppInfoList->setItemsExpandable(false);
        LAppInfoList->setSortingEnabled(true);
        LAppInfoList->setColumnCount(2);
        splitter_2->addWidget(LAppInfoList);
        LAppInfoList->header()->setCascadingSectionResizes(true);
        LAppInfoList->header()->setDefaultSectionSize(100);
        LAppInfoList->header()->setStretchLastSection(false);
        AppInfoText = new QTextBrowser(splitter_2);
        AppInfoText->setObjectName(QStringLiteral("AppInfoText"));
        QFont font2;
        font2.setFamily(QStringLiteral("Helvetica"));
        font2.setPointSize(10);
        AppInfoText->setFont(font2);
        AppInfoText->setFrameShape(QFrame::Box);
        AppInfoText->setFrameShadow(QFrame::Plain);
        AppInfoText->setAutoFormatting(QTextEdit::AutoAll);
        AppInfoText->setReadOnly(true);
        AppInfoText->setAcceptRichText(false);
        AppInfoText->setTextInteractionFlags(Qt::TextBrowserInteraction);
        AppInfoText->setOpenExternalLinks(true);
        splitter_2->addWidget(AppInfoText);

        verticalLayout_2->addWidget(splitter_2);


        verticalLayout_3->addWidget(appListInfo);

        stackedWidget->addWidget(page_2);
        splitter->addWidget(stackedWidget);
        MainSplitter->addWidget(splitter);

        verticalLayout->addWidget(MainSplitter);


        retranslateUi(AppList);

        stackedWidget->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(AppList);
    } // setupUi

    void retranslateUi(QWidget *AppList)
    {
        AppList->setWindowTitle(QApplication::translate("AppList", "Form", 0));
#ifndef QT_NO_TOOLTIP
        btReload->setToolTip(QApplication::translate("AppList", "Reload list", 0));
#endif // QT_NO_TOOLTIP
        btReload->setText(QString());
#ifndef QT_NO_TOOLTIP
        btUpdateInfo->setToolTip(QApplication::translate("AppList", "Update Info", 0));
#endif // QT_NO_TOOLTIP
        btUpdateInfo->setText(QString());
#ifndef QT_NO_TOOLTIP
        btUpdate->setToolTip(QApplication::translate("AppList", "Update Versions", 0));
#endif // QT_NO_TOOLTIP
        btUpdate->setText(QString());
#ifndef QT_NO_TOOLTIP
        btUpgrade->setToolTip(QApplication::translate("AppList", "Select Updates", 0));
#endif // QT_NO_TOOLTIP
        btUpgrade->setText(QString());
        btnTask->setText(QApplication::translate("AppList", "Tasks", 0));
        btnTask->setShortcut(QApplication::translate("AppList", "Alt+CapsLock, Alt+Shift+R", 0));
        startTasksBtn->setText(QApplication::translate("AppList", "Start Tasks", 0));
        groupBox->setTitle(QApplication::translate("AppList", "first:", 0));
        automaticRb->setText(QApplication::translate("AppList", "automatic", 0));
        manualRb->setText(QApplication::translate("AppList", "manual", 0));
        lbLoad->setText(QApplication::translate("AppList", "Loading: ", 0));
        progressBar->setFormat(QApplication::translate("AppList", "%v/%m", 0));
        lbSearch->setPlaceholderText(QApplication::translate("AppList", "type here to search", 0));
        QTreeWidgetItem *___qtreewidgetitem = LAppInfoList->headerItem();
        ___qtreewidgetitem->setText(1, QApplication::translate("AppList", "Actions", 0));
        ___qtreewidgetitem->setText(0, QApplication::translate("AppList", "Name", 0));
        AppInfoText->setHtml(QApplication::translate("AppList", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Helvetica'; font-size:10pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'MS Shell Dlg 2';\">Hi!</span></p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:'MS Shell Dlg 2';\"><br /></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'MS Shell Dlg 2';\">I created a website where everyone can help to keep the information up-to-date:</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; "
                        "margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><a href=\"http://appdriverupdate.sourceforge.net/Apps.php\"><span style=\" font-family:'MS Shell Dlg 2'; text-decoration: underline; color:#0000ff;\">http://appdriverupdate.sourceforge.net/Apps.php</span></a></p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:'MS Shell Dlg 2'; text-decoration: underline; color:#0000ff;\"><br /></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'MS Shell Dlg 2';\">You can ADD and/or EDIT the Software info, just register and login.</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'MS Shell Dlg 2';\">The information will be verified by me and then made available for the program.</"
                        "span></p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:'MS Shell Dlg 2';\"><br /></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'MS Shell Dlg 2';\">It's also possible to just request an unsupported program, we wil then try to support it.</span></p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:'MS Shell Dlg 2';\"><br /></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'MS Shell Dlg 2';\">If you want to help verifying the information, just send me an email to add you to the 'verifier' list.</span></p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0p"
                        "x; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:'MS Shell Dlg 2';\"><br /></p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:'MS Shell Dlg 2';\"><br /></p></body></html>", 0));
    } // retranslateUi

};

namespace Ui {
    class AppList: public Ui_AppList {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_APPLIST_H
