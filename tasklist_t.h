#ifndef TABPAGE2WIDGET_H
#define TABPAGE2WIDGET_H

#include <QWidget>
#include <QTreeWidgetItem>
#include "task_t.h"

namespace Ui {
    class tasklist_t;
}

class tasklist_t : public QWidget
{
    Q_OBJECT

public:
    tasklist_t(QWidget *parent = 0);
    ~tasklist_t();

    bool isEmpty();

signals:
    void onTaskRemoved(appinfo_t *info);
    void updateAppInfo(appinfo_t *info);

public slots:
    void addTask(task_t *task);
    void removeTask(appinfo_t *info);

private slots:
    void on_commandLinkButton_clicked();
    void on_LTaskList_customContextMenuRequested(const QPoint &pos);
    void updateProgressInfo(task_t *t, int p,QString text);

    void onTaskFinished();
    void onSilentTaskFinished();
    void onDownloadFinished();

private:
    Ui::tasklist_t *ui;
    QList<task_t*> installList;
    QList<task_t*> silentInstallList;

    QList<task_t*> downloadList;
    QList<task_t*> downloadSilentList;

    task_t* winapp_manager;


    int download_count;
    int silentdownload_count;
    bool isInstalling;
};

#endif // TABPAGE2WIDGET_H

