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

        bool operator ==(struct fileinfo_t a) const{
            return (a.name == name);
        }

        bool operator <( struct fileinfo_t a) const{
            return (a.name < name);
        }
        fileinfo_t():info(NULL){}
    }fileinfo_t;
    void setListByCategory(QString cat);

    bool version_updates_avail;

signals:
    void taskChosen(task_t *task);
    void unSelected(appinfo_t *info);
    void versions_available();
    void status(QString text);
    void reload_clicked();

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

    void setRegistryInfo(registry_group_t group);

    void on_btReload_clicked();

public slots:
    void onRemovedFromTasks(appinfo_t *info);
    void updateItem(appinfo_t *appinfo);
    void onVersionUpdated(appinfo_t *appinfo,bool updated);
    void on_btUpdate_clicked();
    bool loadList();

private:
    Ui::applist_t *ui;
    bool searching;
    bool stop_searching;
    QList<fileinfo_t> fileinfo_list;
    QList<fileinfo_t> noinfo_list;
    bool info_updates_avail;
    void appendToList(fileinfo_t &fileinfo);
    void saveList();
    void loadCategoryTree(QDomElement node,QTreeWidgetItem *item);
    void addCategory(QStringList categorylist);

    void clear();

};

#endif // TABPAGE1WIDGET_H
