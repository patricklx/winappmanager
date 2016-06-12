#include <QMessageBox>
#include <QMenu>
#include <QFile>
#include <QDir>
#include <QDesktopServices>
#include <QTimer>
#include <QDomDocument>
#include <QDir>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QGraphicsBlurEffect>
#include <settingsdialog.h>
#include <QItemDelegate>
#include <QFontMetrics>
#include "applist.h"
#include "gui/ui_applist.h"
#include "choosedialog.h"
#include "updaterdialog.h"
#include "appinfo_registry.h"
#include "yaml-cpp/yaml.h"
#include "utils/qstringext.h"

Q_DECLARE_METATYPE(AppList::Fileinfo)

class MyItemDelegate : public QItemDelegate
{

	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		QStyleOptionViewItem optionStyle(option);
		QStyle *style = optionStyle.widget ? optionStyle.widget->style() : QApplication::style();
		style->drawPrimitive(QStyle::PE_PanelItemViewItem, &optionStyle, painter, optionStyle.widget);

		QStyleOptionViewItem o = option;
		o.state = o.state&~QStyle::State_Selected;
		QItemDelegate::paint(painter,o,index);
	}

	void drawFocus(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect) const{}

	void drawDisplay(QPainter *fp_painter, const QStyleOptionViewItem &f_option, const QRect &f_rect, const QString &f_text ) const
	{
		QStringList str = f_text.split(";");
		QPen pen ( fp_painter->pen ( ) );
		QFont font ( fp_painter->font ( ) );

		// This does not work; perhaps "setWrapMode" does change the alignment
		//    QTextOption textOption ( f_option.decorationAlignment );
		//    textOption.setWrapMode ( QTextOption::WordWrap );

		// This does work...
		QTextOption textOption;
		textOption.setAlignment ( f_option.displayAlignment );
		textOption.setWrapMode(QTextOption::NoWrap);


		QFont tfont = f_option.font;
		tfont.setPointSize(tfont.pointSize()+1);
		QFontMetrics metrics(tfont);
		QRect textRect = f_rect.adjusted ( 5, 0, -5, 0 ); // remove width padding
		textRect.setTop ( qMin ( f_rect.top ( ), f_option.rect.top ( ) )+5 );
		textRect.setHeight ( metrics.height() );


		fp_painter->setFont ( tfont );
		fp_painter->drawText ( textRect, str[0], textOption );


		textRect.setTop( textRect.top() +  metrics.height()-5);

		tfont.setPointSize(tfont.pointSize()-2);
		metrics = QFontMetrics(tfont);
		textRect.setHeight(metrics.height()+3);


		fp_painter->setFont(tfont);
		fp_painter->setPen(Qt::blue);
		fp_painter->drawText ( textRect, str[1], textOption );

		// Painter zurücksetzen
		fp_painter->setFont ( font );
		fp_painter->setPen ( pen );


	}


	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		QFont tfont = option.font;
		tfont.setBold(true);
		QFontMetrics metrics(tfont);
		int y = metrics.height();

		tfont.setBold(false);
		metrics = QFontMetrics(tfont);
		y += metrics.height();
		return QSize(-1,y+5);
	}


};


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

AppList::AppList(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::AppList)
{
	ui->setupUi(this);
	ui->startTasksBtn->hide();
	ui->groupBox->hide();
	this->setEnabled(false);
	searching = false;
	stop_searching = false;
	version_updates_avail = false;
	ui->LAppInfoList->sortItems(0,Qt::AscendingOrder);
	ui->LAppInfoList->header()->setSectionResizeMode(0,QHeaderView::Stretch);
	ui->LAppInfoList->setItemDelegateForColumn(0,new MyItemDelegate());



	QTreeWidgetItem *item = new QTreeWidgetItem(QStringList("All"));
	ui->TCategoryTree->addTopLevelItem(item);
	timer.setSingleShot(true);

	connect(this,SIGNAL(taskChosen(Task*)),ui->taskList,SLOT(addTask(Task*)));
	connect(this,SIGNAL(unSelected(AppInfo*)),ui->taskList,SLOT(removeTask(AppInfo*)));
	connect(ui->taskList,SIGNAL(onTaskRemoved(AppInfo*)),this,SLOT(onRemovedFromTasks(AppInfo*)));
	connect(ui->taskList,SIGNAL(updateAppInfo(AppInfo*)),this,SLOT(updateItem(AppInfo*)));
	connect(ui->LAppInfoList,SIGNAL(viewportChanged()),this,SLOT(drawButtons()));
	connect(&timer,SIGNAL(timeout()),this,SLOT(timedDrawButtons()));

	QTimer::singleShot(100,this,SLOT(load()));

	if( SettingsDialog::value("INSTALL_MODE").toInt() == SettingsDialog::Silent )
	{
		ui->automaticRb->setChecked(true);
	}else{
		ui->manualRb->setChecked(true);
	}
}

