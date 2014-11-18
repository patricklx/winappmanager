#ifndef TASK_T_H
#define TASK_T_H

#include <QProgressBar>
#include <QLabel>
#include <QTreeWidgetItem>
#include <QThread>
#include "appinfo.h"

class Task : public QThread
{
    Q_OBJECT
public:
    enum task_flags{
        INSTALL   =0x1,
        UNINSTALL =0x2,
        SILENT    =0x4,
        DOWNLOAD  =0x8
    };

    Task(AppInfo *appinfo, int flags);
    Task(AppInfo *appinfo,AppInfo::UrlContainer fileLocation,int flags);
    Task():QThread(),should_quit(false),m_appinfo(NULL){}


    ~Task(){qDebug("deleted task");}




    bool isSet(task_flags flag){return ( (m_flags & flag) == flag);}
    void unSet(task_flags flag){m_flags &= ~flag;}
    void set(task_flags flag){m_flags |= flag;}

    void run();
    int currentTask(){return m_current_task;}

    void stop(){should_quit = true;}

    bool followProcessChilds(int pid);

    AppInfo *appInfo();
    AppInfo::UrlContainer fileLocation();
    QTreeWidgetItem* item();
    void setItem(QTreeWidgetItem *item);

private:
    bool should_quit;
    int m_flags;
    int m_current_task;
    QTreeWidgetItem *m_item;
    AppInfo *m_appinfo;
    AppInfo::UrlContainer m_fileLocation;

    bool testDestroy(){return should_quit;}

    bool download();
    bool install();
    bool uninstall();
    bool unzip();

signals:
    void progress(Task *t,int p, QString info);
    void askUserToExecute(QStringList list,int *answer);
    void askUserUnzipPath(QString &path);
};

#endif // TASK_T_H
