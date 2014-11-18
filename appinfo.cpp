#include "appinfo.h"
#include <QDomDocument>
#include <QFile>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QFileIconProvider>
#include <QIcon>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDate>
#include <QThread>
#include <QEventLoop>
#include <QDir>
#include <QDebug>
#include <QApplication>
#include <appinfo_registry.h>
#include <QTimer>
#include <QDomCDATASection>
#include <QStandardPaths>
#include <QNetworkCookie>
#include <QNetworkConfiguration>
#include <QSysInfo>
#include <QMap>
#include <iostream>
#include <string>
#include <list>
#include "settingsdialog.h"
#include "utils/qstringext.h"
#include "yaml-cpp/yaml.h"

#if !defined(QT_DEBUG)
QFileIconProvider iconprovider;
#endif

AppInfo::AppInfo(QString app_name):
	QObject(NULL)
{
	_name = app_name;
	_state = Invalid;
	setFlag(WinVersionOk);
}

AppInfo::~AppInfo()
{
	saveApplicationInfo();
}

bool AppInfo::isFlagSet(Flags flag)
{
	return ( (_state & flag) == flag);
}

void AppInfo::setFlag(Flags flag)
{
	_state = AppInfo::Flags(_state|flag);
}

void AppInfo::unSetFlag(Flags flag)
{
	_state = AppInfo::Flags(_state & ~flag);
}

/**
* @brief AppInfo::newerVersionAvailable
*   Compares newest version with installed/downloaded
*   to determine of the newest is more recent.
*   This is done by comparing each version part seperatly.
*   Only the numbers are taken into account.
*
* @return true if new version available
*/
bool AppInfo::newerVersionAvailable()
{
	QStringList version1,version2;
	QString installedVersion = _registryInfo.version;

	QString v1 = _userData.latestVersion;
	QString v2 = installedVersion.isEmpty()?_userData.downloadedVersion:installedVersion;

	if(v1.isEmpty() || v2.isEmpty())
		return false;

	if(isFlagSet(IgnoreLatest) || !isFlagSet(WinVersionOk))
		return false;

	int i;

	//seperate version by dots, only extract numbers
	QString vers;
	for(i=0; i < v1.length() && ((v1[i]>='0' && v1[i]<='9') ||  v1[i] == '.' ); i++)
	{
		qDebug() << v1[i];
		if(v1[i] != '.')
			vers += v1[i];

		if(v1[i] == '.' || i == v1.length()-1)
		{
			version1.append(vers);
			vers.clear();
		}
	}
	version1.append(vers);
	vers.clear();

	for(i=0; i < v2.length() && ((v2[i]>='0' && v2[i]<='9') || v2[i] == '.' ); i++)
	{
		if(v2[i] != '.')
			vers += v2[i];

		if(v2[i] == '.' || i == v2.length()-1)
		{
			version2.append(vers);
			vers.clear();
		}
	}
	version2.append(vers);

	i = 0;
	//compare version parts
	while(i < version1.count() && i < version2.count() )
	{
		int v1val,v2val;
		QString vers1,vers2;

		vers1 = version1[i];
		vers2 = version2[i];
		bool ok1,ok2;
		v1val = vers1.toInt(&ok1);
		v2val = vers2.toInt(&ok2);

		if( !ok1 || !ok2 )
			break;

		if(v1val == v2val)
		{
			i++;
			continue;
		}
		return v1val > v2val;
	}

	if( version1.count() != version2.count() )
	{
		return version1.count() > version2.count();
	}

	return v1 > v2;
}

void AppInfo::removeDownloadedFile()
{
	QFile::remove( _path + "/" + _userData.filename);
	unSetFlag(AppInfo::Downloaded);
	unSetFlag(AppInfo::UpdateAvailable);
	_userData.downloadedVersion.clear();
}

void AppInfo::ignoreNewerVersions(bool ignore)
{
	if(ignore){

		setFlag(AppInfo::IgnoreLatest);
		unSetFlag(AppInfo::UpdateAvailable);
		_userData.latestVersion.clear();
	}else{

		unSetFlag(AppInfo::IgnoreLatest);
		updateVersion();
	}
}

bool AppInfo::forceRegistryToLatestVersion()
{
	QSettings values(_registryInfo.path,QSettings::NativeFormat);
	qDebug(_registryInfo.path.toLatin1().data());
	values.setValue("DisplayVersion",_userData.latestVersion);
	if( values.status() == QSettings::NoError )
	{
		_registryInfo.version = _userData.latestVersion;
		unSetFlag(UpdateAvailable);
		return true;
	}
	else
		return false;
}


void AppInfo::updateRegistryVersion()
{
	QSettings settings(_registryInfo.path);
	QString version = settings.value("DisplayVersion").toString();
	_registryInfo.version = version;
}

