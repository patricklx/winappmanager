#include <QMessageBox>
#include <QMenu>
#include <QFile>
#include <QDesktopServices>
#include <QTimer>
#include <QDomDocument>
#include <settingsdialog.h>
#include "applist_t.h"
#include "ui_applist_t.h"
#include "choosedialog.h"
#include "updaterdialog.h"

Q_DECLARE_METATYPE(applist_t::fileinfo_t)

class TreeWidgetItem : public QTreeWidgetItem
{
public:
    TreeWidgetItem(QStringList &list):QTreeWidgetItem(list){}
    virtual bool operator<(const QTreeWidgetItem &other) const
    {
        if(this->treeWidget()->sortColumn()==0)
            return text( 0 ).toLower() < other.text( 0 ).toLower();
        else
            return QTreeWidgetItem::operator <(other);
    }
};

applist_t::applist_t(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::applist_t)
{
    ui->setupUi(this);
    info_updates_avail = false;
    version_updates_avail = false;
    ui->LAppInfoList->sortItems(0,Qt::AscendingOrder);



    QTreeWidgetItem *item = new QTreeWidgetItem(QStringList(tr("All")));
    ui->TCategoryTree->addTopLevelItem(item);

    loadList();

    item = new QTreeWidgetItem(QStringList(tr("Updates")));
    ui->TCategoryTree->addTopLevelItem(item);
    item = new QTreeWidgetItem(QStringList(tr("Installed")));
    ui->TCategoryTree->addTopLevelItem(item);
    item = new QTreeWidgetItem(QStringList(tr("Downloaded")));
    ui->TCategoryTree->addTopLevelItem(item);
    item = new QTreeWidgetItem(QStringList(tr("New/Updated")));
    ui->TCategoryTree->addTopLevelItem(item);
    item = new QTreeWidgetItem(QStringList(tr("Only Installed")));
    ui->TCategoryTree->addTopLevelItem(item);
    item = new QTreeWidgetItem(QStringList(tr("Only Downloaded")));
    ui->TCategoryTree->addTopLevelItem(item);
    item = new QTreeWidgetItem(QStringList(tr("Only Info")));
    ui->TCategoryTree->addTopLevelItem(item);

    ui->TCategoryTree->setCurrentItem(ui->TCategoryTree->topLevelItem(0),0,QItemSelectionModel::Select);

    if( (SettingsDialog::shouldCheckAppInfo() && SettingsDialog::lastInfoUpdate() < QDate::currentDate())
            || fileinfo_list.isEmpty())
    {
        QTimer::singleShot(1000,this,SLOT(onCheckForNewAppInfo()));
        SettingsDialog::setLastInfoUpdate(QDate::currentDate());
    }

    if(SettingsDialog::shouldCheckVersions() && SettingsDialog::lastVersionUpdate() < QDate::currentDate()
            && !fileinfo_list.isEmpty())
    {
        QTimer::singleShot(1000,this,SLOT(on_btUpdate_clicked()));
        SettingsDialog::setLastVersionUpdate(QDate::currentDate());
    }
}

applist_t::~applist_t()
{
    clear();
    delete ui;
}

void applist_t::on_TCategoryTree_itemSelectionChanged()
{
    QTreeWidgetItem *item = ui->TCategoryTree->currentItem();
    ui->LAppInfoList->clear();
    setListByCategory(item->text(0));
}

void applist_t::on_LAppInfoList_itemSelectionChanged()
{
    QTreeWidgetItem *item = ui->LAppInfoList->currentItem();
    applist_t::fileinfo_t info = item->data(0,QTreeWidgetItem::UserType).value<applist_t::fileinfo_t>();
    ui->AppInfoText->clear();
    ui->AppInfoText->moveCursor(QTextCursor::End);/*workaround for QTBUG-539*/

    ui->AppInfoText->append(info.name);
    ui->AppInfoText->append(tr("<a href='")+info.info->WebPage+tr("'>")+info.info->WebPage+tr("</a>"));

    ui->AppInfoText->append(info.info->Description+tr("\n"));
    ui->AppInfoText->append(info.info->Info);

    ui->AppInfoText->moveCursor(QTextCursor::Start);

}

