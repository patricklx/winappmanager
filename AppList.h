#ifndef TABPAGE1WIDGET_H
#define TABPAGE1WIDGET_H

#include <QWidget>
#include <QTreeWidget>
#include <QDomElement>
#include <QTimer>
#include <QFile>

#include "appinfo.h"
#include "task.h"

class AppInfo;
class RegistryGroup;

namespace Ui {
    class AppList;
}

class AppList : public QWidget
{
    Q_OBJECT

public:
    explicit AppList(QWidget *parent = 0);
    ~AppList();

    typedef struct Fileinfo
    {
        QString name;
        AppInfo *appInfo;

        bool operator ==(struct Fileinfo a) const{
            return (a.name.compare(name,Qt::CaseInsensitive)==0);
        }

        bool operator <( struct Fileinfo a) const{
            return (a.name.compare(name,Qt::CaseInsensitive)<0);
        }
        Fileinfo():appInfo(NULL){}
    }Fileinfo;


    void setListByCategory(QString cat);
    void updateVersions();
    bool tasksRunning();


    void loadFiles();


signals:
    void taskChosen(Task *task);
    void unSelected(AppInfo *info);
    void versions_available();
    void status(QString text);

public slots:
    void onRemovedFromTasks(AppInfo *info);
    void updateItem(AppInfo *appinfo);
    void onVersionUpdated(AppInfo *appinfo,bool updated);
    bool load();

private slots:
    void on_TCategoryTree_itemSelectionChanged();
    void on_LAppInfoList_itemSelectionChanged();
    void on_LAppInfoList_customContextMenuRequested(const QPoint &pos);
    void on_btUpdateInfo_clicked();
    void on_btUpdate_clicked();
    void onButtonUpgradeClicked();
    void onButtonInstallClicked();
    void onButtonRemoveClicked();

    void on_lbSearch_textChanged(const QString &arg1);
    void on_btReload_clicked();

    void setRegistryInfo(RegistryGroup group);

    void on_btUpgrade_clicked();
    QTreeWidgetItem* getCurrentItem();
    AppInfo* getAppInfo(QTreeWidgetItem* item);

    void drawButtons();
    void timedDrawButtons();

    void on_startTasksBtn_clicked();

    void on_btnTask_clicked();
    void onSetAppInfoUpdated(QString name);

private:
    Ui::AppList *ui;
    bool searching;
    bool stop_searching;
    QList<Fileinfo> fileinfo_list;
    QList<Fileinfo> noinfo_list;
    bool version_updates_avail;
    QTimer timer;

    void appendToList(Fileinfo &fileinfo);
    void loadCategoryTree(QDomElement node,QTreeWidgetItem *item);
    void addCategory(QStringList categorylist);

    void clear();
    void prepareUi();
    QWidget *createActionButtons(QTreeWidgetItem *item);
    void upgrade(QTreeWidgetItem *item);
    void install(QTreeWidgetItem *item);
    void uninstall(QTreeWidgetItem *item);
    void deselect(QTreeWidgetItem *item);

};

#endif // TABPAGE1WIDGET_H