QNetworkReply* AppInfo::updateVersion(QNetworkAccessManager *qnam)
{
	QString url = _version.url;
	QNetworkReply *reply;
	QNetworkRequest request = QNetworkRequest(url);
	qDebug()<<"url: "<<url;

	if(url.contains("filehippo", Qt::CaseInsensitive)){

		QList<QNetworkCookie> cookies;
		QNetworkCookie cookie;
		cookie.setName("FH_PreferredCulture");
		cookie.setValue("en-US");
		cookies.append(cookie);

		if(!isFlagSet(FilehippoBeta))
		{
			cookie.setName("Filter");
			cookie.setValue("NOBETA=1&NODEMO=0");
			cookies.append(cookie);
		}

		QVariant var;
		var.setValue(cookies);

		request.setHeader(QNetworkRequest::CookieHeader, var);
		qDebug()<<request.rawHeader("Cookie");
	}
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/xml");;
	request.setRawHeader("Accept","*/*");
	request.setRawHeader("User-Agent","*");


	if(qnam == NULL)
	{
		qnam = new QNetworkAccessManager;
		reply= qnam->get(request);

	}else
	{
		reply = qnam->get(request);
		reply->setParent(NULL);
		qDebug(reply->url().toString().toLatin1());
	}

	connect(reply,SIGNAL(finished()),SLOT(onHtmlVersionDownloaded()));
	return reply;
}

void AppInfo::updateAppInfo()
{
	QString url = "http://appdriverupdate.sourceforge.net/Apps/Files/"+_name+".xml";

	QNetworkAccessManager *qnam = new QNetworkAccessManager;

	QNetworkRequest request = QNetworkRequest(url);

	QNetworkReply *reply = qnam->get(request);
	connect(reply,SIGNAL(finished()),SLOT(onHtmlAppInfoDownloaded()));
}


void AppInfo::onHtmlAppInfoDownloaded()
{
	QNetworkReply *reply = (QNetworkReply*) sender();
	QString html;
	html = reply->readAll();
	if(html.isEmpty())
	{
		emit infoUpdated(this);
		return;
	}
	reply->deleteLater();
	if(reply->manager())
		reply->manager()->deleteLater();

	QFile file("Info/"+_name+".xml");

	if( !file.open(QFile::WriteOnly) )
	{
		qDebug(tr("failed to open file: %1").arg(_name).toLatin1());
		emit appInfoUpdated(this,false);
		return;
	}

	file.write( QByteArray(html.toUtf8()));
	saveApplicationInfo();

	emit appInfoUpdated(this,true);
	emit infoUpdated(this);
}

void AppInfo::onHtmlVersionDownloaded()
{
	QNetworkReply *reply = (QNetworkReply*) sender();
	QString html;
	QString version;
	QString regexString = _version.regex;
	QRegExp regex(regexString);
	regex.setMinimal(true);


	html = reply->readAll();

	qDebug(tr("updating version of: %1").arg(_name).toLatin1().data());

	if( reply->parent() != NULL )
	{
		reply->deleteLater();
		if(reply->manager())
			reply->manager()->deleteLater();
	}

	if(html.isEmpty()){
		_error = "downloaded html is empty";
		qDebug("html empty "+reply->url().toString().toLatin1());
	}

	qDebug()<<html;
	qDebug()<<regexString;

	//Get version base from HTML
	regex.indexIn(html);
	version = regex.cap(1);

	if(version.isEmpty())
	{
		qDebug("no version found: %s",regex.errorString().toLatin1().data());
		_error = "no version found -> regex failed";
		emit versionUpdated(this,false);
		return;
	}

	//apply a regex replace to the base version to adapt it to the installed version


	regexString = _version.versionRegex;
	qDebug(regexString.toLatin1());
	qDebug(version.toLatin1());

	regex.setPattern(regexString);
	regex.setMinimal(false);

	version = version.replace(regex,_version.versionRegexReplace);

	_userData.latestVersion = version;

	if( newerVersionAvailable())
	{
		setFlag(UpdateAvailable);
	}
	else
	{
		unSetFlag(UpdateAvailable);
	}


	_userData.lastCheck = QDate::currentDate().toString(Qt::DefaultLocaleShortDate);
	saveApplicationInfo();

	emit versionUpdated(this,true);
	emit infoUpdated(this);
}

bool AppInfo::contains(QStringList &strings)
{
	for(int i=0;i<strings.count();i++)
	{
		bool ok = false;
		QString s = strings.at(i);
		if(_name.contains(s, Qt::CaseInsensitive))
			ok = true;
		if(_description.contains(s, Qt::CaseInsensitive))
			ok = true;
		if(_webpage.contains(s, Qt::CaseInsensitive))
			ok = true;
		if(_info.contains(s, Qt::CaseInsensitive))
			ok = true;

		if(!ok)
			return false;
	}
	return true;
}

