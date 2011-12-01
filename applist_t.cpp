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
#include "appinfo_registry_t.h"

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
    this->setEnabled(false);
    info_updates_avail = false;
    version_updates_avail = false;
    ui->LAppInfoList->sortItems(0,Qt::AscendingOrder);



    QTreeWidgetItem *item = new QTreeWidgetItem(QStringList("All"));
    ui->TCategoryTree->addTopLevelItem(item);

    QTimer::singleShot(100,this,SLOT(loadList()));
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
    ui->AppInfoText->append("<a href='"+info.info->WebPage+"'>"+info.info->WebPage+"</a>");

    ui->AppInfoText->append(info.info->Description+"\n");
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
    if(info->isFlagSet(appinfo_t::NO_INFO))
    {
	menu.addAction("Uninstall");
	goto execute;
    }
    if(info->isFlagSet(appinfo_t::SELECTED_INST_DL) || info->isFlagSet(appinfo_t::SELECTED_REM))
	menu.addAction("Unselect");
    else
	menu.addAction("Select");
    menu.addAction("Check Latest Version");
    menu.addSeparator();

    if( info->isFlagSet(appinfo_t::UPDATE_AVAIL) )
	menu.addAction("Update");


    if( info->isFlagSet(appinfo_t::INSTALLED) )
        menu.addAction("Uninstall");


    if( info->isFlagSet(appinfo_t::INSTALLED) && info->isFlagSet(appinfo_t::DOWNLOADED) )
        menu.addAction("ReInstall");

    menu.addSeparator();
    if( info->isFlagSet(appinfo_t::DOWNLOADED) )
    {
        menu.addAction("Execute installer");
        menu.addAction("Delete downloaded File");
        menu.addAction("Open containing Folder");
    }
    menu.addSeparator();
    if( info->isFlagSet(appinfo_t::INSTALLED) && info->isFlagSet(appinfo_t::UPDATE_AVAIL) )
        menu.addAction("force latest version");
    if( info->isFlagSet(appinfo_t::INSTALLED) || info->isFlagSet(appinfo_t::DOWNLOADED) )
    {
        if(info->isFlagSet(appinfo_t::IGNORE_LATEST))
            menu.addAction("unset ignore newer versions");
        else
            menu.addAction("set ignore newer versions");
    }
    menu.addSeparator();

    if(info->version_info.url.contains("filehippo") )
    {
	if(info->isFlagSet(appinfo_t::FILEHIPPO_BETA))
	    menu.addAction("disable Beta");
	if(!info->isFlagSet(appinfo_t::FILEHIPPO_BETA))
	    menu.addAction("enable Beta");
    }

    /***************************************************
     Execute menu
     ***************************************************/
    execute:

    menu.move(ui->LAppInfoList->mapToGlobal(pos+QPoint(0,25)));
    QAction *action = menu.exec();
    if(action==NULL)
        return;

    if( action->text() == "Select")
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
    if( action->text() == "Unselect")
    {
        emit unSelected(info);
        info->unSetFlag(appinfo_t::SELECTED_INST_DL);
        item->setIcon(0,info->getIcon());
    }

    if( action->text() == "Check Latest Version" )
    {
        qDebug("checking version");
        emit status(tr("checking version for :")+info->Name);
        connect(info,SIGNAL(infoUpdated(appinfo_t*,bool)),SLOT(onVersionUpdated(appinfo_t*,bool)));
        info->updateVersion();
    }

    if( action->text() == "Update" )
    {
        on_LAppInfoList_itemActivated(item,0);
    }

    if( action->text() == "Uninstall")
    {
        task_t *task = new task_t(info,task_t::UNINSTALL);
        emit taskChosen(task);
        info->setFlag(appinfo_t::SELECTED_REM);
        item->setIcon(0,info->getIcon());
    }
    if( action->text() == "ReInstall")
    {
        task_t *task = new task_t(info,task_t::INSTALL);
	if(!task->m_appinfo->install_param.isEmpty() && task->m_appinfo->install_param!="ALWAYS"
		&& !task->m_appinfo->isFlagSet(appinfo_t::ONLY_SILENT))
        {
            switch(SettingsDialog::getInstallMode())
            {
            case SettingsDialog::ASK:
            {
                int ans = QMessageBox::question(this,"Install Mode",tr("%1 :Install in silent mode?").arg(info->Name),QMessageBox::Yes|QMessageBox::No);
                if(ans == QMessageBox::Yes)
                    task->set(task_t::SILENT);
                break;
            }
            case SettingsDialog::SILENT:
                task->set(task_t::SILENT);
            }
        }
        emit taskChosen(task);
        info->setFlag(appinfo_t::SELECTED_INST_DL);
        item->setIcon(0,info->getIcon());
    }

    if( action->text() == "Execute installer")
    {
        QDesktopServices::openUrl(tr("file:///%1").arg(info->Path+"/"+info->fileName));
    }
    if( action->text() == "Open containing Folder")
    {
        QDesktopServices::openUrl(tr("file:///%1").arg(info->Path));
    }
    if( action->text() == "Delete downloaded File")
    {
        QFile::remove( info->Path + "/" + info->fileName);
        info->unSetFlag(appinfo_t::DOWNLOADED);
        info->DlVersion.clear();
        updateItem(info);
    }

    if( action->text() == "force latest version")
    {
        if( !info->forceRegistryToLatestVersion() )
            QMessageBox::information(this,"setting registry value","failed to force latest version , try again as admin!");
        else
            updateItem(info);
    }
    if( action->text() == "set ignore newer versions")
    {
        info->setFlag(appinfo_t::IGNORE_LATEST);
        info->unSetFlag(appinfo_t::UPDATE_AVAIL);
        info->LatestVersion.clear();
    }
    if( action->text() == "unset ignore newer versions")
    {
        info->unSetFlag(appinfo_t::IGNORE_LATEST);
        info->updateVersion();
    }

    if( action->text() == "disable Beta")
	info->setFilehippoBeta(false);

    if( action->text() == "enable Beta")
	    info->setFilehippoBeta(true);
}