AppList::~AppList()
{
	qDebug()<<"in destructor";
	clear();
	qDebug()<<"deleting UI";
	delete ui;
}

void AppList::on_TCategoryTree_itemSelectionChanged()
{
	QTreeWidgetItem *item = ui->TCategoryTree->currentItem();
	ui->LAppInfoList->clear();
	setListByCategory(item->text(0));
}

void AppList::on_LAppInfoList_itemSelectionChanged()
{
	QTreeWidgetItem *item = ui->LAppInfoList->currentItem();
	AppList::Fileinfo info = item->data(0,QTreeWidgetItem::UserType).value<AppList::Fileinfo>();
	ui->AppInfoText->clear();
	ui->AppInfoText->moveCursor(QTextCursor::End);/*workaround for QTBUG-539*/

	ui->AppInfoText->append(info.name);
	ui->AppInfoText->append("<a href='"+info.appInfo->webpage()+"'>"+info.appInfo->webpage()+"</a>");
	ui->AppInfoText->append("<b>latest version:</b> "+info.appInfo->latestVersion());
	ui->AppInfoText->append("<b>installed version:</b> "+info.appInfo->registryInfo().version);
	ui->AppInfoText->append("<b>Downloaded version:</b> "+info.appInfo->getUserData().downloadedVersion+"\n");

	ui->AppInfoText->append(info.appInfo->description()+"\n");
	ui->AppInfoText->append(info.appInfo->info());

	ui->AppInfoText->moveCursor(QTextCursor::Start);

}


bool actionExists(QMenu *m,QString action)
{
	QList<QAction*> list = m->actions();
	for(int i=0;i<list.count();i++)
	{
		if(list.at(i)->text()==action)
			return true;
	}
	return false;
}