void AppInfo::setFilehippoBeta(bool enable)
{
	if(enable){
		setFlag(FilehippoBeta);
		updateVersion();
	}
	else
		unSetFlag(FilehippoBeta);
	saveApplicationInfo();
}


QString AppInfo::getDownloadURL(UrlContainer &fileLocation)
{
	QString url = fileLocation.url;

	if( url.isEmpty() ){
		fileLocation = getDownloadedFileUrl();
		url = fileLocation.url;
	}
	QMap<QString,QString> map = fileLocation.parameter;
	QMap<QString, QString>::const_iterator i = map.constBegin();
	while (i != map.constEnd()) {
		QString replace_what = i.key();
		QString replace_with = i.value();
		url.replace(tr("{%1}").arg(replace_what),replace_with);
		i++;
	}

	url.replace("{version}",_userData.latestVersion);

	UrlContainer::FollowLink *link = fileLocation.followLink;

	int count=1;


	for(; link!=NULL; link=link->followLink)
	{
		qDebug()<<"link: "<<link->regex;
		QUrl baseurl(url);
		QString html;
		QEventLoop eventloop;
		QString regexString = link->regex;
		QRegExp regex(regexString);
		regex.setMinimal(true);
		qDebug()<<regexString;

		QNetworkAccessManager qnam;
		QNetworkRequest request = QNetworkRequest(url);
		if(url.contains("filehippo") )
		{
			QList<QNetworkCookie> cookies = getFileHippoCookies();
			QVariant var;
			var.setValue(cookies);

			request.setHeader(QNetworkRequest::CookieHeader, var);
			qDebug()<<request.rawHeader("Cookie");

		}
		request.setHeader(QNetworkRequest::ContentTypeHeader, "application/xml");;
		request.setRawHeader("Accept","*/*");
		request.setRawHeader("User-Agent","*");
		QNetworkReply *reply = qnam.get(request);
		connect(reply,SIGNAL(finished()),&eventloop,SLOT(quit()));

		qDebug()<<baseurl;

		eventloop.exec();

		html = reply->readAll();

		if(!html.isEmpty())
		{
			int index = regex.indexIn(html);
			if( index == -1)
			{
				qDebug("regex on url failed");
				QString n;n.setNum(count);
				_error=regexString+": regex index failed, step:"+n;
				return "";
			}

			QString urlstr = regex.cap(1);
			if(urlstr.isEmpty()){
				QString n;n.setNum(count);
				_error = regexString+": regex capture failed, step: "+n;
			}

			qDebug() << "regex captured: " << urlstr.left(256)+"...";


			QUrl url_resolve(urlstr);
			if(!url_resolve.isValid()){

				QString n;n.setNum(count);
				_error = "url is not valid, step: "+n;
				return "";
			}


			if ( url_resolve.isRelative() )
				url_resolve = baseurl.resolved(url_resolve);

			qDebug()<<url_resolve;

			url = url_resolve.toString();
		}
		else
		{
			QString n;n.setNum(count);
			_error = "downloaded html is empty, step: "+n;
			qDebug("html is empty");
			return "";
		}
		count++;
	}
	return url;
}

QIcon AppInfo::getIcon()
{
#if defined(QT_DEBUG)
	QFileIconProvider iconprovider;
#endif
	if( isFlagSet(SelectedInstallDownload) )
		return QIcon(":icons/Download.ico");

	if( isFlagSet(SelectedRemove) )
		return QIcon(":icons/Remove.ico");

	if( isFlagSet(UpdateAvailable) )
		return QIcon(":icons/hasupdate.ico");

	if( isFlagSet(Installed) )
	{
		if( _registryInfo.icon.isEmpty() )
			return QIcon(":icons/IsInstalled.ico");

		if( _appIcon.isNull() )
			return QIcon(":icons/IsInstalled.ico");
		else
			return _appIcon;
	}

	if( isFlagSet(NoRegistry) )
	{
		if( _appIcon.isNull() )
			return QIcon(":icons/IsInstalled.ico");
		else
			return _appIcon;
	}

	if( isFlagSet(Downloaded) )
		return QIcon(":icons/IsDL.ico");

	if( isFlagSet(New) || isFlagSet(OnlyInfo) )
		return QIcon(":icons/isnew.ico");

	return QIcon(":icons/isnew.ico");
}



