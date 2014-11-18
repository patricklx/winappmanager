#include <QProgressBar>
#include <QObject>
#include <QLabel>
#include <QMenu>
#include <QPointer>
#include <QUrl>
#include <QDesktopServices>
#include <QMessageBox>
#include <QInputDialog>
#include "task.h"
#include "tasklist.h"
#include "gui/ui_tasklist.h"
#include "settingsdialog.h"

Q_DECLARE_METATYPE(Task*)


TaskList::TaskList(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::TaskList)
{
	ui->setupUi(this);
	silentdownload_count = 0;
	download_count = 0;
	isInstalling = false;
	winapp_manager = NULL;

	if(SettingsDialog::value("INSTALL_MODE").toInt() == SettingsDialog::Silent)
	{
		automaticFirst = true;
	}else
		automaticFirst = false;


}

TaskList::~TaskList()
{
	for(int i=0;i<ui->LTaskList->topLevelItemCount();i++)
	{
		Task *t = ui->LTaskList->topLevelItem(i)->data(0,QTreeWidgetItem::UserType).value<Task*>();
		if(t->currentTask()==Task::DOWNLOAD)
		{
			t->stop();
		}
	}
	for(int i=0;i<ui->LTaskList->topLevelItemCount();i++)
	{
		Task *t = ui->LTaskList->topLevelItem(i)->data(0,QTreeWidgetItem::UserType).value<Task*>();
		if(t->currentTask()==Task::DOWNLOAD)
		{
			t->wait();
		}
	}
	delete ui;
}

void TaskList::removeTask(AppInfo *info)
{
	qDebug("removed");
	QList<QTreeWidgetItem*> items = ui->LTaskList->findItems(info->name(),Qt::MatchContains,1);
	QTreeWidgetItem *item = NULL;
	Task *t = NULL;
	for(int i=0;i<items.count();i++)
	{
		item = items[i];
		t = item->data(0,QTreeWidgetItem::UserType).value<Task*>();
		AppInfo *item_info = t->appInfo();
		if(item_info==info)
			break;
	}

	if(t==NULL)
		return;

	if(!t->isRunning())
	{
		installList.removeOne(t);
		silentInstallList.removeOne(t);
		downloadSilentList.removeOne(t);
		downloadList.removeOne(t);
		emit onTaskRemoved(t->appInfo());
		if(t->appInfo()->name() == "WinApp_Manager")
		{
			winapp_manager = NULL;
		}
		t->disconnect();
		t->deleteLater();
		ui->LTaskList->removeItemWidget(item,1);
		delete item;
	}else
	{
		QMessageBox::information(this,"Removing Task","Can't remove: the task is currently running!");
	}
}

void TaskList::addTask(Task *task)
{
	qDebug("added");
	QList<QTreeWidgetItem*> items = ui->LTaskList->findItems(task->appInfo()->name(),Qt::MatchStartsWith,1);
	Task *t = NULL;
	for(int i=0;i<items.count();i++)
	{
		QTreeWidgetItem *item = items[i];
		t = item->data(0,QTreeWidgetItem::UserType).value<Task*>();;
		AppInfo *item_info = t->appInfo();
		if(item_info==task->appInfo())
			return;
	}

	QProgressBar *pbar = new QProgressBar();
	pbar->setTextVisible(false);

	connect(task,SIGNAL(progress(Task*,int,QString)),SLOT(updateProgressInfo(Task*,int,QString)));
	if(task->isSet(Task::DOWNLOAD))
		connect(task,SIGNAL(finished()),SLOT(onDownloadFinished()));
	else
	{
		if(task->isSet(Task::INSTALL) || task->isSet(Task::UNINSTALL))
		{
			if(task->isSet(Task::SILENT))
				connect(task,SIGNAL(finished()),SLOT(onSilentTaskFinished()));
			else
				connect(task,SIGNAL(finished()),SLOT(onTaskFinished()));
			connect(task,SIGNAL(askUserToExecute(QStringList,int*)),SLOT(userChooseExecutable(QStringList,int*)),Qt::BlockingQueuedConnection);
		}
	}

	QTreeWidgetItem *item = new QTreeWidgetItem();
	QString description = task->fileLocation().description;
	if(description.isEmpty())
		 item->setText(1,task->appInfo()->name());
	else
		 item->setText(1,task->appInfo()->name()+" - " + description);

	task->setItem(item);

	if(task->isSet(Task::UNINSTALL))
		item->setText(2,"will be Uninstalled");
	if(task->isSet(Task::INSTALL))
		item->setText(2,"will be installed");
	if(task->isSet(Task::INSTALL) && task->appInfo()->isFlagSet(AppInfo::Installed) && task->appInfo()->getInstallInfo().needsUninstall)
		item->setText(2,"will be uninstalled before upgrade");
	if(task->isSet(Task::DOWNLOAD))
		item->setText(2,"will be downloaded");
	if(task->isSet(Task::DOWNLOAD) && task->isSet(Task::INSTALL))
		item->setText(2,"will be downloaded and installed");

	item->setData(0,QTreeWidgetItem::UserType,qVariantFromValue(task));

	ui->LTaskList->addTopLevelItem(item);
	ui->LTaskList->setColumnWidth(1,ui->LTaskList->columnWidth(1)+20);
	ui->LTaskList->setItemWidget(item,0,pbar);

	if(task->isSet(Task::DOWNLOAD))
	{
		if(task->isSet(Task::SILENT))
			downloadSilentList.append(task);
		else
			downloadList.append(task);
	}else
	{
		if(task->appInfo()->name() == "WinApp_Manager")
		{
			winapp_manager = task;
			item->setText(2,item->text(2)+"will be installed last");
		}else
		{
			if(task->isSet(Task::SILENT))
				silentInstallList.append(task);
			else
				installList.append(task);
		}
	}
}