void applist_t::on_LAppInfoList_customContextMenuRequested(const QPoint &pos)
{
    QTreeWidgetItem *item = ui->LAppInfoList->itemAt(pos);
    if(item==NULL)
        return;

    applist_t::fileinfo_t data = item->data(0,QTreeWidgetItem::UserType).value<applist_t::fileinfo_t>();
    appinfo_t *info = data.info;

    QMenu menu;
    if(info->isFlagSet(appinfo_t::SELECTED_INST_DL) || info->isFlagSet(appinfo_t::SELECTED_REM))
        menu.addAction(tr("Unselect"));
    else
        menu.addAction(tr("Select"));
    menu.addAction(tr("Check Latest Version"));
    menu.addSeparator();

    if( info->isFlagSet(appinfo_t::UPDATE_AVAIL) )
        menu.addAction(tr("Update"));
    if( info->isFlagSet(appinfo_t::INSTALLED) )
        menu.addAction(tr("Uninstall"));
    if( info->isFlagSet(appinfo_t::INSTALLED) && info->isFlagSet(appinfo_t::DOWNLOADED) )
        menu.addAction(tr("ReInstall"));

    menu.addSeparator();
    if( info->isFlagSet(appinfo_t::DOWNLOADED) )
    {
        menu.addAction(tr("Execute installer"));
        menu.addAction(tr("Open containing Folder"));
    }
    menu.addSeparator();
    if( info->isFlagSet(appinfo_t::INSTALLED) && info->isFlagSet(appinfo_t::UPDATE_AVAIL) )
        menu.addAction(tr("force latest version"));
    if( info->isFlagSet(appinfo_t::INSTALLED) || info->isFlagSet(appinfo_t::DOWNLOADED) )
    {
        if(info->isFlagSet(appinfo_t::IGNORE_LATEST))
            menu.addAction(tr("unset ignore newer versions"));
        else
            menu.addAction(tr("set ignore newer versions"));
    }

    /***************************************************
     Execute menu
     ***************************************************/

    menu.move(ui->LAppInfoList->mapToGlobal(pos+QPoint(0,25)));
    QAction *action = menu.exec();
    if(action==NULL)
        return;

    if( action->text() == tr("Select"))
    {
        ChooseDialog dialog(info,this);
        task_t *task = dialog.exec();
        if(task!=NULL)
        {
            emit taskChosen(task);
            info->setFlag(appinfo_t::SELECTED_INST_DL);
            item->setIcon(0,info->getIcon());
        }
    }
    if( action->text() == tr("Unselect"))
    {
        emit unSelected(info);
        info->unSetFlag(appinfo_t::SELECTED_INST_DL);
        item->setIcon(0,info->getIcon());
    }

    if( action->text() == tr("Check Latest Version") )
    {
        qDebug("checking version");
        connect(info,SIGNAL(infoUpdated(appinfo_t*,bool)),SLOT(onVersionUpdated(appinfo_t*,bool)));
        info->updateVersion();
    }

    if( action->text() == tr("Update") )
    {
        on_LAppInfoList_itemActivated(item,0);
    }

    if( action->text() == tr("Uninstall"))
    {
        task_t *task = new task_t(info,task_t::UNINSTALL);
        emit taskChosen(task);
        info->setFlag(appinfo_t::SELECTED_REM);
        item->setIcon(0,info->getIcon());
    }
    if( action->text() == tr("ReInstall"))
    {
        task_t *task = new task_t(info,task_t::INSTALL);
        emit taskChosen(task);
        info->setFlag(appinfo_t::SELECTED_INST_DL);
        item->setIcon(0,info->getIcon());
    }

    if( action->text() == tr("Execute installer"))
    {
        QDesktopServices::openUrl(tr("file:///%1").arg(info->Path+tr("/")+info->fileName));
    }
    if( action->text() == tr("Open containing Folder"))
    {
        QDesktopServices::openUrl(tr("file:///%1").arg(info->Path));
    }
    if( action->text() == tr("force latest version"))
    {
        if( !info->forceRegistryToLatestVersion() )
            QMessageBox::information(this,tr("setting registry value"),tr("failed to force latest version , try again as admin!"));
    }
    if( action->text() == tr("set ignore newer versions"))
    {
        info->setFlag(appinfo_t::IGNORE_LATEST);
        info->LatestVersion = info->InstalledVersion.isEmpty()?info->DlVersion:info->InstalledVersion;
    }
    if( action->text() == tr("unset ignore newer versions"))
    {
        info->unSetFlag(appinfo_t::IGNORE_LATEST);
        info->LatestVersion = info->InstalledVersion.isEmpty()?info->DlVersion:info->InstalledVersion;
    }
}