void AppInfo::saveApplicationInfo()
{
	if( isFlagSet(NoInfo) )
		return;

	YAML::Emitter out;
	out << YAML::BeginMap;
	out << YAML::Key << "downloaded_id";
	out << YAML::Value << _userData.downloadedId;
	out << YAML::Key << "downloaded_version";
	out << YAML::Value << _userData.downloadedVersion.toStdString();
	out << YAML::Key << "filename";
	out << YAML::Value << _userData.filename.toStdString();
	out << YAML::Key << "ignore_latest_version";
	out << YAML::Value << isFlagSet(IgnoreLatest);
	out << YAML::Key << "is_not_new";
	out << YAML::Value << !isFlagSet(New);
	out << YAML::Key << "last_check";
	out << YAML::Value << _userData.lastCheck.toStdString();
	out << YAML::Key << "version";
	out << YAML::Value << _userData.latestVersion.toStdString();
	out << YAML::Key << "filehippo_beta";
	out << YAML::Value << isFlagSet(FilehippoBeta);

	out << YAML::Key << "choice";
	out << YAML::Value << YAML::BeginMap;
	QMapIterator<QString, QString> i(_userData.choice);
	while (i.hasNext()) {

		i.next();
		out << YAML::Key << i.key().toStdString();
		out << YAML::Value << i.value().toStdString();
	}
	out << YAML::EndMap;

	out << YAML::EndMap;

	QDir dir = QDir::current();
	dir.mkdir("user");
	QFile yamldoc("user/"+_name+".yaml");
	if( yamldoc.open(QFile::WriteOnly) ){

		yamldoc.write(out.c_str());
	}
}


void AppInfo::checkMaxWinVersion(QString max_version)
{
	QSysInfo::WinVersion version =  QSysInfo::windowsVersion();
	if(max_version == "XP" && version>QSysInfo::WV_XP)
		unSetFlag(WinVersionOk);
	if(max_version == "VISTA" && version>QSysInfo::WV_VISTA)
		unSetFlag(WinVersionOk);
	if(max_version == "WIN7" && version>QSysInfo::WV_WINDOWS7)
		unSetFlag(WinVersionOk);

	if(max_version == "XP64" && (version>QSysInfo::WV_XP || QSysInfo::WordSize != 64) )
		unSetFlag(WinVersionOk);
	if(max_version == "VISTA64" && (version>QSysInfo::WV_VISTA || QSysInfo::WordSize != 64) )
		unSetFlag(WinVersionOk);
	if(max_version == "WIN764" && (version>QSysInfo::WV_WINDOWS7 || QSysInfo::WordSize != 64) )
		unSetFlag(WinVersionOk);

	if(max_version == "XP32" && (version>QSysInfo::WV_XP || QSysInfo::WordSize == 64) )
		unSetFlag(WinVersionOk);
	if(max_version == "VISTA32" && (version>QSysInfo::WV_VISTA || QSysInfo::WordSize == 64) )
		unSetFlag(WinVersionOk);
	if(max_version == "WIN732" && (version>QSysInfo::WV_WINDOWS7 || QSysInfo::WordSize == 64) )
		unSetFlag(WinVersionOk);

}

void AppInfo::checkMinWinVersion(QString min_version)
{
	QSysInfo::WinVersion version =  QSysInfo::windowsVersion();
	if(min_version == "XP" && version<QSysInfo::WV_XP)
		unSetFlag(WinVersionOk);
	if(min_version == "VISTA" && version<QSysInfo::WV_VISTA)
		unSetFlag(WinVersionOk);
	if(min_version == "WIN7" && version<QSysInfo::WV_WINDOWS7)
		unSetFlag(WinVersionOk);

	if(min_version == "XP64" && (version<QSysInfo::WV_XP || QSysInfo::WordSize != 64) )
		unSetFlag(WinVersionOk);
	if(min_version == "VISTA64" && (version<QSysInfo::WV_VISTA || QSysInfo::WordSize != 64) )
		unSetFlag(WinVersionOk);
	if(min_version == "WIN764" && (version<QSysInfo::WV_WINDOWS7 || QSysInfo::WordSize != 64) )
		unSetFlag(WinVersionOk);

	if(min_version == "XP32" && (version<QSysInfo::WV_XP || QSysInfo::WordSize == 64) )
		unSetFlag(WinVersionOk);
	if(min_version == "VISTA32" && (version<QSysInfo::WV_VISTA || QSysInfo::WordSize == 64) )
		unSetFlag(WinVersionOk);
	if(min_version == "WIN732" && (version<QSysInfo::WV_WINDOWS7 || QSysInfo::WordSize == 64) )
		unSetFlag(WinVersionOk);

}

void AppInfo::delayedSetIcon()
{
#if defined(QT_DEBUG)
	QFileIconProvider iconprovider;
#endif

	if(isFlagSet(NoRegistry) && isFlagSet(Downloaded))
	{
		QPixmap pixmap;
		QString Fullfilename = _path+"/"+_userData.filename;
		if( pixmap.load(Fullfilename) )
			_appIcon = QIcon(pixmap);
		else
			_appIcon = iconprovider.icon(QFileInfo(Fullfilename));

		emit infoUpdated(this);
		return;
	}

	if(!QFile::exists(_registryInfo.icon))
		return;

	QPixmap pixmap;
	if( pixmap.load(_registryInfo.icon) )
		_appIcon = QIcon(pixmap);
	else
		_appIcon = iconprovider.icon(QFileInfo(_registryInfo.icon));

	emit infoUpdated(this);
}

