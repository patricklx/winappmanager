#include <QProgressBar>
#include <QObject>
#include <QLabel>
#include <QMenu>
#include <QPointer>
#include <QMessageBox>
#include "task_t.h"
#include "tasklist_t.h"
#include "ui_tasklist_t.h"
#include "settingsdialog.h"

Q_DECLARE_METATYPE(task_t*)


tasklist_t::tasklist_t(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tasklist_t)
{
    ui->setupUi(this);
    dl_inProcess_count = 0;
    isInstalling = false;

    if(SettingsDialog::getInstallTaskMode() == SettingsDialog::SILENT)
    {
        ui->checkSilent->setChecked(true);
    }else
        ui->checkAttendet->setChecked(true);
}

tasklist_t::~tasklist_t()
{
    for(int i=0;i<ui->LTaskList->topLevelItemCount();i++)
    {
        task_t *t = ui->LTaskList->topLevelItem(i)->data(0,QTreeWidgetItem::UserType).value<task_t*>();
        if(t->current_task()==task_t::DOWNLOAD)
        {
            t->stop();
        }
    }
    for(int i=0;i<ui->LTaskList->topLevelItemCount();i++)
    {
        task_t *t = ui->LTaskList->topLevelItem(i)->data(0,QTreeWidgetItem::UserType).value<task_t*>();
        if(t->current_task()==task_t::DOWNLOAD)
        {
            t->wait();
        }
    }
    delete ui;
}

void tasklist_t::removeTask(appinfo_t *info)
{
    qDebug("removed");
    QList<QTreeWidgetItem*> items = ui->LTaskList->findItems(info->Name,Qt::MatchContains,1);
    if(items.count()==0)
        return;

    QTreeWidgetItem *item = items[0];
    task_t *t = item->data(0,QTreeWidgetItem::UserType).value<task_t*>();
    if(!t->isRunning())
    {
        installList.removeOne(t);
        silentInstallList.removeOne(t);
        downloadSilentList.removeOne(t);
        downloadList.removeOne(t);
        emit onTaskRemoved(t->m_appinfo);
        t->deleteLater();
        delete item;
    }else
        QMessageBox::information(this,tr("Removing Task"),tr("Can't remove: the task is currently running!"));
}

void tasklist_t::addTask(task_t *task)
{
    qDebug("added");
    QList<QTreeWidgetItem*> items = ui->LTaskList->findItems(task->m_appinfo->Name,Qt::MatchStartsWith,1);
    if(items.count()!=0)
        return;

    QProgressBar *pbar = new QProgressBar();
    pbar->setTextVisible(false);

    connect(task,SIGNAL(progress(task_t*,int,QString)),SLOT(updateProgressInfo(task_t*,int,QString)));
    if(task->isSet(task_t::DOWNLOAD))
        connect(task,SIGNAL(finish_success()),SLOT(onDownloadFinished()));
    else
    {
        if(task->isSet(task_t::INSTALL) || task->isSet(task_t::UNINSTALL))
        {
            if(task->isSet(task_t::SILENT))
                connect(task,SIGNAL(finished()),SLOT(onSilentTaskFinished()));
            else
                connect(task,SIGNAL(finished()),SLOT(onTaskFinished()));
        }
    }

    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(1,task->m_appinfo->Name+tr(" ") + task->m_inet_file.description);
    task->m_item = item;

    if(task->isSet(task_t::INSTALL))
        item->setText(2,tr("will be installed"));
    if(task->isSet(task_t::DOWNLOAD))
        item->setText(2,tr("will be downloaded"));
    if(task->isSet(task_t::UNINSTALL))
        item->setText(2,tr("will be Uninstalled"));
    if(task->isSet(task_t::DOWNLOAD) && task->isSet(task_t::INSTALL))
        item->setText(2,tr("will be downloaded and installed"));

    item->setData(0,QTreeWidgetItem::UserType,qVariantFromValue(task));

    ui->LTaskList->addTopLevelItem(item);
    ui->LTaskList->resizeColumnToContents(1);
    ui->LTaskList->setColumnWidth(1,ui->LTaskList->columnWidth(1)+20);
    ui->LTaskList->setItemWidget(item,0,pbar);

    if(task->isSet(task_t::DOWNLOAD))
    {
        if(task->isSet(task_t::SILENT))
            downloadSilentList.append(task);
        else
            downloadList.append(task);
    }else
    {
        if(task->isSet(task_t::SILENT))
            silentInstallList.append(task);
        else
            installList.append(task);
    }
}

void tasklist_t::updateProgressInfo(task_t *t,int p,QString text)
{
    QTreeWidgetItem *item = t->m_item;

    item->setText(2,text);
    QProgressBar *pbar = (QProgressBar *) ui->LTaskList->itemWidget(item,0);

    if(p!=-1)
    {
        pbar->setMaximum(100);
        pbar->setValue(p);
    }
    else
        pbar->setMaximum(0);

    ui->LTaskList->resizeColumnToContents(1);
}