void applist_t::updateItem(appinfo_t *appinfo)
{
    QList<QTreeWidgetItem*> items = ui->LAppInfoList->findItems(appinfo->Name,Qt::MatchExactly);
    if(items.count()==0)
        return;
    QTreeWidgetItem *item = items.at(0);
    if(item==NULL)
        return;

    item->setIcon(0,appinfo->getIcon());
    item->setText(1,appinfo->DlVersion.isEmpty()?appinfo->InstalledVersion:appinfo->DlVersion);
    item->setText(2,appinfo->LatestVersion);
    item->setText(4,appinfo->LastVersionCheck);
}

void applist_t::onVersionUpdated(appinfo_t *appinfo, bool updated)
{
    QString msg;
    if(updated)
        msg = tr("succesfuly retrieved latest version for %1 :\n -> %2").arg(appinfo->Name,appinfo->LatestVersion);
    else
        msg = tr("failed to retrieve latest version for ") + appinfo->Name;
    QMessageBox::information(this,tr("Version updated"),msg);
    disconnect(appinfo,SIGNAL(infoUpdated(appinfo_t*,bool)),this,SLOT(onVersionUpdated(appinfo_t*,bool)));
}

void applist_t::clear()
{
    while( !fileinfo_list.isEmpty() )
    {
        fileinfo_t fileinfo = fileinfo_list.at(0);
        fileinfo_list.removeFirst();
        delete fileinfo.info;
    }
}


void applist_t::saveList()
{
    QFile appinfolist_file(tr("Info/PkgList.xml"));

    if( !appinfolist_file.open(QFile::WriteOnly) )
        return;

    QDomDocument doc;
    QDomElement root = doc.createElement(tr("PKG_LIST"));
    doc.appendChild(root);
    for(int i=0;i<fileinfo_list.count();i++)
    {
        QDomElement node = doc.createElement(tr("PKG_LIST"));
        node = doc.createElement(tr("PKG"));
        node.setAttribute(tr("name"),fileinfo_list.at(i).name);
        node.setAttribute(tr("LastUpdate"),fileinfo_list.at(i).lastUpdate);
        root.appendChild(node);
    }
    appinfolist_file.write(doc.toByteArray());
}


void applist_t::loadCategoryTree(QDomElement node,QTreeWidgetItem *item)
{
    while(!node.isNull())
    {
        QString category = node.tagName();
        QTreeWidgetItem *child = new QTreeWidgetItem;
        child->setText(0,category);
        item->addChild(child);
        loadCategoryTree(node.firstChildElement(),child);
        node = node.nextSiblingElement();
    }
}

bool applist_t::loadList()
{

    QFile appinfolist_file(tr("Info/PkgList.xml"));

    if( !appinfolist_file.open(QFile::ReadOnly) )
        return false;

    QDomDocument doc;
    if( !doc.setContent(&appinfolist_file) )
        return false;

    QDomElement node = doc.documentElement();

    node = node.firstChildElement();

    while( !node.isNull() )
    {
        fileinfo_t fileinfo;
        fileinfo.name       = node.attribute(tr("name"));
        fileinfo.lastUpdate = node.attribute(tr("LastUpdate"));

        /*fixes old bug in old appication that copied entries*/
        if( fileinfo_list.contains(fileinfo) )
        {
            node = node.nextSiblingElement();
            continue;
        }

        if( !fileinfo.name.isEmpty() )
        {
            fileinfo.info = new appinfo_t(fileinfo.name);
            if( fileinfo.info->loadFileInfo() )
            {
                fileinfo_list.append(fileinfo);
                connect(fileinfo.info,SIGNAL(infoUpdated(appinfo_t*,bool)),SLOT(updateItem(appinfo_t*)));
            }
            else
                delete fileinfo.info;
        }
        node = node.nextSiblingElement();
    }
    saveList();

    QFile categories_file(tr("Info/TreeInfo.xml"));
    if( !categories_file.open(QFile::ReadOnly) )
    {
        qDebug("failed to open Info/TreeInfo.xml");
        return false;
    }

    doc.clear();
    if( !doc.setContent(&categories_file) )
    {
        qDebug("failed to set content");
        return false;
    }

    node = doc.documentElement().firstChildElement();
    QTreeWidgetItem *root = new QTreeWidgetItem();
    loadCategoryTree(node,root);
    while(root->childCount()>0)
    {
        QTreeWidgetItem *child = root->child(0);
        root->removeChild(child);
        ui->TCategoryTree->addTopLevelItem(child);
    }

    return true;
}