void applist_t::updateItem(appinfo_t *appinfo)
{
    QList<QTreeWidgetItem*> items = ui->LAppInfoList->findItems(appinfo->Name,Qt::MatchExactly);
    QTreeWidgetItem *item = NULL;
    for(int i=0;i<items.count();i++)
    {
        item = items[i];
        applist_t::fileinfo_t data = item->data(0,QTreeWidgetItem::UserType).value<applist_t::fileinfo_t>();
        appinfo_t *item_info = data.info;
        if(item_info==appinfo)
            break;
    }
    if(item==NULL)
        return;

    item->setIcon(0,appinfo->getIcon());
    item->setText(1,appinfo->registry_info.version.isEmpty()?appinfo->DlVersion:appinfo->registry_info.version);
    item->setText(2,appinfo->LatestVersion);
    item->setText(4,appinfo->LastVersionCheck);
}

void applist_t::onVersionUpdated(appinfo_t *appinfo, bool updated)
{
    QString msg;
    if(updated)
        msg = tr("succesfuly retrieved latest version for %1 :\n -> %2").arg(appinfo->Name,appinfo->LatestVersion);
    else
        msg = "failed to retrieve latest version for " + appinfo->Name;
    QMessageBox::information(this,"Version updated",msg);
    disconnect(appinfo,SIGNAL(infoUpdated(appinfo_t*,bool)),this,SLOT(onVersionUpdated(appinfo_t*,bool)));
}

void applist_t::clear()
{
    ui->LAppInfoList->clear();
    ui->TCategoryTree->clear();
    while( !fileinfo_list.isEmpty() )
    {
        fileinfo_t fileinfo = fileinfo_list.takeFirst();
        emit unSelected(fileinfo.info);
        delete fileinfo.info;
    }
}


