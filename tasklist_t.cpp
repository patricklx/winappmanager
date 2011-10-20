#include <QProgressBar>
#include <QObject>
#include <QLabel>
#include <QMenu>
#include <QPointer>
#include <QUrl>
#include <QDesktopServices>
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
    silentdownload_count = 0;
    download_count = 0;
    isInstalling = false;
    winapp_manager = NULL;

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
    QTreeWidgetItem *item = NULL;
    task_t *t = NULL;
    for(int i=0;i<items.count();i++)
    {
        item = items[i];
        t = item->data(0,QTreeWidgetItem::UserType).value<task_t*>();;
        appinfo_t *item_info = t->m_appinfo;
        if(item_info==info)
            break;
    }

    if(!t->isRunning())
    {
        installList.removeOne(t);
        silentInstallList.removeOne(t);
        downloadSilentList.removeOne(t);
        downloadList.removeOne(t);
        emit onTaskRemoved(t->m_appinfo);
	if(t->m_appinfo->Name == "WinApp_Manager")
	{
	    winapp_manager = NULL;
	}
        t->deleteLater();
        delete item;
    }else
        QMessageBox::information(this,"Removing Task","Can't remove: the task is currently running!");
}

void tasklist_t::addTask(task_t *task)
{
    qDebug("added");
    QList<QTreeWidgetItem*> items = ui->LTaskList->findItems(task->m_appinfo->Name,Qt::MatchStartsWith,1);
    task_t *t = NULL;
    for(int i=0;i<items.count();i++)
    {
        QTreeWidgetItem *item = items[i];
        t = item->data(0,QTreeWidgetItem::UserType).value<task_t*>();;
        appinfo_t *item_info = t->m_appinfo;
        if(item_info==task->m_appinfo)
            return;
    }

    QProgressBar *pbar = new QProgressBar();
    pbar->setTextVisible(false);

    connect(task,SIGNAL(progress(task_t*,int,QString)),SLOT(updateProgressInfo(task_t*,int,QString)));
    if(task->isSet(task_t::DOWNLOAD))
        connect(task,SIGNAL(finished()),SLOT(onDownloadFinished()));
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
    item->setText(1,task->m_appinfo->Name+" " + task->m_inet_file.description);
    task->m_item = item;

    if(task->isSet(task_t::UNINSTALL))
        item->setText(2,"will be Uninstalled");
    if(task->isSet(task_t::INSTALL))
        item->setText(2,"will be installed");
    if(task->isSet(task_t::INSTALL) && task->m_appinfo->isFlagSet(appinfo_t::INSTALLED) && task->m_appinfo->isFlagSet(appinfo_t::NEEDS_UNINSTALL))
        item->setText(2,"will be uninstalled before upgrade");
    if(task->isSet(task_t::DOWNLOAD))
        item->setText(2,"will be downloaded");
    if(task->isSet(task_t::DOWNLOAD) && task->isSet(task_t::INSTALL))
        item->setText(2,"will be downloaded and installed");

    item->setData(0,QTreeWidgetItem::UserType,qVariantFromValue(task));

    ui->LTaskList->addTopLevelItem(item);
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
	if(task->m_appinfo->Name == "WinApp_Manager")
	{
	    winapp_manager = task;
	    item->setText(2,item->text(2)+"will be installed last");
	}else
	{
	    if(task->isSet(task_t::SILENT))
		silentInstallList.append(task);
	    else
		installList.append(task);
	}
    }
}

void tasklist_t::updateProgressInfo(task_t *t,int p,QString text)
{
    QTreeWidgetItem *item = t->m_item;

    item->setText(2,text);
    QProgressBar *pbar = (QProgressBar *) ui->LTaskList->itemWidget(item,0);
    if(pbar!=NULL)
    {
        if(p!=-1)
        {
            pbar->setMaximum(100);
            pbar->setValue(p);
        }
        else
        {
            pbar->reset();
            pbar->setMinimum(0);
            pbar->setMaximum(0);
        }
    }else
        qDebug("pbar is NULL!!");
}


void tasklist_t::onDownloadFinished()
{
    qDebug("download finished");
    task_t *t = (task_t*)sender();
    t->wait();

    t->disconnect();
    connect(t,SIGNAL(progress(task_t*,int,QString)),SLOT(updateProgressInfo(task_t*,int,QString)));

    if( t->isSet(task_t::SILENT) )
    {
        silentdownload_count--;
    }else
    {
        download_count--;
    }

    if( !t->isSet(task_t::DOWNLOAD) )
    {
        QTreeWidgetItem *item = t->m_item;
        if( t->isSet(task_t::INSTALL) )
        {
            if(t->m_appinfo->Name == "WinApp_Manager")
            {
                winapp_manager = t;
                item->setText(2,item->text(2)+"will be installed last");
            }else
            {
                if( t->isSet(task_t::SILENT) )
                {
                    silentInstallList.append(t);
                    qDebug("silent install");
                    connect(t,SIGNAL(finished()),SLOT(onSilentTaskFinished()));
                    item->setText(2,item->text(2)+" -> will be silently installed");
                }
                else
                {
                    installList.append(t);
                    connect(t,SIGNAL(finished()),SLOT(onTaskFinished()));
                    item->setText(2,item->text(2)+" -> will be installed");
                }
            }
        }

        t->m_appinfo->DlVersion = t->m_appinfo->LatestVersion;
        t->m_appinfo->downloaded_id = t->m_inet_file.id;
        t->m_appinfo->saveApplicationInfo();
    }


    on_commandLinkButton_clicked();
}