void applist_t::appendToList(fileinfo_t &info)
{
    QStringList columninfo;
    columninfo << info.name << (info.info->InstalledVersion.isEmpty()?info.info->DlVersion:info.info->InstalledVersion) << info.info->LatestVersion << info.info->Description << info.info->LastVersionCheck;
    TreeWidgetItem *item = new TreeWidgetItem(columninfo);
    item->setToolTip(3,info.info->Description);
    item->setToolTip(0,info.info->Name);
    item->setIcon(0,info.info->getIcon());
    item->setData(0,QTreeWidgetItem::UserType,qVariantFromValue(info));

    ui->LAppInfoList->addTopLevelItem(item);
}

void applist_t::setListByCategory(QString cat)
{
    appinfo_t::appinfo_flags flag=appinfo_t::INVALID;

    if( cat == tr("All") )
    {
        for(int i=0;i<fileinfo_list.count();i++)
        {
            fileinfo_t &f = (fileinfo_t&)fileinfo_list.at(i);
            appendToList(f);
        }
        return;
    }

    if( cat == tr("Only Downloaded") )
    {
        for(int i=0;i<fileinfo_list.count();i++)
        {
            fileinfo_t &f = (fileinfo_t&)fileinfo_list.at(i);
            if( f.info->isFlagSet(appinfo_t::DOWNLOADED) && !f.info->isFlagSet(appinfo_t::INSTALLED) )
                appendToList(f);
        }
        return;
    }
    if( cat == tr("Only Installed") )
    {
        for(int i=0;i<fileinfo_list.count();i++)
        {
            fileinfo_t &f = (fileinfo_t&)fileinfo_list.at(i);
            if( f.info->isFlagSet(appinfo_t::INSTALLED) && !f.info->isFlagSet(appinfo_t::DOWNLOADED) )
                appendToList(f);
        }
        return;
    }




    if( cat == tr("New/Updated") )
        flag = appinfo_t::NEW;

    if( cat == tr("Updates") )
        flag = appinfo_t::UPDATE_AVAIL;

    if( cat == tr("Downloaded") )
        flag = appinfo_t::DOWNLOADED;

    if( cat == tr("Installed") )
        flag = appinfo_t::INSTALLED;

    if( cat == tr("Only Info") )
        flag = appinfo_t::ONLY_INFO;

    if( flag != appinfo_t::INVALID)
    {
        for(int i=0;i<fileinfo_list.count();i++)
        {
            fileinfo_t &f = (fileinfo_t&)fileinfo_list.at(i);
            if( f.info->isFlagSet(flag) )
                appendToList(f);
        }

        if(flag == appinfo_t::UPDATE_AVAIL && ui->LAppInfoList->topLevelItemCount()>0)
            version_updates_avail = true;
        else
            version_updates_avail = false;
        return;
    }

    for(int i=0;i<fileinfo_list.count();i++)
    {
        fileinfo_t &f = (fileinfo_t&)fileinfo_list.at(i);
        if( f.info->categories.contains(cat) )
            appendToList(f);
    }
    return;
}