void AppInfo::setRegistryInfo(RegistryGroup *reg_group)
{
	if(!isFlagSet(WinVersionOk))
	{
		qDebug()<< reg_group->displayed_name <<": winversion not supported";
		return;
	}

	if(isFlagSet(NoRegistry))
		return;

	_registryInfo.path = reg_group->group_base+reg_group->group_name;

	qDebug("%s: %s\n",_name.toLatin1().data(),_registryInfo.path.toLatin1().data());

	_registryInfo.icon              = ext(reg_group->getValue("DisplayIcon")).beforeLast(",");
	_registryInfo.silentUninstall  = reg_group->getValue("QuietUninstallString");
	_registryInfo.uninstall         = reg_group->getValue("UninstallString");
	_registryInfo.version           = reg_group->getValue("DisplayVersion");
	_registryInfo.displayName	    = reg_group->displayed_name;

	if(_registryInfo.version.isEmpty() && _registry.versionPath=="DISPLAYNAME")
	{
		_registryInfo.version = _registryInfo.displayName;
	}

	if(!_registryInfo.versionPath.isEmpty() && _registry.versionPath!="DISPLAYNAME")
	{
		QSettings values( ext(_registryInfo.versionPath).beforeLast('\\'),QSettings::NativeFormat);
		_registryInfo.version = values.value(ext(_registryInfo.versionPath).afterLast('\\')).toString();
	}

	if(_registryInfo.icon.isEmpty() && _registryInfo.version.isEmpty()
			&& _registryInfo.uninstall.isEmpty() && _registryInfo.silentUninstall.isEmpty() )
	{
		qDebug("no version,icon,uninstall,silentuninstall found");
		return;
	}

	QString regexString = _registry.versionRegex;
	QRegExp regex(regexString);
	_registryInfo.version = _registryInfo.version.replace(regex,_registry.regexReplace);

	QTimer::singleShot((qrand()%30+5)*1000,this,SLOT(delayedSetIcon()));
	setFlag(Installed);
	if( newerVersionAvailable() )
		setFlag(UpdateAvailable);
	else
		unSetFlag(UpdateAvailable);

	unSetFlag(OnlyInfo);
	QTimer::singleShot((qrand()%30+5)*1000,this,SLOT(delayedSetIcon()));
	emit infoUpdated(this);
}