void tasklist_t::onTaskFinished()
{
    qDebug("task finished");
    task_t *t = (task_t*)sender();
    emit updateAppInfo(t->m_appinfo);
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
    for(int i=download_count+silentdownload_count;i<SettingsDialog::simulDownloadCount();i++)
    {
        if( ui->checkSilent->isChecked() )
        {
            if(downloadSilentList.count()>0)
            {
                task_t *t = downloadSilentList[0];
                downloadSilentList.removeAt(0);
                silentdownload_count++;
                t->start();
                continue;
            }

            if(downloadList.count()>0)
            {
                task_t *t = downloadList[0];
                downloadList.removeAt(0);
                download_count++;
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
                download_count++;
                t->start();
                continue;
            }

            if(downloadSilentList.count()>0)
            {
                task_t *t = downloadSilentList[0];
                downloadSilentList.removeAt(0);
                silentdownload_count++;
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
                t = silentInstallList.first();
                qDebug("start silent install");
                if(t->isRunning())
                    return;
                isInstalling = true;
                t->start();
                return;
            }else if( (installList.count() > 0) && (downloadSilentList.count()==0) && (silentdownload_count==0) )
            {
                t = installList.first();
                qDebug("start normal install, silent queue empty");
                if(t->isRunning())
                    return;
                isInstalling = true;
                t->start();
                return;
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
                t = installList.first();
                if(t->isRunning())
                    return;
                isInstalling = true;
                t->start();
                return;
            }else if(silentInstallList.count() > 0 && downloadList.count() == 0 && download_count==0)
            {
                t = silentInstallList.first();
                if(t->isRunning())
                    return;
                isInstalling = true;
                t->start();
                return;
            }
        }
    }

    if(download_count+silentdownload_count==0 && silentInstallList.count()+installList.count()==0)
    {
        if(winapp_manager!=NULL)
            winapp_manager->start();
    }
}


void tasklist_t::on_LTaskList_customContextMenuRequested(const QPoint &pos)
{
    QTreeWidgetItem *item = ui->LTaskList->itemAt(pos);
    if(item==NULL)
        return;

    task_t *task = item->data(0,QTreeWidgetItem::UserType).value<task_t*>();

    QMenu menu;
    menu.addAction("remove");

    if(task->isRunning() && task->current_task() == task_t::DOWNLOAD)
    {
        menu.addAction("stop download");
    }

    if(task->isRunning() && task->current_task() == task_t::INSTALL)
    {
	menu.addAction("start next installer");
    }

    if(!task->isRunning() && (task->isSet(task_t::DOWNLOAD) || task->isSet(task_t::INSTALL) || task->isSet(task_t::UNINSTALL)) )
    {
        menu.addAction("start");
    }

    if(!task->isRunning() && !task->isSet(task_t::DOWNLOAD))
    {
	menu.addAction("execute installer");
    }

    if(!task->isRunning() && task->isSet(task_t::INSTALL))
    {
        menu.addAction("force installed version to latest");
    }

    menu.addSeparator();
    menu.addAction("open containing folder");

    menu.move(ui->LTaskList->mapToGlobal(pos)+QPoint(0,25));
    QAction *action = menu.exec();
    if(action==NULL)
        return;

    if( action->text() == "remove" )
    {
        removeTask(task->m_appinfo);
    }

    if( action->text() == "stop download" )
    {
        if(task->current_task() == task_t::DOWNLOAD)
        {
            task->stop();
        }else
            QMessageBox::information(this,"stopping task","To late, the download has already finished :( ");
    }

    if( action->text() == "start" )
    {
        if(!task->isRunning())
        {
            if(task->isSet(task_t::DOWNLOAD))
            {
                if(task->isSet(task_t::SILENT))
                    silentdownload_count++;
                else
                    download_count++;
            }
            task->start();
        }else
            QMessageBox::information(this,"restarting task","The task is already running");
    }

    if( action->text() == "force installed version to latest" )
    {
        if( !task->m_appinfo->forceRegistryToLatestVersion() )
            updateProgressInfo(task,0,"failed to set latest version in registry, try again as Admin!");
        else
            updateProgressInfo(task,100,"successfully set to latest version");
    }

    if( action->text() == "open containing folder")
    {
        QDesktopServices::openUrl(tr("file:///%1").arg(task->m_appinfo->Path));
    }

    if( action->text() == "execute installer")
    {
	appinfo_t *info = task->m_appinfo;
	QDesktopServices::openUrl(tr("file:///%1").arg(info->Path+"/"+info->fileName));
	if(task->isSet(task_t::SILENT))
	{
    	    silentInstallList.removeOne(task);
	}else
	{
	    installList.removeOne(task);
	}
	updateProgressInfo(task,0,"removed from install queue");
    }

    if( action->text() == "start next installer")
    {
	if(task->isSet(task_t::SILENT))
	{
    	    silentInstallList.removeOne(task);
	}else
	{
	    installList.removeOne(task);
	}
	isInstalling = false;
	on_commandLinkButton_clicked();
    }

}