void AppList::on_LAppInfoList_customContextMenuRequested(const QPoint &pos)
{
	QList<QTreeWidgetItem*> list = ui->LAppInfoList->selectedItems();
	QTreeWidgetItem *item = NULL;
	if(list.isEmpty())
	{
		item= ui->LAppInfoList->itemAt(pos);
		if(item==NULL)
			return;
		list.append(item);
	}
	if(list.count()==1)
		item=list[0];



	QMenu menu;
	for(int i=0;i<list.count();i++)
	{
		item = list[i];
		AppList::Fileinfo data = item->data(0,QTreeWidgetItem::UserType).value<AppList::Fileinfo>();
		AppInfo *info = data.appInfo;

		if(info->isFlagSet(AppInfo::NoInfo))
		{
			if(actionExists(&menu,"Uninstall")) menu.addAction("Uninstall");
		}else
		{
			if(info->isFlagSet(AppInfo::SelectedInstallDownload) || info->isFlagSet(AppInfo::SelectedRemove))
				menu.addAction("Unselect");
			else
				menu.addAction("Select");
			menu.addAction("Check Latest Version");
			menu.addSeparator();

			if( info->isFlagSet(AppInfo::UpdateAvailable) )
				menu.addAction("Update");


			if( info->isFlagSet(AppInfo::Installed) )
				menu.addAction("Uninstall");


			if( info->isFlagSet(AppInfo::Installed) && info->isFlagSet(AppInfo::Downloaded) )
				menu.addAction("ReInstall");

			menu.addSeparator();
			if( info->isFlagSet(AppInfo::Downloaded) )
			{
				if(info->isFlagSet(AppInfo::NoRegistry))
					menu.addAction("Execute program");
				else
					menu.addAction("Execute installer");
				menu.addAction("Delete downloaded File");
				menu.addAction("Open containing Folder");
			}
			menu.addSeparator();
			if( info->isFlagSet(AppInfo::Installed) && info->isFlagSet(AppInfo::UpdateAvailable) )
				menu.addAction("Set to latest version");
			if( info->isFlagSet(AppInfo::Installed) || info->isFlagSet(AppInfo::Downloaded) )
			{
				if(info->isFlagSet(AppInfo::IgnoreLatest))
					menu.addAction("unset ignore newer versions");
				else
					menu.addAction("set ignore newer versions");
			}
			menu.addSeparator();

			if(info->version().url.contains("filehippo") )
			{
				if(info->isFlagSet(AppInfo::FilehippoBeta))
					menu.addAction("disable Beta");
				if(!info->isFlagSet(AppInfo::FilehippoBeta))
					menu.addAction("enable Beta");
			}

			if(list.count()==1)
			{
				menu.addSeparator();
				menu.addAction("report a problem");
			}
		}
	}
	/***************************************************
	 Execute menu
	 ***************************************************/
	menu.move(ui->LAppInfoList->mapToGlobal(pos+QPoint(0,25)));
	QAction *action = menu.exec();
	if(action==NULL)
		return;

	for(int i=0;i<list.count();i++)
	{
		item = list[i];
		AppList::Fileinfo data = item->data(0,QTreeWidgetItem::UserType).value<AppList::Fileinfo>();
		AppInfo *info = data.appInfo;

		if( action->text() == "Select")
		{
			ChooseDialog dialog(info,this);
			Task *task = dialog._exec();
			if(task!=NULL)
			{
				emit taskChosen(task);
				info->setFlag(AppInfo::SelectedInstallDownload);
				item->setIcon(0,info->getIcon());
			}
		}
		if( action->text() == "Unselect")
		{
			emit unSelected(info);
			info->unSetFlag(AppInfo::SelectedInstallDownload);
			item->setIcon(0,info->getIcon());
		}

		if( action->text() == "Check Latest Version" )
		{
			qDebug("checking version");
			emit status(tr("checking version for :")+info->name());
			connect(info,SIGNAL(versionUpdated(AppInfo*,bool)),SLOT(onVersionUpdated(AppInfo*,bool)));
			info->updateVersion();
		}

		if( action->text() == "Update" )
		{
			Task *task = new Task(info,Task::DOWNLOAD|Task::INSTALL);
			emit taskChosen(task);
			info->setFlag(AppInfo::SelectedRemove);
			item->setIcon(0,info->getIcon());
		}

		if( action->text() == "Uninstall")
		{
			Task *task = new Task(info,Task::UNINSTALL);
			emit taskChosen(task);
			info->setFlag(AppInfo::SelectedRemove);
			item->setIcon(0,info->getIcon());
		}
		if( action->text() == "ReInstall")
		{
			Task *task = new Task(info,Task::INSTALL);
			if(!task->appInfo()->getInstallInfo().silentParameter.isEmpty()
					&& !task->appInfo()->getInstallInfo().onlySilent)
			{
				switch(SettingsDialog::value("INSTALL_MODE").toInt())
				{
				case SettingsDialog::Ask:
				{
					int ans = QMessageBox::question(this,
													tr("Install Mode"),
													tr("%1 :Install in silent mode?").arg(info->name()),
													QMessageBox::Yes|QMessageBox::No);
					if(ans == QMessageBox::Yes)
						task->set(Task::SILENT);
					break;
				}
				case SettingsDialog::Silent:
					task->set(Task::SILENT);
				}
			}
			emit taskChosen(task);
			info->setFlag(AppInfo::SelectedInstallDownload);
			item->setIcon(0,info->getIcon());
		}

		if( action->text() == "Execute installer" || action->text() == "Execute program")
		{
			QDesktopServices::openUrl(tr("file:///%1").arg(info->path()+"/"+info->fileName()));
		}
		if( action->text() == "Open containing Folder")
		{
			QDesktopServices::openUrl(tr("file:///%1").arg(info->path()));
		}
		if( action->text() == "Delete downloaded File")
		{
			info->removeDownloadedFile();
			updateItem(info);
		}

		if( action->text() == "Set to latest version")
		{
			if( !info->forceRegistryToLatestVersion() )
				QMessageBox::information(this,"setting registry value","failed to force latest version , try again as admin!");
			else
				updateItem(info);
		}
		if( action->text() == "set ignore newer versions")
		{
			info->ignoreNewerVersions();
			updateItem(info);
		}
		if( action->text() == "unset ignore newer versions")
		{
			info->ignoreNewerVersions(false);
			updateItem(info);
		}

		if( action->text() == "disable Beta")
		{
			info->setFilehippoBeta(false);
		}

		if( action->text() == "enable Beta")
		{
			info->setFilehippoBeta(true);
		}

		if( action->text() == "report a problem")
		{
			QDesktopServices::openUrl("https://winappmanager.herokuapp.com/apps/"+info->name());
		}
	}
}

void AppList::updateItem(AppInfo *appinfo)
{
	QList<QTreeWidgetItem*> items = ui->LAppInfoList->findItems(appinfo->name()+";",Qt::MatchStartsWith);
	QTreeWidgetItem *item = NULL;
	for(int i=0;i<items.count();i++)
	{
		item = items[i];
		AppInfo *item_info = getAppInfo(item);
		if(item_info==appinfo)
			break;
	}
	if(item==NULL){
		return;
	}

	item->setIcon(0,appinfo->getIcon());

	if( item->isSelected() )
		on_LAppInfoList_itemSelectionChanged();
}

void AppList::onVersionUpdated(AppInfo *appinfo, bool updated)
{
	QString msg;
	if(updated)
		msg = tr("succesfuly retrieved latest version for %1 :\n -> %2").arg(appinfo->name(),appinfo->latestVersion());
	else
		msg = "failed to retrieve latest version for " + appinfo->name()+"\n"+appinfo->lastError();
	QMessageBox::information(this,"Version updated",msg);
	disconnect(appinfo,SIGNAL(versionUpdated(AppInfo*,bool)),this,SLOT(onVersionUpdated(AppInfo*,bool)));
}