bool AppInfo::parseRegistryInfo()
{
	QStringList regBase;
	QString path;
	if(!isFlagSet(WinVersionOk))
		return false;
	if(isFlagSet(NoRegistry))
		return false;

	if( QSysInfo::WordSize == 64 )
	{
		regBase <<  "HKEY_CURRENT_USER\\Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\";
		regBase <<  "HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\";
	}
	regBase <<  "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\";
	regBase <<  "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\";

	qDebug()<<"search registry for: "<<_registry.searchKey;


	if(_registryInfo.path.startsWith("HKEY_CURRENT_USER")
			|| _registryInfo.path.startsWith("HKEY_LOCAL_MACHINE"))
	{
		path = _registryInfo.path;
		goto next;
	}

	for(int a=0;a<regBase.count();a++)
	{
		QSettings registry(regBase[a], QSettings::NativeFormat);
		QStringList childkeys;

		QSettings settings( ext(_registry.searchKey).beforeLast('\\'), QSettings::NativeFormat);

		if( settings.contains( ext(_registry.searchKey).afterLast('\\')) )
			goto next;

		if( registry.contains( _registry.searchKey ) )
		{
			path = regBase[a] + _registry.searchKey;
			goto next;
		}

		childkeys = registry.childGroups();
		for(int i=0;i<childkeys.count();i++)
		{
			if( childkeys.at(i).contains(_registry.searchKey, Qt::CaseInsensitive) )
			{
				path = regBase[a] + childkeys[i];
				goto next;
			}
			QSettings valuesettings(regBase[a]+childkeys[i],QSettings::NativeFormat);
			QStringList valuekeys = valuesettings.childKeys();
			for(int j=0;j<valuekeys.count();j++)
			{
				if( valuesettings.value( valuekeys.at(j) ).toString().contains(_registry.searchKey, Qt::CaseInsensitive) )
				{
					path = regBase[a] + childkeys[i];
					goto next;
				}
			}
		}

		if( !_registry.versionPath.isEmpty() )
		{
			QSettings settings2( ext(_registryInfo.versionPath).beforeLast('\\'), QSettings::NativeFormat);
			QString version = settings2.value( ext(_registryInfo.versionPath).afterLast('\\')).toString();

			if( !version.isEmpty())
			{
				QString regexString = _registry.versionRegex;
				QRegExp regex(regexString);
				version = version.replace(regex,_registry.regexReplace);
				_registryInfo.version = version;

				qDebug()<<"registry version found: "<<_registryInfo.versionPath.toLatin1().data();

				setFlag(Installed);
				if( newerVersionAvailable() )
					setFlag(UpdateAvailable);
				else
					unSetFlag(UpdateAvailable);

				unSetFlag(OnlyInfo);
				emit infoUpdated(this);
				return true;
			}
		}
	}

	return false;

next:
	path.replace("/","\\");
	qDebug()<<"registry found: %s"<<path;
	_registryInfo.path = path;
	QSettings values(path,QSettings::NativeFormat);


	_registryInfo.icon              = values.value("DisplayIcon").toString().remove(",0");
	_registryInfo.silentUninstall   = values.value("QuietUninstallString").toString();
	_registryInfo.uninstall         = values.value("UninstallString").toString();
	_registryInfo.version           = values.value("DisplayVersion").toString();
	_registryInfo.displayName	    = values.value("DisplayName").toString();

	if(_registryInfo.version.isEmpty() && _registry.versionPath=="DISPLAYNAME")
	{
		_registryInfo.version = _registryInfo.displayName;
	}

	if(!_registryInfo.versionPath.isEmpty() && _registry.versionPath!="DISPLAYNAME")
	{
		QSettings values(ext(_registryInfo.versionPath).beforeLast('\\'),QSettings::NativeFormat);
		_registryInfo.version = values.value( ext(_registryInfo.versionPath).afterLast('\\')).toString();
	}

	if(_registryInfo.icon.isEmpty() && _registryInfo.version.isEmpty()
			&& _registryInfo.uninstall.isEmpty() && _registryInfo.silentUninstall.isEmpty() )
		return false;

	QString regexString = _registry.versionRegex;
	QRegExp regex(regexString);
	_registryInfo.version = _registryInfo.version.replace(regex,_registry.regexReplace);

#if defined(QT_DEBUG)
	QFileIconProvider iconprovider;
#endif

	QTimer::singleShot((qrand()%30+5)*1000,this,SLOT(delayedSetIcon()));
	setFlag(Installed);
	if( newerVersionAvailable() )
		setFlag(UpdateAvailable);
	else
		unSetFlag(UpdateAvailable);

	unSetFlag(OnlyInfo);
	emit infoUpdated(this);
	return true;
}


bool AppInfo::load()
{
	qDebug()<<"loading "<<_name;
	if (!loadYaml())
		return false;

	if (this->_registry.searchKey.isEmpty()
			&& this->_registryInfo.versionPath.isEmpty()){
		setFlag(NoRegistry);
	}

	_path="/Files/";

	for (int index = 0; index<_categories.count(); index++)
	{
		_path=_path + _categories[index] + "/" ;
	}

    if(SettingsDialog::value<bool>(SettingsDialog::SaveDownloaded)){
		_path = QDir::currentPath() + "/" + _path + _name;
	}else{
		QString path = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
		_path = path + _path + _name;
	}

	QFileInfo qfi(_path+"/"+_userData.filename);
	if(!_userData.filename.isEmpty() && qfi.exists())
	{
		qDebug()<<"is downloaded";
		setFlag(Downloaded);
	}else
		_userData.downloadedVersion.clear();



	if( newerVersionAvailable() )
		setFlag(UpdateAvailable);

	if(!isFlagSet(Downloaded))
		setFlag(OnlyInfo);

	if( _registry.searchKey.contains("\\") || _registry.versionPath.contains("\\"))
	{
		qDebug("parse registry for: %s",_name.toLatin1().data());
		if(parseRegistryInfo()){
			setFlag(Installed);
		}
	}

	if(isFlagSet(NoRegistry))
		QTimer::singleShot((qrand()%30+5)*1000,this,SLOT(delayedSetIcon()));
	return true;
}



void operator >> (const YAML::Node &node,QString &string){

	if(!node){
		return;
	}

	std::string value;
	value = node.as<std::string>();
	string = QString::fromStdString(value);
}

void operator >> (const YAML::Node &node,int &integer){

	if(!node){
		return;
	}

	try{
		integer = node.as<int>();
	}catch(...){}
}

void operator >> (const YAML::Node &node,bool &boolean){
    if(!node){
        return;
    }
	qDebug()<<"parsing bool";
	try{
		boolean = node.as<bool>();
    }catch(const std::runtime_error &e){
		qDebug()<<"failed to parse bool";
	}
	qDebug()<<"parsed bool";
}

