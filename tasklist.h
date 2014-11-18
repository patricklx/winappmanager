#ifndef TABPAGE2WIDGET_H
#define TABPAGE2WIDGET_H

#include <QWidget>
#include <QTreeWidgetItem>
#include "task.h"

namespace Ui {
    class TaskList;
}

class TaskList : public QWidget
{
    Q_OBJECT

public:
    TaskList(QWidget *parent = 0);
    ~TaskList();

    bool isEmpty();
    void setAutomaticFirst(bool automatic);
    void startTasks();


signals:
    void onTaskRemoved(AppInfo *info);
    void updateAppInfo(AppInfo *info);

public slots:
    void addTask(Task *task);
    void removeTask(AppInfo *info);

private slots:

    void on_LTaskList_customContextMenuRequested(const QPoint &pos);
    void updateProgressInfo(Task *t, int p,QString text);

    void onTaskFinished();
    void onSilentTaskFinished();
    void onDownloadFinished();
    void userChooseExecutable(QStringList list,int* answer);

private:
    Ui::TaskList *ui;
    QList<Task*> installList;
    QList<Task*> silentInstallList;

    QList<Task*> downloadList;
    QList<Task*> downloadSilentList;

    Task* winapp_manager;
    bool automaticFirst;



    int download_count;
    int silentdownload_count;
    bool isInstalling;
};

#endif // TABPAGE2WIDGET_H