void AppList::clear()
{
	ui->LAppInfoList->clear();
	ui->TCategoryTree->clear();
	while( !fileinfo_list.isEmpty() )
	{
		Fileinfo fileinfo = fileinfo_list.takeFirst();
		emit unSelected(fileinfo.appInfo);
		delete fileinfo.appInfo;
	}
}



void AppList::loadCategoryTree(QDomElement node,QTreeWidgetItem *item)
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


void AppList::setRegistryInfo(RegistryGroup group)
{
	QString name = group.displayed_name;
	QString groupName = group.group_name;
	ui->progressBar->setValue(ui->progressBar->value()+1);
	if(name.isEmpty())
		return;

	qDebug()<<"registry app: "<<name;

	foreach(Fileinfo fileInfo,fileinfo_list)
	{

		if(fileInfo.appInfo->isFlagSet(AppInfo::NoRegistry)){
			qDebug()<<"No registry: "<<fileInfo.appInfo->name();
			continue;
		}

		if(name.contains(fileInfo.appInfo->registry().searchKey, Qt::CaseInsensitive ) ||
				groupName.contains(fileInfo.appInfo->registry().searchKey, Qt::CaseInsensitive))
		{
			if(!fileInfo.appInfo->registryInfo().path.isEmpty())
			{
				if(fileInfo.appInfo->registryInfo().displayName==group.displayed_name)
				{
					qDebug()<<"already in list";
					return;
				}
				else
					break;
			}

			qDebug()<<"found in list: " << fileInfo.name;
			fileInfo.appInfo->setRegistryInfo(&group);
			return;
		}
	}

	if(!SettingsDialog::value("SHOW_ALL_APPS").toBool())
		return;

	qDebug()<<"added " <<group.displayed_name;
	Fileinfo fileinfo;
	fileinfo.name = group.displayed_name;
	fileinfo.appInfo = new AppInfo(group.displayed_name);
	fileinfo.appInfo->setRegistryInfo(&group);
	fileinfo.appInfo->setFlag(AppInfo::NoInfo);
	connect(fileinfo.appInfo,SIGNAL(infoUpdated(AppInfo*)),SLOT(updateItem(AppInfo*)));
	fileinfo_list.append(fileinfo);
}


void AppList::addCategory(QStringList categorylist)
{
	QTreeWidgetItem *parent=ui->TCategoryTree->invisibleRootItem();
	for(int i=0;i<categorylist.count();i++)
	{
		QString category = categorylist[i];
		QTreeWidgetItem *child = NULL;
		for(int a=0;a<parent->childCount();a++)
		{
			if(parent->child(a)->text(0)==category)
			{
				child = parent->child(a);
				break;
			}
		}
		if(child==NULL)
		{
			child = new QTreeWidgetItem(parent);
			child->setText(0,category);
		}
		parent = child;
	}
}

void AppList::loadFiles()
{
	QDir dir("Info");
	QStringList filters; filters << "*.yaml" << "*.yml";
	QStringList files = dir.entryList(filters,QDir::Files,QDir::Name);

	foreach(QString filename,files)
	{
		Fileinfo fileinfo;
		fileinfo.name = QStringExt(filename).beforeLast(".");
		qDebug()<<"load: "<<fileinfo.name;


		if( !fileinfo.name.isEmpty() )
		{
			fileinfo.appInfo = new AppInfo(fileinfo.name);
			bool ok = fileinfo.appInfo->load();
			if( ok )
			{
				fileinfo.name = fileinfo.appInfo->name();
				connect(fileinfo.appInfo,SIGNAL(infoUpdated(AppInfo*)),SLOT(updateItem(AppInfo*)));
				addCategory(fileinfo.appInfo->categories());
			}
			fileinfo_list.append(fileinfo);
		}
	}
}

bool AppList::load()
{
	clear();
	ui->progressBar->show();
	ui->progressBar->setValue(0);
	QApplication::processEvents();

	if(!SettingsDialog::updatedToLatestVersion())
	{
		qDebug()<<"update to latest version";
		QDir dir("Info");
		dir.removeRecursively();
		QTimer::singleShot(500,this,SLOT(on_btUpdateInfo_clicked()));
		SettingsDialog::setUpdatedToLatestVersion();
		return false;
	}


	loadFiles();

	qDebug()<<"prepare UI";
	prepareUi();
	qDebug()<<"prepare UI done";

	qDebug()<<fileinfo_list.count();
	QDateTime datetime(QDate::currentDate());
	qDebug() << "will update?" << datetime.toMSecsSinceEpoch()/1000;
	if( (SettingsDialog::value("CHECK_INFO").toBool() &&
		 SettingsDialog::value("LAST_INFO_CHECK").toInt() < datetime.toMSecsSinceEpoch()/1000)
			|| fileinfo_list.isEmpty())
	{
		qDebug()<<"list is empty-> download";
		QTimer::singleShot(500,this,SLOT(on_btUpdateInfo_clicked()));
		return false;
	}

    qDebug()<<"CHECK_VERSION" << SettingsDialog::value("CHECK_VERSION").toBool();
    qDebug()<<"CHECK_VERSION" << SettingsDialog::value("CHECK_VERSION");
	if( SettingsDialog::value("CHECK_VERSION").toBool() &&
			SettingsDialog::value("LAST_VERSION_CHECK").toDate() < QDate::currentDate() )
	{
		QTimer::singleShot(500,this,SLOT(on_btUpdate_clicked()));
		SettingsDialog::setValue("LAST_VERSION_CHECK",QDate::currentDate());
	}

	AppinfoRegistry reginfo;
	qDebug()<<"prepare progress";
	ui->progressBar->setMaximum(reginfo.getCount());

	qDebug()<< "registry:";
	qRegisterMetaType<RegistryGroup>();
	connect(&reginfo,SIGNAL(registry_app_info(RegistryGroup)),SLOT(setRegistryInfo(RegistryGroup)));
	reginfo.getAppsFromRegistry();
	qDebug()<<"got apps from registry";

	qDebug()<< "registry end";
	qSort(fileinfo_list.begin(),fileinfo_list.end());
	this->setEnabled(true);

	return true;
}