void TaskList::updateProgressInfo(Task *t,int p,QString text)
{
	QTreeWidgetItem *item = t->item();

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


void TaskList::onDownloadFinished()
{
	qDebug("download finished");
	Task *t = (Task*)sender();
	t->wait();
	t->disconnect();
	connect(t,SIGNAL(askUserToExecute(QStringList,int*)),SLOT(userChooseExecutable(QStringList,int*)),Qt::BlockingQueuedConnection);

	emit updateAppInfo(t->appInfo());

	connect(t,SIGNAL(progress(Task*,int,QString)),SLOT(updateProgressInfo(Task*,int,QString)));

	if( t->isSet(Task::SILENT) )
	{
		silentdownload_count--;
	}else
	{
		download_count--;
	}

	if( !t->isSet(Task::DOWNLOAD) )
	{
		QTreeWidgetItem *item = t->item();
		if( t->isSet(Task::INSTALL) )
		{
			if(t->appInfo()->name() == "WinApp_Manager")
			{
				winapp_manager = t;
				item->setText(2,item->text(2)+"will be installed last");
			}else
			{
				if( t->isSet(Task::SILENT) )
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

		t->appInfo()->updateDownloadVersion();
		t->appInfo()->saveApplicationInfo();
	}


	startTasks();
}

void TaskList::onTaskFinished()
{
	qDebug("task finished");
	Task *t = (Task*)sender();
	emit updateAppInfo(t->appInfo());
	installList.removeOne(t);
	isInstalling = false;
	startTasks();
}

void TaskList::onSilentTaskFinished()
{
	qDebug("silent task finished");
	Task *t = (Task*)sender();
	emit updateAppInfo(t->appInfo());
	silentInstallList.removeOne(t);
	isInstalling = false;
	startTasks();
}

void TaskList::startTasks()
{
	int maxDownloads = SettingsDialog::value("DOWNLOAD_COUNT").toInt();
	for(int i=download_count+silentdownload_count;i<maxDownloads;i++)
	{
		if( this->automaticFirst )
		{
			if(downloadSilentList.count()>0)
			{
				Task *t = downloadSilentList[0];
				downloadSilentList.removeAt(0);
				silentdownload_count++;
				t->start();
				continue;
			}

			if(downloadList.count()>0)
			{
				Task *t = downloadList[0];
				downloadList.removeAt(0);
				download_count++;
				t->start();
				continue;
			}
		}

		if( !this->automaticFirst )
		{
			if(downloadList.count()>0)
			{
				Task *t = downloadList[0];
				downloadList.removeAt(0);
				download_count++;
				t->start();
				continue;
			}

			if(downloadSilentList.count()>0)
			{
				Task *t = downloadSilentList[0];
				downloadSilentList.removeAt(0);
				silentdownload_count++;
				t->start();
				continue;
			}
		}
	}

	if( !isInstalling )
	{
		if( this->automaticFirst )
		{
			Task *t;
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
		if( !this->automaticFirst )
		{
			Task *t;
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


void TaskList::on_LTaskList_customContextMenuRequested(const QPoint &pos)
{
	QList<QTreeWidgetItem*> list = ui->LTaskList->selectedItems();

	QTreeWidgetItem *item=NULL;
	if(list.isEmpty())
	{
		item = ui->LTaskList->itemAt(pos);
		if(item==NULL)
			return;
	}
	if(list.count()==1)
	{
		item = list[0];
	}


	QMenu menu;
	menu.addAction("remove");

	if(item!=NULL)
	{
		Task *task = item->data(0,QTreeWidgetItem::UserType).value<Task*>();


		if(task->isRunning() && task->currentTask() == Task::DOWNLOAD)
		{
			menu.addAction("stop download");
		}

		if(task->isRunning() && (task->currentTask() == Task::INSTALL || task->currentTask() == Task::UNINSTALL))
		{
			menu.addAction("detach process");
		}

		if(!task->isRunning() && (task->isSet(Task::DOWNLOAD) || task->isSet(Task::INSTALL) || task->isSet(Task::UNINSTALL)) )
		{
			menu.addAction("start");
		}

		if(!task->isRunning() && !task->isSet(Task::DOWNLOAD))
		{
			if(task->appInfo()->isFlagSet(AppInfo::NoRegistry))
				menu.addAction("execute program");
			else
				menu.addAction("execute installed");
		}

		if(!task->isRunning() && task->isSet(Task::INSTALL))
		{
			menu.addAction("Set to latest version");
		}

		menu.addSeparator();
		menu.addAction("open containing folder");
	}
	menu.move(ui->LTaskList->mapToGlobal(pos)+QPoint(0,25));
	QAction *action = menu.exec();
	if(action==NULL)
		return;

	if( action->text() == "remove" )
	{
		Task *task = NULL;
		if(item!=NULL)
		{
			task = item->data(0,QTreeWidgetItem::UserType).value<Task*>();
			removeTask(task->appInfo());
		}else
		{
			for(int i=0;i<list.count();i++)
			{
				item = list[i];
				task = item->data(0,QTreeWidgetItem::UserType).value<Task*>();
				removeTask(task->appInfo());
			}
		}
		return;
	}

	Task *task = item->data(0,QTreeWidgetItem::UserType).value<Task*>();

	if( action->text() == "stop download" )
	{
		if(task->currentTask() == Task::DOWNLOAD)
		{
			task->stop();
		}else
			QMessageBox::information(this,"stopping task","To late, the download has already finished :( ");
	}

	if( action->text() == "start" )
	{
		if(!task->isRunning())
		{
			if(task->isSet(Task::DOWNLOAD))
			{
				if(task->isSet(Task::SILENT))
					silentdownload_count++;
				else
					download_count++;
			}
			task->start();
		}else
			QMessageBox::information(this,"restarting task","The task is already running");
	}

	if( action->text() == "Set to latest version" )
	{
		if( !task->appInfo()->forceRegistryToLatestVersion() )
			updateProgressInfo(task,0,"failed to set latest version in registry, try again as Admin!");
		else
			updateProgressInfo(task,100,"successfully set to latest version");
	}

	if( action->text() == "open containing folder")
	{
		QDesktopServices::openUrl(tr("file:///%1").arg(task->appInfo()->path()));
	}

	if( action->text() == "execute installer" || action->text() == "execute program" )
	{
		AppInfo *info = task->appInfo();
		QDesktopServices::openUrl(tr("file:///%1").arg(info->path()+"/"+info->fileName()));
		if(task->isSet(Task::SILENT))
		{
			silentInstallList.removeOne(task);
		}else
		{
			installList.removeOne(task);
		}
		updateProgressInfo(task,0,"removed from install queue");
	}

	if( action->text() == "detach process")
	{
		task->stop();
	}

}



bool TaskList::isEmpty()
{
	for(int i=0; i < ui->LTaskList->topLevelItemCount();i++)
	{
		QTreeWidgetItem *item = ui->LTaskList->topLevelItem(i);
		Task *t = item->data(0,QTreeWidgetItem::UserType).value<Task*>();
		if(t->isRunning())
			return false;
	}
	while(ui->LTaskList->topLevelItemCount()>0)
	{
		QTreeWidgetItem *item = ui->LTaskList->topLevelItem(0);
		Task *t = item->data(0,QTreeWidgetItem::UserType).value<Task*>();
		removeTask(t->appInfo());
	}
	return true;
}

void TaskList::setAutomaticFirst(bool automatic)
{
	this->automaticFirst = automatic;
}


void TaskList::userChooseExecutable(QStringList list, int *answer)
{
	bool ok;
	QInputDialog dialog(this);
	dialog.setOption(QInputDialog::UseListViewForComboBoxItems,true);
	dialog.setWindowTitle("Choose executable");
	dialog.setLabelText("We were unable to determine the right executable to install the program.\nPlease choose one executable or cancel if it's not installable");
	dialog.setComboBoxItems(list);
	dialog.exec();
	ok = dialog.result();
	QString text = dialog.textValue();


	if(ok && !text.isEmpty())
	{
		*answer = list.indexOf(text);
	}else
		*answer = -1;
}


