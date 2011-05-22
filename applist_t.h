#ifndef TABPAGE1WIDGET_H
#define TABPAGE1WIDGET_H

#include <QWidget>
#include <QTreeWidget>
#include <appinfo_t.h>
#include <QDomElement>
#include "task_t.h"

namespace Ui {
    class applist_t;
}

class applist_t : public QWidget
{
    Q_OBJECT

public:
    explicit applist_t(QWidget *parent = 0);
    ~applist_t();

    typedef struct fileinfo_t
    {
        QString lastUpdate;
        QString name;
        appinfo_t *info;

        bool operator ==(struct fileinfo_t a){
            return (a.name == name);
        }
        fileinfo_t():info(NULL){}
    }fileinfo_t;
    void setListByCategory(QString cat);

    bool version_updates_avail;

signals:
    void taskChosen(task_t *task);
    void unSelected(appinfo_t *info);
    void versions_available();

private slots:
    void on_TCategoryTree_itemSelectionChanged();
    void on_LAppInfoList_itemSelectionChanged();
    void on_LAppInfoList_customContextMenuRequested(const QPoint &pos);
    void on_LAppInfoList_itemActivated(QTreeWidgetItem *item, int column);

    void on_btCheckUpdates_clicked();
    void on_btUpdateInfo_clicked();
private slots:
    void onNewAppInfoAvailable();
    void onCheckForNewAppInfo();
    void onSetAppInfoUpdated(QString name, QString lastUpdate);

    void on_lbSearch_textChanged(const QString &arg1);
    bool loadList();

public slots:
    void onRemovedFromTasks(appinfo_t *info);
    void updateItem(appinfo_t *appinfo);
    void onVersionUpdated(appinfo_t *appinfo,bool updated);
    void on_btUpdate_clicked();

private:
    Ui::applist_t *ui;
    QList<fileinfo_t> fileinfo_list;
    bool info_updates_avail;
    void appendToList(fileinfo_t &fileinfo);
    void saveList();
    void loadCategoryTree(QDomElement node,QTreeWidgetItem *item);

    void clear();

};

#endif // TABPAGE1WIDGET_H