void tasklist_t::onDownloadFinished()
{
    qDebug("download finished successfull");
    task_t *t = (task_t*)sender();

    disconnect(t,SIGNAL(finished()),this,SLOT(onDownloadFinished()));
    dl_inProcess_count--;
    if( t->isSet(task_t::INSTALL) )
    {
        QTreeWidgetItem *item = t->m_item;
        if( t->isSet(task_t::SILENT) )
        {
            silentInstallList.append(t);
            connect(t,SIGNAL(finished()),SLOT(onSilentTaskFinished()));
            item->setText(2,item->text(2)+tr(" -> will be silently installed"));
        }
        else
        {
            installList.append(t);
            connect(t,SIGNAL(finished()),SLOT(onTaskFinished()));
            item->setText(2,item->text(2)+tr(" -> will be installed"));
        }
    }

    t->m_appinfo->DlVersion = t->m_appinfo->LatestVersion;
    t->m_appinfo->downloaded_id = t->m_inet_file.id;
    t->m_appinfo->saveApplicationInfo();
    t->wait();

    on_commandLinkButton_clicked();
}

void tasklist_t::onTaskFinished()
{
    qDebug("task finished");
    task_t *t = (task_t*)sender();
    installList.removeOne(t);
    isInstalling = false;
    on_commandLinkButton_clicked();
}

void tasklist_t::onSilentTaskFinished()
{
    qDebug("silent task finished");
    task_t *t = (task_t*)sender();

    silentInstallList.removeOne(t);
    isInstalling = false;
    on_commandLinkButton_clicked();
}

void tasklist_t::on_commandLinkButton_clicked()
{
    for(int i=dl_inProcess_count;i<SettingsDialog::simulDownloadCount();i++)
    {
        if( ui->checkSilent->isChecked() )
        {
            if(downloadSilentList.count()>0)
            {
                task_t *t = downloadSilentList[0];
                downloadSilentList.removeAt(0);
                dl_inProcess_count++;
                t->start();
                continue;
            }
            if(downloadList.count()>0)
            {
                task_t *t = downloadList[0];
                downloadList.removeAt(0);
                dl_inProcess_count++;
                t->start();
                continue;
            }
        }

        if( ui->checkAttendet->isChecked() )
        {
            if(downloadList.count()>0)
            {
                task_t *t = downloadList[0];
                downloadList.removeAt(0);
                dl_inProcess_count++;
                t->start();
                continue;
            }

            if(downloadSilentList.count()>0)
            {
                task_t *t = downloadSilentList[0];
                downloadSilentList.removeAt(0);
                dl_inProcess_count++;
                t->start();
                continue;
            }
        }
    }

    if( !isInstalling )
    {
        if( ui->checkSilent->isChecked() )
        {
            task_t *t;
            if( silentInstallList.count() > 0)
            {
                t = silentInstallList[0];
                if(t->isRunning())
                    return;
                isInstalling = true;
                t->start();
            }else if(installList.count() > 0 && downloadSilentList.count()==0)
            {
                t = installList[0];
                if(t->isRunning())
                    return;
                isInstalling = true;
                t->start();
            }
        }
    }

    if( !isInstalling )
    {
        if( ui ->checkAttendet->isChecked() )
        {
            task_t *t;
            if( installList.count() > 0)
            {
                t = installList[0];
                if(t->isRunning())
                    return;
                isInstalling = true;
                t->start();
            }else if(silentInstallList.count() > 0 && downloadList.count() == 0)
            {
                t = silentInstallList[0];
                if(t->isRunning())
                    return;
                isInstalling = true;
                t->start();
            }
        }
    }
}


void tasklist_t::on_LTaskList_customContextMenuRequested(const QPoint &pos)
{
    QTreeWidgetItem *item = ui->LTaskList->itemAt(pos);
    if(item==NULL)
        return;

    task_t *task = item->data(0,QTreeWidgetItem::UserType).value<task_t*>();

    QMenu menu;
    menu.addAction(tr("remove"));

    if(task->isRunning() && task->current_task() == task_t::DOWNLOAD)
    {
        menu.addAction(tr("stop download"));
    }
    if(!task->isRunning() && (task->isSet(task_t::DOWNLOAD) || task->isSet(task_t::INSTALL) || task->isSet(task_t::UNINSTALL)) )
    {
        menu.addAction(tr("start"));
    }
    if(!task->isRunning() && task->isSet(task_t::INSTALL))
    {
        menu.addAction(tr("force installed version to latest"));
    }

    menu.move(ui->LTaskList->mapToGlobal(pos)+QPoint(0,25));
    QAction *action = menu.exec();
    if(action==NULL)
        return;

    if( action->text() == tr("remove") )
    {
        removeTask(task->m_appinfo);
    }

    if( action->text() == tr("stop download") )
    {
        if(task->current_task() == task_t::DOWNLOAD)
        {
            task->stop();
        }else
            QMessageBox::information(this,tr("stopping task"),tr("To late, the download has already ended :( "));
    }

    if( action->text() == tr("start") )
    {
        if(!task->isRunning())
        {
            task->start();
            if( task->isSet(task_t::DOWNLOAD) )
            {
                downloadSilentList.removeOne(task);
                downloadList.removeOne(task);
            }
            if(task->isSet(task_t::INSTALL) || task->isSet(task_t::UNINSTALL))
            {
                installList.removeOne(task);
                silentInstallList.removeOne(task);
            }
        }else
            QMessageBox::information(this,tr("restarting task"),tr("The task is already running"));
    }

    if( action->text() == tr("force installed version to latest") )
    {
        if( !task->m_appinfo->forceRegistryToLatestVersion() )
            updateProgressInfo(task,0,tr("failed to set latest version in registry, try again as Admin!"));
        else
            updateProgressInfo(task,100,tr("successfully set to latest version"));
    }
}