void AppList::prepareUi()
{
	ui->progressBar->hide();
	ui->lbLoad->hide();

	QTreeWidgetItem *item;
	item = new QTreeWidgetItem(QStringList("Updates"));
	item->setIcon(0,QIcon(":/icons/hasupdate.ico"));
	ui->TCategoryTree->addTopLevelItem(item);
	item = new QTreeWidgetItem(QStringList("Installed"));
	item->setIcon(0,QIcon(":/icons/IsInstalled.ico"));
	ui->TCategoryTree->addTopLevelItem(item);
	item = new QTreeWidgetItem(QStringList("Downloaded"));
	item->setIcon(0,QIcon(":/icons/IsDL.ico"));
	ui->TCategoryTree->addTopLevelItem(item);
	item = new QTreeWidgetItem(QStringList("New/Updated"));
	item->setIcon(0,QIcon("://icons/isnew.ico"));
	ui->TCategoryTree->addTopLevelItem(item);
	item = new QTreeWidgetItem(QStringList("Only Installed"));
	item->setIcon(0,QIcon("://icons/IsInstalled.ico"));
	ui->TCategoryTree->addTopLevelItem(item);
	item = new QTreeWidgetItem(QStringList("Only Downloaded"));
	item->setIcon(0,QIcon("://icons/IsDL.ico"));
	ui->TCategoryTree->addTopLevelItem(item);
	item = new QTreeWidgetItem(QStringList("Only Info"));
	item->setIcon(0,QIcon("://icons/isnew.ico"));
	ui->TCategoryTree->addTopLevelItem(item);

	if(SettingsDialog::value("SHOW_ALL_APPS").toBool())
	{
		item = new QTreeWidgetItem(QStringList("all installed apps"));
		ui->TCategoryTree->addTopLevelItem(item);
	}

	ui->TCategoryTree->setCurrentItem(ui->TCategoryTree->topLevelItem(0),0,QItemSelectionModel::Select);


}


QWidget* AppList::createActionButtons(QTreeWidgetItem *item)
{
	AppInfo* info = getAppInfo(item);
	if( info->isFlagSet(AppInfo::UpdateAvailable) )
	{
		QPushButton* button = new QPushButton("Upgrade");
		button->setIcon(QIcon(":/icons/hasupdate.ico"));
		button->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Minimum);
		button->setFlat(true);
		button->setStyleSheet("text-align:left;");
		button->setCheckable(true);
		if( info->isFlagSet(AppInfo::SelectedInstallDownload) ){
			button->setChecked(true);
		}
		connect(button,SIGNAL(clicked()),SLOT(onButtonUpgradeClicked()));
		return button;
	}

	if( info->isFlagSet(AppInfo::Installed) )
	{
		QPushButton* button = new QPushButton("Remove");
		button->setIcon(QIcon(":/icons/Remove.ico"));
		button->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Minimum);
		button->setFlat(true);
		button->setStyleSheet("text-align:left;");
		button->setCheckable(true);
		if( info->isFlagSet(AppInfo::SelectedRemove) ){
			button->setChecked(true);
		}
		connect(button,SIGNAL(clicked()),SLOT(onButtonRemoveClicked()));
		return button;
	}

	if( info->isFlagSet(AppInfo::OnlyInfo) || info->isFlagSet(AppInfo::Downloaded) )
	{
		QPushButton* button = new QPushButton("Install");
		button->setIcon(QIcon(":/icons/Download.ico"));
		button->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Minimum);
		button->setFlat(true);
		button->setStyleSheet("text-align:left;");
		button->setCheckable(true);
		if( info->isFlagSet(AppInfo::SelectedInstallDownload) ){
			button->setChecked(true);
		}
		connect(button,SIGNAL(clicked()),SLOT(onButtonInstallClicked()));
		return button;
	}


	return NULL;
}