void operator >> (const YAML::Node &node,AppInfo::Version &v){
	if(!node){
		return;
	}

	node["regex"] >> v.regex;
	node["url"] >> v.url;

	if(v.regex.isEmpty()){
		QString startString;
		QString endString;
		node["start_string"] >> startString;
		node["end_string"] >> endString;
		v.regex = QRegExp::escape(startString) + "(.{1,20})" + QRegExp::escape(endString);
	}


	node["version_regex"] >> v.versionRegex;

	if(v.versionRegex.isEmpty()){
		v.versionRegex = "(\\S*)(.*)";
	}


	node["version_regex_replace"] >> v.versionRegexReplace;
	if(v.versionRegexReplace.isEmpty()){
		v.versionRegexReplace = "\\1";
	}

}


void operator >> (const YAML::Node &node,QMap<QString,QString> &map){

	if(!node){
		return;
	}

	for(YAML::const_iterator it=node.begin();it!=node.end();++it) {
		QString key, value;
		it->first >> key;
		it->second >> value;

		map[key]=value;
	}
}

void operator >> (const YAML::Node &node,QList<AppInfo::UrlContainer> &l){

	if(!node){
		return;
	}

	for(unsigned int i=0;i<node.size();i++){

		AppInfo::UrlContainer location;
		location.load(node[i]);
		l.append(location);
	}
}

void operator >> (const YAML::Node &node,AppInfo::UrlContainer &f){

	if(!node){
		return;
	}

	f.load(node);
}

void operator >> (const YAML::Node &node,QList<QString> &l){

	if(!node){
		return;
	}

	for(int i=0;i<node.size();i++){

		QString value;
		node[i] >> value;
		l.append(value);
	}
}
void operator >> (const YAML::Node &node,AppInfo::Install &i){

	if(!node){
		return;
	}

	node["needs_admin"] >> i.needsAdmin;
	node["only_silent"] >> i.onlySilent;
	node["needs_uninstall"] >> i.needsUninstall;

	node["last_process"] >> i.lastProcess;
	node["parameter"] >> i.silentParameter;
	//old
	node["params"] >> i.silentParameter;

	node["silent_parameter"] >> i.silentParameter;
	node["zip_exe_regex"] >> i.zipExeRegex;

}

void operator >> (const YAML::Node &node,AppInfo::Remove &r){

	if(!node){
		return;
	}

	node["last_process"] >> r.lastProcess;
	node["parameter"] >> r.silentParameter;
	node["silent_parameter"] >> r.silentParameter;
	node["needs_admin"] >> r.needsAdmin;
	node["only_silent"] >> r.onlySilent;
}
void operator >> (YAML::Node &n,AppInfo::UserData &s){

	try{
		n["downloaded_id"] >> s.downloadedId;
		n["downloaded_version"] >> s.downloadedVersion;
		n["last_check"] >> s.lastCheck;
		n["version"] >> s.latestVersion;
		n["filename"] >> s.filename;
		n["choice"] >> s.choice;
	}catch(YAML::ParserException& e){
		qDebug()<<"user data";
		qDebug()<<"error: "<< e.what();
	}
}


void operator >> (const YAML::Node &node, AppInfo::Registry &r){

	if(!node){
		return;
	}


	if( QSysInfo::WordSize == 64 )
	{
		node["search64"] >> r.searchKey;
		node["version64"] >> r.versionPath;
	}

	if(r.searchKey.isEmpty())
		node["search"] >> r.searchKey;


	if(r.versionPath.isEmpty())
		node["version"] >> r.versionPath;


	node["version_regex"] >> r.versionRegex;
	if(r.versionRegex.isEmpty())
		r.versionRegex = "(\\S*)(.*)";

	node["regex_replace"] >> r.regexReplace;
	if(r.regexReplace.isEmpty())
		r.regexReplace = "\\1";

}

bool AppInfo::loadYaml()
{
	QFile file("Info/"+_name+".yaml");
	file.open(QFile::ReadOnly);
	QString yaml = file.readAll();
	std::stringstream stream(yaml.toStdString());
	YAML::Node doc;


	try{
		doc = YAML::Load(stream);
		load(doc["package"]);
		loadUserData();
		return true;

    }catch(YAML::Exception &e){
		this->_description = "an error occurred during yaml parsing";
		this->_info = e.what();
		qDebug()<<"error: "<< e.what();
    }
    return false;


}


void AppInfo::loadUserData()
{
	qDebug()<<"loading user data";
	QFile userdoc("user/"+_name+".yaml");
	if( userdoc.open(QFile::ReadOnly) ){

		QString yaml = userdoc.readAll();
		std::stringstream stream(yaml.toStdString());


		YAML::Node doc;
		doc = YAML::Load(stream);
		doc >> _userData;

		bool ok;
		doc["ignore_latest_version"] >> ok;
		if(ok)
			setFlag(IgnoreLatest);

		doc["is_not_new"] >> ok;
		if(!ok)
			setFlag(New);

		doc["filehippo_beta"] >> ok;
		if(ok)
			setFlag(FilehippoBeta);
	}else
		setFlag(New);
}