void applist_t::on_LAppInfoList_itemActivated(QTreeWidgetItem *item, int column)
{
    applist_t::fileinfo_t data = item->data(0,QTreeWidgetItem::UserType).value<applist_t::fileinfo_t>();
    appinfo_t *info = data.info;

    if( !info->isFlagSet(appinfo_t::SELECTED_REM) && !info->isFlagSet(appinfo_t::SELECTED_INST_DL) )
    {
        task_t *task = NULL;

        if(info->isFlagSet(appinfo_t::UPDATE_AVAIL))
        {
            appinfo_t::inet_file_t inet_file = info->getFileInfoByDlId();
            if(inet_file.description.isEmpty())
            {
                info->unSetFlag(appinfo_t::DOWNLOADED);
            }else
            {
                if(info->isFlagSet(appinfo_t::INSTALLED))
                    task = new task_t(info,inet_file,task_t::DOWNLOAD|task_t::INSTALL);
                else
                    task = new task_t(info,inet_file,task_t::DOWNLOAD);

                info->setFlag(appinfo_t::SELECTED_INST_DL);
                goto chosen;
            }
            ChooseDialog dialog(info,this);
            task = dialog.exec();
            if(task!=NULL)
            {
                info->setFlag(appinfo_t::SELECTED_INST_DL);
                emit taskChosen(task);
                item->setIcon(0,info->getIcon());
                return;
            }else
                return;
        }

        if(info->isFlagSet(appinfo_t::INSTALLED))
        {
            /*always try silent uninstall*/
            task = new task_t(info,task_t::UNINSTALL);
            if( !info->uninstall_param.isEmpty() || !info->registry_info.silent_uninstall.isEmpty())
            {
                int ans = QMessageBox::question(this,tr("Uninstall Mode"),tr("%1 :UNinstall in silent mode?").arg(info->Name),QMessageBox::Yes|QMessageBox::No);
                if(ans == QMessageBox::Yes)
                    task->set(task_t::SILENT);
            }
            info->setFlag(appinfo_t::SELECTED_REM);
            goto chosen;
        }

        if(info->isFlagSet(appinfo_t::DOWNLOADED))
        {
            appinfo_t::inet_file_t inet_file = info->getFileInfoByDlId();
            if(inet_file.description.isEmpty())
            {
                info->unSetFlag(appinfo_t::DOWNLOADED);
            }else
            {
                task = new task_t(info,inet_file,task_t::INSTALL);
                info->setFlag(appinfo_t::SELECTED_INST_DL);
                goto chosen;
            }
        }

        if(!info->isFlagSet(appinfo_t::DOWNLOADED) && !info->isFlagSet(appinfo_t::INSTALLED))
        {
            ChooseDialog dialog(info,this);
            task = dialog.exec();
            if(task!=NULL)
            {
                info->setFlag(appinfo_t::SELECTED_INST_DL);
                emit taskChosen(task);
                item->setIcon(0,info->getIcon());
                return;
            }else
                return;
        }


        chosen:

        if(task!=NULL)
        {
            if(task->isSet(task_t::INSTALL) && !info->isFlagSet(appinfo_t::UPDATE_AVAIL))
            {
                switch(SettingsDialog::getInstallMode())
                {
                case SettingsDialog::ASK:
                {
                    int ans = QMessageBox::question(this,tr("Install Mode"),tr("%1 :Install in silent mode?").arg(info->Name),QMessageBox::Yes|QMessageBox::No);
                    if(ans == QMessageBox::Yes)
                        task->set(task_t::SILENT);
                    break;
                }
                case SettingsDialog::SILENT:
                    task->set(task_t::SILENT);
                }
            }
            if(task->isSet(task_t::INSTALL) && info->isFlagSet(appinfo_t::UPDATE_AVAIL))
            {
                switch(SettingsDialog::getUpgradeMode())
                {
                case SettingsDialog::ASK:
                {
                    int ans = QMessageBox::question(this,tr("Upgrade Mode"),tr("%1 :Upgrade in silent mode?").arg(info->Name),QMessageBox::Yes|QMessageBox::No);
                    if(ans == QMessageBox::Yes)
                        task->set(task_t::SILENT);
                    break;
                }
                case SettingsDialog::SILENT:
                    task->set(task_t::SILENT);
                }
            }
            emit taskChosen(task);
            item->setIcon(0,info->getIcon());
        }
        return;
    }

    if( info->isFlagSet(appinfo_t::SELECTED_INST_DL) || info->isFlagSet(appinfo_t::SELECTED_REM))
    {
        emit unSelected(info);
    }
}


void applist_t::onSetAppInfoUpdated(QString name, QString lastUpdate)
{
    qDebug("updaing app info");

    fileinfo_t sf;
    sf.name = name;
    int index = fileinfo_list.indexOf(sf);
    if( index != -1)
    {
        qDebug("updated info");
        fileinfo_t &f = (fileinfo_t&) fileinfo_list.at(index);
        f.lastUpdate = lastUpdate;
        f.info->saveApplicationInfo();
        f.info->loadFileInfo();
        f.info->setFlag(appinfo_t::NEW);
        f.info->saveApplicationInfo();
    }else
    {
        qDebug("added new");
        fileinfo_t f;
        f.info = new appinfo_t(name);
        f.lastUpdate = lastUpdate;
        f.name = name;
        if( f.info->loadFileInfo() )
        {
            f.info->setFlag(appinfo_t::NEW);
            fileinfo_list.append(f);
            connect(f.info,SIGNAL(infoUpdated(appinfo_t*,bool)),SLOT(updateItem(appinfo_t*)));
            f.info->saveApplicationInfo();
        }
        else
            delete f.info;
    }
}