void AppList::appendToList(Fileinfo &info)
{
	QStringList columnInfo;
	TreeWidgetItem *item = new TreeWidgetItem(columnInfo);
	item->setText(0,info.name+";"+info.appInfo->description());
	item->setIcon(0,info.appInfo->getIcon());
	item->setToolTip(0,info.appInfo->name()+"\n"+info.appInfo->description());
	item->setData(0,QTreeWidgetItem::UserType,qVariantFromValue(info));

	ui->LAppInfoList->addTopLevelItem(item);
}

void AppList::setListByCategory(QString cat)
{
	AppInfo::Flags flag = AppInfo::Invalid;

	ui->AppInfoText->show();
	ui->LAppInfoList->setColumnCount(2);
	ui->LAppInfoList->setColumnWidth(2,50);

	ui->stackedWidget->setCurrentIndex(1);
	ui->startTasksBtn->hide();
	ui->btnTask->show();
	ui->groupBox->hide();
	ui->lbSearch->show();

	if( cat == "All" )
	{
		for(int i=0;i<fileinfo_list.count();i++)
		{
			Fileinfo &f = (Fileinfo&)fileinfo_list.at(i);
			if(f.appInfo->isFlagSet(AppInfo::NoInfo))
				continue;
			appendToList(f);
		}
		goto end;
	}

	if( cat == "Only Downloaded" )
	{
		for(int i=0;i<fileinfo_list.count();i++)
		{
			Fileinfo &f = (Fileinfo&)fileinfo_list.at(i);
			if(f.appInfo->isFlagSet(AppInfo::NoInfo))
				continue;
			if( f.appInfo->isFlagSet(AppInfo::Downloaded) && !f.appInfo->isFlagSet(AppInfo::Installed) )
				appendToList(f);
		}
		goto end;
	}
	if( cat == "Only Installed" )
	{
		for(int i=0;i<fileinfo_list.count();i++)
		{
			Fileinfo &f = (Fileinfo&)fileinfo_list.at(i);
			if(f.appInfo->isFlagSet(AppInfo::NoInfo))
				continue;
			if( f.appInfo->isFlagSet(AppInfo::Installed) && !f.appInfo->isFlagSet(AppInfo::Downloaded) )
				appendToList(f);
		}
		goto end;
	}
	if( cat == "New/Updated" )
		flag = AppInfo::New;

	if( cat == "Updates" )
		flag = AppInfo::UpdateAvailable;

	if( cat == "Downloaded" )
		flag = AppInfo::Downloaded;

	if( cat == "Installed" )
		flag = AppInfo::Installed;

	if( cat == "Only Info" )
		flag = AppInfo::OnlyInfo;

	if( cat == "all installed apps" )
		flag = AppInfo::NoInfo;

	if( flag != AppInfo::Invalid )
	{
		for(int i=0;i<fileinfo_list.count();i++)
		{
			Fileinfo &f = (Fileinfo&)fileinfo_list.at(i);
			if(flag!=AppInfo::NoInfo && f.appInfo->isFlagSet(AppInfo::NoInfo))
				continue;

			if( f.appInfo->isFlagSet(flag) )
			{
				appendToList(f);
				continue;
			}

			if( flag == AppInfo::NoInfo && f.appInfo->isFlagSet(AppInfo::Installed) )
				appendToList(f);
		}

		if(flag==AppInfo::NoInfo)
		{
			ui->AppInfoText->hide();
			ui->LAppInfoList->setColumnCount(2);
		}

		if(flag == AppInfo::UpdateAvailable && ui->LAppInfoList->topLevelItemCount()>0)
			version_updates_avail = true;
		else
			version_updates_avail = false;

		goto end;
	}

	for(int i=0;i<fileinfo_list.count();i++)
	{
		Fileinfo &f = (Fileinfo&)fileinfo_list.at(i);
		if( f.appInfo->categories().contains(cat) )
			appendToList(f);
	}

end:
	drawButtons();
}

void AppList::updateVersions()
{
}

bool AppList::tasksRunning()
{
	return !ui->taskList->isEmpty();
}