QList<QNetworkCookie> AppInfo::getFileHippoCookies()
{
	QList<QNetworkCookie> cookies;
	QNetworkCookie cookie;
	cookie.setName("FH_PreferredCulture");
	cookie.setValue("en-US");
	cookies.append(cookie);

	if(!isFlagSet(FilehippoBeta))
	{
		cookie.setName("Filter");
		cookie.setValue("NOBETA=1&NODEMO=0");
		cookies.append(cookie);
	}

	return cookies;
}

void AppInfo::load(const YAML::Node &node){

	node["name"] >> this->_name;
	node["description"] >> this->_description;
	node["webpage"] >> this->_webpage;
	node["info"] >> this->_info ;

	QString min_version;
	QString max_version;
	node["min_win_version"] >> min_version;
	node["max_win_version"] >> max_version;

	checkMinWinVersion(min_version);
	checkMaxWinVersion(max_version);

	qDebug()<<"loading version";
	node["version"] >> this->_version;
	qDebug()<<"loading url_containers";
	node["url_containers"] >> this->_urlContainers;
	qDebug()<<"loading categories";
	node["categories"] >> this->_categories;
	qDebug()<<"loading install";
	node["install"] >> this->_install;
	qDebug()<<"loading remove";
	node["remove"] >> this->_remove;
	qDebug()<<"loading registry";
	node["registry"] >> this->_registry;
}

void AppInfo::updateDownloadVersion()
{
	_userData.downloadedVersion = _userData.latestVersion;
	setFlag(AppInfo::Downloaded);
	unSetFlag(AppInfo::OnlyInfo);
}

QString AppInfo::path()
{
	return this->_path;
}

QString AppInfo::fileName()
{
	return this->_userData.filename;
}

void AppInfo::setDownloaded(int id, QString name)
{
	_userData.filename = name;
	_userData.downloadedId=id;
	updateDownloadVersion();
}

void AppInfo::setChoice(QString key, QString value)
{
	_userData.choice[key] = value;
}

void AppInfo::setExtractedFilename(QString name)
{
	_userData.filename = name;
}

QString AppInfo::lastError()
{
	return _error;
}

QString AppInfo::latestVersion()
{
	return _userData.latestVersion;
}


void AppInfo::UrlContainer::load(const YAML::Node &node){

	node["description"] >> description;
	node["url"] >> url;
	node["id"] >> id;

	QMap<QString,QString> map;
	try{
		node["parameters"] >> map;
	}catch(...){}

	parameter = map;

	const YAML::Node &value = node["follow_link"];
	if(value){
		followLink = new AppInfo::UrlContainer::FollowLink();
		followLink->load(value);
	}
}

void AppInfo::UrlContainer::FollowLink::load(const YAML::Node &node)
{

	node["regex"] >> regex;

	if( regex.isEmpty() ){

		QString startString;
		QString endString;
		node["start_string"] >> startString;
		node["end_string"] >> endString;

		regex = QRegExp::escape(startString) + "(\\S*)" + QRegExp::escape(endString);
	}


	if(node["follow_link"]){
		followLink = new AppInfo::UrlContainer::FollowLink();
		followLink->load(node["follow_link"]);
	}
}


/*------------------info-------------------*/

QString AppInfo::name(){
	return this->_name;
}

QString AppInfo::description(){
	return this->_description;
}

QString AppInfo::webpage(){
	return this->_webpage;
}

QString AppInfo::info(){
	return this->_info;
}

AppInfo::Version AppInfo::version(){
	return this->_version;
}

QList<AppInfo::UrlContainer> AppInfo::urls(){
	return this->_urlContainers;
}

QList<QString> AppInfo::categories(){
	return this->_categories;
}

AppInfo::Install AppInfo::getInstallInfo(){
	return this->_install;
}

AppInfo::Remove AppInfo::getRemoveInfo(){
	return this->_remove;
}

AppInfo::UserData AppInfo::getUserData(){
	return this->_userData;
}

AppInfo::Registry AppInfo::registry()
{
	return _registry;
}

AppInfo::RegistryInfo AppInfo::registryInfo()
{
	return _registryInfo;
}

AppInfo::UrlContainer AppInfo::getDownloadedFileUrl()
{
	foreach(UrlContainer urlContainer,_urlContainers){

		if( urlContainer.id == _userData.downloadedId){
			if( _userData.choice.size()>0 ){
				urlContainer.parameter = _userData.choice;

				urlContainer.description = "";
				QMapIterator<QString, QString> i(urlContainer.parameter);
				while (i.hasNext()) {
					i.next();
					urlContainer.description += i.key()+": "+i.value()+", ";
				}
			}
			return urlContainer;
		}
	}
	return UrlContainer();
}