void applist_t::saveList()
{
    QFile appinfolist_file("Info/PkgList.xml");

    if( !appinfolist_file.open(QFile::WriteOnly) )
        return;

    QDomDocument doc;
    QDomElement root = doc.createElement("PKG_LIST");
    doc.appendChild(root);
    for(int i=0;i<fileinfo_list.count();i++)
    {
	if(fileinfo_list[i].info->isFlagSet(appinfo_t::NO_INFO))
	    continue;
	QDomElement node = doc.createElement("PKG");
        node.setAttribute("name",fileinfo_list.at(i).name);
        node.setAttribute("LastUpdate",fileinfo_list.at(i).lastUpdate);
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


void applist_t::setRegistryInfo(registry_group_t group)
{
    QString name = group.displayed_name;
    QString groupName = group.group_name;
    ui->progressBar->setValue(ui->progressBar->value()+1);
    if(name.isEmpty())
        return;

    QList<fileinfo_t>::iterator i;
    for(i = fileinfo_list.begin();i!=fileinfo_list.end();i++)
    {
        fileinfo_t *info = &(*i);
        if(name.toUpper().contains(info->info->registry_info.seachValue.toUpper() ) ||
           groupName.toUpper().contains(info->info->registry_info.seachValue.toUpper()))
        {
            if(!info->info->registry_info.path.isEmpty())
            {
		return;
            }

            info->info->setRegistryInfo(&group);
            qDebug(info->name.toAscii());
            qDebug(info->info->registry_info.path.toAscii());
            return;
        }
    }

    if(!SettingsDialog::showAllApps())
        return;

    fileinfo_t fileinfo;
    fileinfo.info = new appinfo_t(group.displayed_name);
    fileinfo.info->setRegistryInfo(&group);
    fileinfo.info->setFlag(appinfo_t::NO_INFO);
    fileinfo.info->registry_info.displayName = group.displayed_name;
    fileinfo.name = group.displayed_name;


    fileinfo_list.append(fileinfo);
}

bool applist_t::loadList()
{
    QFile appinfolist_file("Info/PkgList.xml");
    bool listDownloaded = false;
    clear();
    ui->progressBar->show();
    ui->progressBar->setValue(0);
    //ui->lbLoad->show();
    QApplication::processEvents();

    if( !appinfolist_file.open(QFile::ReadOnly) )
    {
        listDownloaded = true;
        onCheckForNewAppInfo();
        SettingsDialog::setUpdatedToLatestVersion();
        if( !appinfolist_file.open(QFile::ReadOnly) )
        {
            QMessageBox::information(NULL,"Error opening File","failed to open file \"Info/PkgList.xml\"");
            return false;
        }
    }

    if(!SettingsDialog::updatedToLatestVersion() && !listDownloaded)
    {
        listDownloaded = true;
        onCheckForNewAppInfo();
        SettingsDialog::setUpdatedToLatestVersion();
    }


    QDomDocument doc;
    if( !doc.setContent(&appinfolist_file) )
    {
        QMessageBox message;
        message.setWindowTitle("Error loading Xml-File");
        message.setText("<span>failed to load Info/PkgList.xml\n</span> delete the specified file or replace it with: <a href='http://appdriverupdate.sourceforge.net/Apps/Files/PkgList.xml'> PkgList.xml </a>");
        message.exec();
        return false;
    }

    QDomElement node = doc.documentElement();

    node = node.firstChildElement();

    while( !node.isNull() )
    {
        fileinfo_t fileinfo;
        fileinfo.name       = node.attribute("name");
        fileinfo.lastUpdate = node.attribute("LastUpdate");

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


    appinfo_registry_t reginfo;
    ui->progressBar->setMaximum(reginfo.getCount());

    qRegisterMetaType<registry_group_t>();
    connect(&reginfo,SIGNAL(registry_app_info(registry_group_t)),SLOT(setRegistryInfo(registry_group_t)));
    reginfo.getAppsFromRegistry();

    qSort(fileinfo_list.begin(),fileinfo_list.end());
    this->setEnabled(true);

    ui->progressBar->hide();
    ui->lbLoad->hide();
    ui->horizontalLayout->insertSpacerItem(3,new QSpacerItem(40, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));

    QFile categories_file("Info/TreeInfo.xml");
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

    QTreeWidgetItem *item;
    item = new QTreeWidgetItem(QStringList("Updates"));
    ui->TCategoryTree->addTopLevelItem(item);
    item = new QTreeWidgetItem(QStringList("Installed"));
    ui->TCategoryTree->addTopLevelItem(item);
    item = new QTreeWidgetItem(QStringList("Downloaded"));
    ui->TCategoryTree->addTopLevelItem(item);
    item = new QTreeWidgetItem(QStringList("New/Updated"));
    ui->TCategoryTree->addTopLevelItem(item);
    item = new QTreeWidgetItem(QStringList("Only Installed"));
    ui->TCategoryTree->addTopLevelItem(item);
    item = new QTreeWidgetItem(QStringList("Only Downloaded"));
    ui->TCategoryTree->addTopLevelItem(item);
    item = new QTreeWidgetItem(QStringList("Only Info"));
    ui->TCategoryTree->addTopLevelItem(item);

    if(SettingsDialog::showAllApps())
    {
        item = new QTreeWidgetItem(QStringList("all installed apps"));
        ui->TCategoryTree->addTopLevelItem(item);
    }

    ui->TCategoryTree->setCurrentItem(ui->TCategoryTree->topLevelItem(0),0,QItemSelectionModel::Select);

    if( (SettingsDialog::shouldCheckAppInfo() && SettingsDialog::lastInfoUpdate() < QDate::currentDate())
            || fileinfo_list.isEmpty())
    {
        if(!listDownloaded)
            QTimer::singleShot(500,this,SLOT(onCheckForNewAppInfo()));
        SettingsDialog::setLastInfoUpdate(QDate::currentDate());
    }

    return true;
}

void applist_t::appendToList(fileinfo_t &info)
{
    QStringList columninfo;
    columninfo << info.name << (info.info->registry_info.version.isEmpty()?info.info->DlVersion:info.info->registry_info.version) << info.info->LatestVersion << info.info->Description << info.info->LastVersionCheck;
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

    ui->AppInfoText->show();
    ui->LAppInfoList->setColumnCount(5);
    ui->LAppInfoList->setColumnWidth(0,150);

    if( cat == "All" )
    {
        for(int i=0;i<fileinfo_list.count();i++)
        {
            fileinfo_t &f = (fileinfo_t&)fileinfo_list.at(i);
            if(f.info->isFlagSet(appinfo_t::NO_INFO))
                continue;
            appendToList(f);
        }
        return;
    }

    if( cat == "Only Downloaded" )
    {
        for(int i=0;i<fileinfo_list.count();i++)
        {
            fileinfo_t &f = (fileinfo_t&)fileinfo_list.at(i);
            if(f.info->isFlagSet(appinfo_t::NO_INFO))
                continue;
            if( f.info->isFlagSet(appinfo_t::DOWNLOADED) && !f.info->isFlagSet(appinfo_t::INSTALLED) )
                appendToList(f);
        }
        return;
    }
    if( cat == "Only Installed" )
    {
        for(int i=0;i<fileinfo_list.count();i++)
        {
            fileinfo_t &f = (fileinfo_t&)fileinfo_list.at(i);
            if(f.info->isFlagSet(appinfo_t::NO_INFO))
                continue;
            if( f.info->isFlagSet(appinfo_t::INSTALLED) && !f.info->isFlagSet(appinfo_t::DOWNLOADED) )
                appendToList(f);
        }
        return;
    }
    if( cat == "New/Updated" )
        flag = appinfo_t::NEW;

    if( cat == "Updates" )
        flag = appinfo_t::UPDATE_AVAIL;

    if( cat == "Downloaded" )
        flag = appinfo_t::DOWNLOADED;

    if( cat == "Installed" )
        flag = appinfo_t::INSTALLED;

    if( cat == "Only Info" )
        flag = appinfo_t::ONLY_INFO;

    if( cat == "all installed apps" )
        flag = appinfo_t::NO_INFO;

    if( flag != appinfo_t::INVALID )
    {
        for(int i=0;i<fileinfo_list.count();i++)
        {
            fileinfo_t &f = (fileinfo_t&)fileinfo_list.at(i);
            if(flag!=appinfo_t::NO_INFO && f.info->isFlagSet(appinfo_t::NO_INFO))
                continue;

            if( f.info->isFlagSet(flag) )
            {
                appendToList(f);
                continue;
            }

            if( flag == appinfo_t::NO_INFO && f.info->isFlagSet(appinfo_t::INSTALLED) )
                appendToList(f);
        }

        if(flag==appinfo_t::NO_INFO)
        {
            ui->AppInfoText->hide();
            ui->LAppInfoList->setColumnCount(1);
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
            appinfo_t::file_info_t inet_file = info->getFileInfoByDlId();
            if(inet_file.description.isEmpty())
            {
                info->unSetFlag(appinfo_t::DOWNLOADED);
            }else
            {
                if( info->LatestVersion == info->DlVersion )
                {
                    task = new task_t(info,inet_file,task_t::INSTALL);
                }else
                {
                    if(info->isFlagSet(appinfo_t::INSTALLED))
                        task = new task_t(info,inet_file,task_t::DOWNLOAD|task_t::INSTALL);
                    else
                        task = new task_t(info,inet_file,task_t::DOWNLOAD);
                }
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
            task = new task_t(info,task_t::UNINSTALL);
            if( (!info->uninstall_param.isEmpty()&&info->uninstall_param!="ALWAYS")|| !info->registry_info.silent_uninstall.isEmpty())
            {
                int ans = QMessageBox::question(this,"Uninstall Mode",tr("%1 :UNinstall in silent mode?").arg(info->Name),QMessageBox::Yes|QMessageBox::No);
                if(ans == QMessageBox::Yes)
                    task->set(task_t::SILENT);
            }
            info->setFlag(appinfo_t::SELECTED_REM);
            goto chosen;
        }

        if(info->isFlagSet(appinfo_t::DOWNLOADED))
        {
            appinfo_t::file_info_t inet_file = info->getFileInfoByDlId();
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
                if(!task->m_appinfo->install_param.isEmpty() && task->m_appinfo->install_param!="ALWAYS" && !task->m_appinfo->isFlagSet(appinfo_t::ONLY_SILENT))
                {
                    switch(SettingsDialog::getInstallMode())
                    {
                    case SettingsDialog::ASK:
                    {
                        int ans = QMessageBox::question(this,"Install Mode",tr("%1 :Install in silent mode?").arg(info->Name),QMessageBox::Yes|QMessageBox::No);
                        if(ans == QMessageBox::Yes)
                            task->set(task_t::SILENT);
                        break;
                    }
                    case SettingsDialog::SILENT:
                        task->set(task_t::SILENT);
                    }
                }
            }

            if(task->isSet(task_t::INSTALL) && info->isFlagSet(appinfo_t::UPDATE_AVAIL) && !task->m_appinfo->isFlagSet(appinfo_t::ONLY_SILENT))
            {
                if(!task->m_appinfo->install_param.isEmpty() && task->m_appinfo->install_param!="ALWAYS")
                {
                    switch(SettingsDialog::getUpgradeMode())
                    {
                    case SettingsDialog::ASK:
                    {
                        int ans = QMessageBox::question(this,"Upgrade Mode",tr("%1 :Upgrade in silent mode?").arg(info->Name),QMessageBox::Yes|QMessageBox::No);
                        if(ans == QMessageBox::Yes)
                            task->set(task_t::SILENT);
                        break;
                    }
                    case SettingsDialog::SILENT:
                        task->set(task_t::SILENT);
                    }
                }
            }
            if(task->m_appinfo->isFlagSet(appinfo_t::ONLY_SILENT))
                task->set(task_t::SILENT);

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
        f.info->ParseRegistryInfo();
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
	    f.info->ParseRegistryInfo();
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
        on_btUpdateInfo_clicked();
    }else
    {
        UpdaterDialog *dlg = new UpdaterDialog(fileinfo_list,UpdaterDialog::update_check_appinfo);
        emit status("checking for new info");
        qDebug("checking for new info");
        connect(dlg,SIGNAL(accepted()),SLOT(onNewAppInfoAvailable()));
        dlg->setAttribute(Qt::WA_DeleteOnClose,true);
        dlg->exec();
    }
    if( (SettingsDialog::shouldCheckVersions() && SettingsDialog::lastVersionUpdate() < QDate::currentDate())
            && !fileinfo_list.isEmpty())
    {
        QTimer::singleShot(1000,this,SLOT(on_btUpdate_clicked()));
        SettingsDialog::setLastVersionUpdate(QDate::currentDate());
    }
}

void applist_t::onNewAppInfoAvailable()
{
    qDebug("new info");
    info_updates_avail = true;
    ui->btUpdateInfo->setIcon(QIcon(":icons/information.ico"));
    ui->btUpdateInfo->raise();
    emit status("new info is available");
    if(SettingsDialog::isSetDownloadPackages())
    {
        on_btUpdateInfo_clicked();
    }
}

void applist_t::onRemovedFromTasks(appinfo_t *info)
{
    info->unSetFlag(appinfo_t::SELECTED_INST_DL);
    info->unSetFlag(appinfo_t::SELECTED_REM);
    QList<QTreeWidgetItem*> items = ui->LAppInfoList->findItems(info->Name,Qt::MatchContains);
    for(int i=0;i<items.count();i++)
    {
        QTreeWidgetItem *item = items[i];
        applist_t::fileinfo_t data = item->data(0,QTreeWidgetItem::UserType).value<applist_t::fileinfo_t>();
        appinfo_t *item_info = data.info;
        if(item_info==info)
        {
            item->setIcon(0,info->getIcon());
            if( info->isFlagSet(appinfo_t::NO_INFO) && !info->isFlagSet(appinfo_t::INSTALLED) )
            {
                delete item;
                for(int a=0;a<fileinfo_list.count();a++)
                {
                    if( fileinfo_list.at(a).info == info )
                    {
                        fileinfo_list.removeAt(a);
                        delete info;
                        break;
                    }
                }
                return;
            }
        }
    }
}

void applist_t::on_btUpdate_clicked()
{
    if( fileinfo_list.count()>0 )
    {
        QList<fileinfo_t> list;
        for(int i=0;i<fileinfo_list.count();i++)
        {
            fileinfo_t &f = (fileinfo_t&)fileinfo_list.at(i);
            if( f.info->isFlagSet(appinfo_t::DOWNLOADED) ||
                    (f.info->isFlagSet(appinfo_t::INSTALLED) && !f.info->isFlagSet(appinfo_t::NO_INFO)) )
                list.append(f);
        }
        if(list.count()==0)
            list = fileinfo_list;
        UpdaterDialog dlg(list,UpdaterDialog::update_version);
        dlg.move( mapToGlobal( this->rect().center()/2 ));
        if(!this->isVisible())
            dlg.showMinimized();
        dlg.exec();
        QList<QTreeWidgetItem*> items = ui->TCategoryTree->findItems("Updates",Qt::MatchExactly);
        if(items.count()==0)
            return;
        QTreeWidgetItem *item = items[0];

        ui->TCategoryTree->setCurrentItem(item);

        ui->LAppInfoList->clear();
        setListByCategory("Updates");
        if(ui->LAppInfoList->topLevelItemCount()>0)
            emit versions_available();
    }
}

void applist_t::on_btCheckUpdates_clicked()
{
    QList<QTreeWidgetItem*> items = ui->TCategoryTree->findItems("Updates",Qt::MatchExactly);
    if(items.count()==0)
        return;
    QTreeWidgetItem *item = items[0];
    ui->TCategoryTree->setCurrentItem(item,0,QItemSelectionModel::ClearAndSelect);

    for(int i=0;i<ui->LAppInfoList->topLevelItemCount();i++)
    {
        item = ui->LAppInfoList->topLevelItem(i);
        applist_t::fileinfo_t data = item->data(0,QTreeWidgetItem::UserType).value<applist_t::fileinfo_t>();
        appinfo_t *info = data.info;
        if(!info->isFlagSet(appinfo_t::SELECTED_INST_DL))
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
            ui->btUpdateInfo->setIcon(QIcon(":icons/isnew.ico"));
            saveList();
            QList<QTreeWidgetItem*> items = ui->TCategoryTree->findItems("New/Updated",Qt::MatchExactly);
            if(items.count()==0)
                return;
            QTreeWidgetItem *item = items[0];
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