void AppList::onRemovedFromTasks(AppInfo *info)
{
	info->unSetFlag(AppInfo::SelectedInstallDownload);
	info->unSetFlag(AppInfo::SelectedRemove);
	QList<QTreeWidgetItem*> items = ui->LAppInfoList->findItems(info->name()+";",Qt::MatchStartsWith);
	for(int i=0;i<items.count();i++)
	{
		QTreeWidgetItem *item = items[i];
		AppList::Fileinfo data = item->data(0,QTreeWidgetItem::UserType).value<AppList::Fileinfo>();
		AppInfo *item_info = data.appInfo;
		if(item_info==info)
		{
			item->setIcon(0,info->getIcon());
			if( info->isFlagSet(AppInfo::NoInfo) && !info->isFlagSet(AppInfo::Installed) )
			{
				delete item;
				for(int a=0;a<fileinfo_list.count();a++)
				{
					if( fileinfo_list.at(a).appInfo == info )
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

void AppList::on_btUpdate_clicked()
{
	if( fileinfo_list.count()>0 )
	{
		QList<Fileinfo> list;
		for(int i=0;i<fileinfo_list.count();i++)
		{
			Fileinfo &f = (Fileinfo&)fileinfo_list.at(i);
			if( f.appInfo->isFlagSet(AppInfo::Downloaded) ||
					(f.appInfo->isFlagSet(AppInfo::Installed) && !f.appInfo->isFlagSet(AppInfo::NoInfo)) )
				list.append(f);
		}
		if(list.count()==0)
			list = fileinfo_list;
		UpdaterDialog dlg(this,list,UpdaterDialog::UpdateVersion);
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

void AppList::on_btUpgrade_clicked()
{
	QList<QTreeWidgetItem*> items = ui->TCategoryTree->findItems("Updates",Qt::MatchExactly);
	if(items.count()==0)
		return;
	QTreeWidgetItem *item = items[0];
	ui->TCategoryTree->setCurrentItem(item,0,QItemSelectionModel::ClearAndSelect);

	for(int i=0;i<ui->LAppInfoList->topLevelItemCount();i++)
	{
		item = ui->LAppInfoList->topLevelItem(i);
		upgrade(item);
	}

	if( ui->LAppInfoList->topLevelItemCount()==0 ){
		return;
	}

	ui->btnTask->click();
}

QTreeWidgetItem *AppList::getCurrentItem()
{
	QPoint p = ui->LAppInfoList->mapFromGlobal(QCursor::pos()-QPoint(0,20));
	QTreeWidgetItem *item = ui->LAppInfoList->itemAt(p);

	if(item==NULL){
		item = ui->LAppInfoList->selectedItems().first();
	}

	return item;
}

AppInfo *AppList::getAppInfo(QTreeWidgetItem *item)
{
	AppList::Fileinfo data = item->data(0,QTreeWidgetItem::UserType).value<AppList::Fileinfo>();
	AppInfo *info = data.appInfo;
	return info;
}

void AppList::onButtonUpgradeClicked()
{
	QPushButton *button = (QPushButton*)sender();
	if( button == NULL )
		return;

	QTreeWidgetItem* item = getCurrentItem();
	if(item==NULL){
		return;
	}
	if( ui->LAppInfoList->itemWidget(item,1) != button ){
		return;
	}
	if( button->isChecked() )
	{
		upgrade(item);
	}else
	{
		deselect(item);
	}
}

void AppList::onButtonInstallClicked()
{
	QPushButton *button = (QPushButton*)sender();
	if( button == NULL ){
		qDebug()<<"button NULL";
		return;
	}

	QTreeWidgetItem* item = getCurrentItem();
	if( ui->LAppInfoList->itemWidget(item,1) != button ){
		qDebug()<<"not same widget";
		return;
	}

	if( button->isChecked() )
	{
		install(item);
	}else{
		deselect(item);
	}
}

void AppList::onButtonRemoveClicked()
{
	QPushButton *button = (QPushButton*)sender();
	if( button == NULL )
		return;

	QTreeWidgetItem* item = getCurrentItem();
	if( ui->LAppInfoList->itemWidget(item,1) != button ){
		return;
	}

	if( button->isChecked() )
	{
		uninstall(item);
	}else
	{
		deselect(item);
	}
}

void AppList::upgrade(QTreeWidgetItem* item)
{
	AppInfo *info = getAppInfo(item);
	if(info->isFlagSet(AppInfo::SelectedInstallDownload))
		return;

	if(info->getUserData().downloadedVersion==info->latestVersion()){
		install(item);
		return;
	}
	if(info->getDownloadedFileUrl().url.isEmpty()){
		ChooseDialog dialog(info,this);
		Task *task = dialog._exec();
		if(task!=NULL)
		{
			emit taskChosen(task);
			info->setFlag(AppInfo::SelectedInstallDownload);
			item->setIcon(0,info->getIcon());
		}
		return;
	}
	Task *task = new Task(info,Task::DOWNLOAD|Task::INSTALL|Task::SILENT);
	emit taskChosen(task);
	info->setFlag(AppInfo::SelectedInstallDownload);
	item->setIcon(0,info->getIcon());
	QPushButton* button = (QPushButton*)ui->LAppInfoList->itemWidget(item,1);
	if(button) button->setChecked(true);
}

void AppList::install(QTreeWidgetItem* item)
{
	AppInfo *info = getAppInfo(item);
	ChooseDialog dialog(info,this);
	Task *task = dialog._exec();
	if(task!=NULL)
	{
		emit taskChosen(task);
		info->setFlag(AppInfo::SelectedInstallDownload);
		item->setIcon(0,info->getIcon());
		QPushButton* button = (QPushButton*)ui->LAppInfoList->itemWidget(item,1);
		if(button) button->setChecked(true);
	}else{
		QPushButton* button = (QPushButton*)ui->LAppInfoList->itemWidget(item,1);
		if(button) button->setChecked(false);
	}
}

void AppList::uninstall(QTreeWidgetItem* item)
{
	AppInfo *info = getAppInfo(item);
	Task *task = new Task(info,Task::UNINSTALL);
	emit taskChosen(task);
	info->setFlag(AppInfo::SelectedRemove);
	item->setIcon(0,info->getIcon());
}

void AppList::deselect(QTreeWidgetItem* item)
{
	AppInfo *info = getAppInfo(item);
	emit unSelected(info);
	info->unSetFlag(AppInfo::SelectedInstallDownload);
	item->setIcon(0,info->getIcon());
}

void AppList::drawButtons()
{
	timer.start(50);
}

void AppList::timedDrawButtons()
{
	QTreeWidgetItem *item = ui->LAppInfoList->itemAt(0,0);
	if( item==NULL ){
		return;
	}

	while ( ui->LAppInfoList->itemBelow(item))
	{
		if( ui->LAppInfoList->itemWidget(item,1) == NULL)
			ui->LAppInfoList->setItemWidget(item,1,createActionButtons(item));

		QRect rect = ui->LAppInfoList->visualItemRect(ui->LAppInfoList->itemBelow(item));
		if ( ui->LAppInfoList->viewport()->rect().intersects( rect ))
		{
			item = ui->LAppInfoList->itemBelow(item);
		}else{
			break;
		}
	}
	if( ui->LAppInfoList->itemWidget(item,1) == NULL)
		ui->LAppInfoList->setItemWidget(item,1,createActionButtons(item));
}



void AppList::on_btUpdateInfo_clicked()
{
	UpdaterDialog::DownloadType flag = this->fileinfo_list.isEmpty()?UpdaterDialog::DownloadAll:UpdaterDialog::UpdateAppinfo;
	UpdaterDialog dlg(this,fileinfo_list,flag);
	dlg.move( mapToGlobal( this->rect().center()/2 ));
	int result = dlg.exec() ;
	if( result )
	{
		/*reset all appinfo from new to only_info*/
		for(int i=0;i<fileinfo_list.count();i++)
		{
			fileinfo_list[i].appInfo->unSetFlag(AppInfo::New);
			fileinfo_list[i].appInfo->saveApplicationInfo();
		}

		load();

		foreach(QString name,dlg.getNewAppsList()){

			onSetAppInfoUpdated(name);
		}

		SettingsDialog::setValue("LAST_INFO_CHECK",QDateTime::currentMSecsSinceEpoch()/1000);
		ui->btUpdateInfo->setIcon(QIcon(":icons/isnew.ico"));
		QList<QTreeWidgetItem*> items = ui->TCategoryTree->findItems("New/Updated",Qt::MatchExactly);

		if(items.count()==0)
			return;
		QTreeWidgetItem *item = items[0];

		ui->TCategoryTree->setCurrentItem(item,0,QItemSelectionModel::ClearAndSelect);
	}else{
		SettingsDialog::setValue("LAST_INFO_CHECK",QDateTime::currentMSecsSinceEpoch()/1000);
		load();
	}
}

void AppList::on_lbSearch_textChanged(const QString &arg1)
{
	ui->LAppInfoList->clear();
	QStringList strings = arg1.split(' ');

	for(int i=0;i<fileinfo_list.count();i++)
	{
		if( fileinfo_list.at(i).appInfo->contains(strings) )
			appendToList(fileinfo_list[i]);
	}
	drawButtons();
}

void AppList::on_btReload_clicked()
{
	if(!tasksRunning())
		load();
	else
		QMessageBox::information(this,"Unable to reload","Can't reload list while there are running tasks!");
}

void AppList::on_startTasksBtn_clicked()
{
	if(ui->automaticRb->isChecked()){
		ui->taskList->setAutomaticFirst(true);
	}else{
		ui->taskList->setAutomaticFirst(false);
	}
	ui->taskList->startTasks();
}

void AppList::on_btnTask_clicked()
{
	ui->stackedWidget->setCurrentIndex(0);
	ui->startTasksBtn->show();
	ui->groupBox->show();
	ui->lbSearch->hide();
	ui->btnTask->hide();
}

void AppList::onSetAppInfoUpdated(QString name)
{
	Fileinfo info;
	info.name = name;
	int i = this->fileinfo_list.indexOf(info);
	if(i>=0){
		info = this->fileinfo_list.at(i);
		info.appInfo->setFlag(AppInfo::New);
		info.appInfo->saveApplicationInfo();
		qDebug()<<"Set to New: "<<name;
	}else{
		qDebug()<<name<<" not found";
	}
}
