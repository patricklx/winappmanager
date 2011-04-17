#ifndef TASK_T_H
#define TASK_T_H

#include <QProgressBar>
#include <QLabel>
#include <QTreeWidgetItem>
#include <QThread>

#include "appinfo_t.h"
class task_t : public QThread
{
    Q_OBJECT
public:
    enum task_flags{
        INSTALL   =0x1,
        UNINSTALL =0x2,
        SILENT    =0x4,
        DOWNLOAD  =0x8
    };

    task_t(appinfo_t *appinfo, int flags);
    task_t(appinfo_t *appinfo,appinfo_t::inet_file_t inet_file,int flags);
    task_t():QThread(),m_appinfo(NULL),should_quit(false){}
    ~task_t(){qDebug("deleted task");}

    appinfo_t::inet_file_t m_inet_file;

    int m_flags;
    int m_current_task;
    QTreeWidgetItem *m_item;

    appinfo_t *m_appinfo;
    bool isSet(task_flags flag){return ( (m_flags & flag) == flag);}
    void unSet(task_flags flag){m_flags &= ~flag;}
    void set(task_flags flag){m_flags |= flag;}

    void run();
    int current_task(){return m_current_task;}

    void stop(){should_quit = true;}

private:
    bool should_quit;

    bool testDestroy(){return should_quit;}
    void waitForExternalProcess(QString PrcName);
    bool download();
    bool install();
    bool uninstall();
signals:
    void progress(task_t *t,int p, QString info);
    void finish_success();
};

#endif // TASK_T_H