void applist_t::onCheckForNewAppInfo()
{
    if( fileinfo_list.isEmpty() )
    {
        info_updates_avail = true;
        int ans = QMessageBox::warning(this,tr("No list"),tr("Unable to load the application list.\n Download the list now?"),QMessageBox::Yes|QMessageBox::No);
        if( ans == QMessageBox::Yes )
        {
            on_btUpdateInfo_clicked();
        }
    }else
    {
        UpdaterDialog *dlg = new UpdaterDialog(fileinfo_list,UpdaterDialog::update_check_appinfo);
        dlg->setAttribute(Qt::WA_DeleteOnClose,true);
        connect(dlg,SIGNAL(accepted()),SLOT(onNewAppInfoAvailable()));
        qDebug("checking for new info");
    }
}

void applist_t::onNewAppInfoAvailable()
{
    qDebug("new info");
    info_updates_avail = true;
    ui->btUpdateInfo->setIcon(QIcon(tr(":icons/information.ico")));
    ui->btUpdateInfo->raise();
}

void applist_t::onRemovedFromTasks(appinfo_t *info)
{
    info->unSetFlag(appinfo_t::SELECTED_INST_DL);
    info->unSetFlag(appinfo_t::SELECTED_REM);
    QList<QTreeWidgetItem*> items = ui->LAppInfoList->findItems(info->Name,Qt::MatchContains);
    if(items.count()>0)
        items.at(0)->setIcon(0,info->getIcon());
}

void applist_t::on_btUpdate_clicked()
{
    QList<fileinfo_t> list;
    for(int i=0;i<fileinfo_list.count();i++)
    {
        fileinfo_t &f = (fileinfo_t&)fileinfo_list.at(i);
        if( f.info->isFlagSet(appinfo_t::DOWNLOADED) || f.info->isFlagSet(appinfo_t::INSTALLED) )
            list.append(f);
    }
    UpdaterDialog dlg(list,UpdaterDialog::update_version);
    dlg.move( mapToGlobal( this->rect().center()/2 ));
    if(!this->isVisible())
        dlg.showMinimized();
    dlg.exec();
    QTreeWidgetItem *item = ui->TCategoryTree->findItems(tr("Updates"),Qt::MatchExactly).at(0);
    ui->TCategoryTree->setCurrentItem(item);
}

void applist_t::on_btCheckUpdates_clicked()
{
    QTreeWidgetItem *item = ui->TCategoryTree->findItems(tr("Updates"),Qt::MatchExactly).at(0);
    ui->TCategoryTree->setCurrentItem(item,0,QItemSelectionModel::ClearAndSelect);

    for(int i=0;i<ui->LAppInfoList->topLevelItemCount();i++)
    {
        item = ui->LAppInfoList->topLevelItem(i);
        on_LAppInfoList_itemActivated(item,0);
    }
}

void applist_t::on_btUpdateInfo_clicked()
{

    if(info_updates_avail)
    {
        /*reset all appinfo from new to only_info*/
        for(int i=0;i<fileinfo_list.count();i++)
        {
            fileinfo_list[i].info->unSetFlag(appinfo_t::NEW);
            fileinfo_list[i].info->saveApplicationInfo();
        }
        UpdaterDialog dlg(fileinfo_list,UpdaterDialog::update_appinfo);
        dlg.move( mapToGlobal( this->rect().center()/2 ));
        connect(&dlg,SIGNAL(newFileInfoUpdated(QString,QString)),SLOT(onSetAppInfoUpdated(QString,QString)));
        int result = dlg.exec() ;
        if( result )
        {
            qDebug("saving list");
            info_updates_avail = false;
            ui->btUpdateInfo->setIcon(QIcon(tr(":icons/isnew.ico")));
            saveList();
            QTreeWidgetItem *item = ui->TCategoryTree->findItems(tr("New/Updated"),Qt::MatchExactly).at(0);
            ui->TCategoryTree->setCurrentItem(item,0,QItemSelectionModel::ClearAndSelect);
        }
    }else
        onCheckForNewAppInfo();
}

void applist_t::on_lbSearch_textChanged(const QString &arg1)
{
    ui->LAppInfoList->clear();
    QStringList strings = arg1.split(' ');

    for(int i=0;i<fileinfo_list.count();i++)
    {
        if( fileinfo_list.at(i).info->contains(strings) )
            appendToList(fileinfo_list[i]);
    }
}
