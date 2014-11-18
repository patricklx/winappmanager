/********************************************************************************
** Form generated from reading UI file 'tasklist.ui'
**
** Created by: Qt User Interface Compiler version 5.3.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TASKLIST_H
#define UI_TASKLIST_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TaskList
{
public:
    QVBoxLayout *verticalLayout;
    QTreeWidget *LTaskList;

    void setupUi(QWidget *TaskList)
    {
        if (TaskList->objectName().isEmpty())
            TaskList->setObjectName(QStringLiteral("TaskList"));
        TaskList->resize(629, 432);
        QPalette palette;
        QBrush brush(QColor(255, 255, 255, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Base, brush);
        palette.setBrush(QPalette::Active, QPalette::Window, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Base, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Window, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Base, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Window, brush);
        TaskList->setPalette(palette);
        verticalLayout = new QVBoxLayout(TaskList);
        verticalLayout->setSpacing(0);
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        LTaskList = new QTreeWidget(TaskList);
        LTaskList->setObjectName(QStringLiteral("LTaskList"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(LTaskList->sizePolicy().hasHeightForWidth());
        LTaskList->setSizePolicy(sizePolicy);
        LTaskList->setContextMenuPolicy(Qt::CustomContextMenu);
        LTaskList->setSelectionMode(QAbstractItemView::ExtendedSelection);
        LTaskList->setIconSize(QSize(20, 20));
        LTaskList->setTextElideMode(Qt::ElideRight);
        LTaskList->setIndentation(0);
        LTaskList->setRootIsDecorated(false);
        LTaskList->setUniformRowHeights(true);
        LTaskList->setItemsExpandable(false);
        LTaskList->setWordWrap(false);
        LTaskList->header()->setCascadingSectionResizes(false);
        LTaskList->header()->setDefaultSectionSize(150);
        LTaskList->header()->setMinimumSectionSize(0);

        verticalLayout->addWidget(LTaskList);


        retranslateUi(TaskList);

        QMetaObject::connectSlotsByName(TaskList);
    } // setupUi

    void retranslateUi(QWidget *TaskList)
    {
        TaskList->setWindowTitle(QApplication::translate("TaskList", "Form", 0));
        QTreeWidgetItem *___qtreewidgetitem = LTaskList->headerItem();
        ___qtreewidgetitem->setText(2, QApplication::translate("TaskList", "Progress Info", 0));
        ___qtreewidgetitem->setText(1, QApplication::translate("TaskList", "Name", 0));
        ___qtreewidgetitem->setText(0, QApplication::translate("TaskList", "Progress", 0));
    } // retranslateUi

};

namespace Ui {
    class TaskList: public Ui_TaskList {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TASKLIST_H
