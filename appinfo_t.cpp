#include "appinfo_t.h"
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

#if !defined(QT_DEBUG)
    QFileIconProvider iconprovider;
#endif

appinfo_t::appinfo_t(QString app_name):
    QObject(NULL)
{
    Name = app_name;
    state = ONLY_INFO;
    downloaded_id = -1;
}

appinfo_t::~appinfo_t()
{
    saveApplicationInfo();
}

bool appinfo_t::isFlagSet(appinfo_t::appinfo_flags flag)
{
    return ( (state & flag) == flag);
}

void appinfo_t::setFlag(appinfo_t::appinfo_flags flag)
{
    state = appinfo_t::appinfo_flags(state|flag);
}

void appinfo_t::unSetFlag(appinfo_t::appinfo_flags flag)
{
    state = appinfo_t::appinfo_flags(state & ~flag);
}

bool appinfo_t::newerVersionAvailable()
{
    QStringList version1,version2;
    QString v1part2,v2part2;

    QString v1 = LatestVersion;
    QString v2 = InstalledVersion.isEmpty()?DlVersion:InstalledVersion;


    if(v1.isEmpty() || v2.isEmpty())
        return false;

    int i;
    QString vers;
    for(i=0; i < v1.length() && ((v1[i]>='0' && v1[i]<='9') ||  v1[i] == '.' ); i++)
    {
        if(v1[i] != '.')
            vers = vers + v1[i];

        if(v1[i] == '.' || i == v1.length()-1)
        {
            version1.append(vers);
            vers.clear();
        }
    }
    vers.clear();
    v1part2 = v1.right(v1.length()-i);

    for(i=0; i < v2.length() && ((v2[i]>='0' && v2[i]<='9') || v2[i] == '.' ); i++)
    {
        if(v2[i] != '.')
            vers = vers + v2[i];

        if(v2[i] == '.' || i == v2.length()-1)
        {
            version2.append(vers);
            vers.clear();
        }
    }
    v2part2 = v2.right(v2.length()-i);

    i = 0;
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


void appinfo_t::updateVersion(QNetworkAccessManager *qnam)
{
    QString url = version_info.url;
    QNetworkReply *reply;
    if(qnam == NULL)
    {
        qnam = new QNetworkAccessManager;
        reply= qnam->get(QNetworkRequest(url));

    }else
    {
        reply = qnam->get(QNetworkRequest(url));
        reply->setParent(NULL);
    }

    connect(reply,SIGNAL(finished()),SLOT(onHtmlVersionDownloaded()));
}


bool appinfo_t::forceRegistryToLatestVersion()
{
    QSettings values(registry_info.search_term,QSettings::NativeFormat);
    values.setValue(tr("DisplayVersion"),LatestVersion);
    if( values.status() == QSettings::NoError )
    {
        registry_info.version = LatestVersion;
        return true;
    }
    else
        return false;
}


void appinfo_t::updateAppInfo()
{
    QString url = tr("http://appdriverupdate.sourceforge.net/Files/")+Name+tr(".xml");

    QNetworkAccessManager *qnam = new QNetworkAccessManager;

    QNetworkReply *reply = qnam->get(QNetworkRequest(url));
    connect(reply,SIGNAL(finished()),SLOT(onHtmlAppInfoDownloaded()));
}


void appinfo_t::onHtmlAppInfoDownloaded()
{
    QNetworkReply *reply = (QNetworkReply*) sender();
    QString html;
    html = reply->readAll();
    if(html.isEmpty())
    {
        emit infoUpdated(this,false);
        return;
    }
    reply->deleteLater();
    reply->manager()->deleteLater();

    QFile file(tr("Info/")+Name+tr(".xml"));

    if( !file.open(QFile::WriteOnly) )
    {
        qDebug(tr("failed to open file: %1").arg(Name).toAscii());
        emit infoUpdated(this,false);
        return;
    }

    file.write( QByteArray(html.toUtf8()));
    saveApplicationInfo();

    emit infoUpdated(this,true);
}

void appinfo_t::onHtmlVersionDownloaded()
{
    QNetworkReply *reply = (QNetworkReply*) sender();
    QString html;
    QString StartString = version_info.url_find.start;
    QString EndString = version_info.url_find.end;
    int start,end;

    html = reply->readAll();
    reply->deleteLater();

    if( reply->parent() != NULL )
    {
        reply->manager()->deleteLater();
    }

    if(html.isEmpty())
        qDebug("html empty "+reply->url().toString().toAscii());

    start = html.indexOf(StartString);
    if(start==-1)
    {
        infoUpdated(this,false);
        return;
    }

    start = start + StartString.length();
    html.remove(0, start );
    end = html.indexOf(EndString);

    if(end == -1)
    {
        infoUpdated(this,false);
        return;
    }
    QString fullversion = html.mid(0,end).beforeFirst(' ');

    QString layout = version_info.layout;
    QString version;

    int  i;
    for(i=0; i < layout.length(); i++)
    {
        if( layout[i] == 'x' || layout[i] == '.')
        {
            version += fullversion[i];
        }
    }

    LatestVersion = version;

    if(LatestVersion.isEmpty())
        LatestVersion = fullversion;

    if(!InstalledVersion.isEmpty() && newerVersionAvailable())
    {
        setFlag(UPDATE_AVAIL);
    }
    else if( InstalledVersion.isEmpty() && newerVersionAvailable() )
    {
        setFlag(UPDATE_AVAIL);
    }
    else
    {
        unSetFlag(UPDATE_AVAIL);
    }

    LastVersionCheck = QDate::currentDate().toString(tr("dd/MM/yy"));
    saveApplicationInfo();

    emit infoUpdated(this,true);
}

bool appinfo_t::contains(QStringList &strings)
{
    for(int i=0;i<strings.count();i++)
    {
        bool ok = false;
        QString s = strings.at(i).toUpper();
        if(Name.toUpper().contains(s))
            ok = true;
        if(Description.toUpper().contains(s))
            ok = true;
        if(WebPage.toUpper().contains(s))
            ok = true;
        if(Info.toUpper().contains(s))
            ok = true;

        if(!ok)
            return false;
    }
    return true;
}

appinfo_t::inet_file_t appinfo_t::getFileInfoByDlId()
{
    qDebug("downloaded_id:%d",downloaded_id);
    for(int i=0;i<inet_files.count();i++)
    {
        qDebug("id:%d",inet_files[i].id);

        if( inet_files[i].id == downloaded_id )
            return inet_files[i];
    }
    inet_file_t f;
    return f;
}

QString appinfo_t::getDownloadURL(inet_file_t &inet_file)
{
    QString url = inet_file.url;
    url.replace(tr("{version}"),LatestVersion);

    for(int i=0;i<inet_file.follow_urls.count();i++)
    {
        QUrl baseurl(url);
        QString html;
        QEventLoop eventloop;
        QString StartString = inet_file.follow_urls[i].start;
        QString EndString = inet_file.follow_urls[i].end;

        QNetworkAccessManager qnam;
        QNetworkReply *reply = qnam.get(QNetworkRequest(url));
        connect(reply,SIGNAL(finished()),&eventloop,SLOT(quit()));

        qDebug(baseurl.toString().toAscii());

        eventloop.exec();

        html += reply->readAll();

        if(!html.isEmpty())
        {
            int start,end;
            QString urlstr;
            start = html.indexOf(StartString);
            if(start==-1)  return tr("");

            start = start + StartString.length();
            html.remove(0,start);
            end = html.indexOf(EndString);

            if(end == -1) return tr("");
            urlstr = html.mid(0,end);
            QUrl url_resolve(urlstr);
            qDebug(urlstr.toAscii());

            if ( url_resolve.isRelative() )
                url_resolve = baseurl.resolved(url_resolve);

            qDebug(url_resolve.toString().toAscii());

            url = url_resolve.toString();
        }
        else
        {
            return tr("");
        }
    }
    return url;
}

QIcon appinfo_t::getIcon()
{
#if defined(QT_DEBUG)
    QFileIconProvider iconprovider;
#endif
    if( isFlagSet(SELECTED_INST_DL) )
        return QIcon(tr(":icons/Download.ico"));

    if( isFlagSet(SELECTED_REM) )
        return QIcon(tr(":icons/Remove.ico"));

    if( isFlagSet(UPDATE_AVAIL) )
        return QIcon(tr(":icons/hasupdate.ico"));

    if( isFlagSet(INSTALLED) )
    {
        if( registry_info.icon.isEmpty() )
            return QIcon(tr(":icons/IsInstalled.ico"));
        if( app_icon.isNull() )
            return QIcon(tr(":icons/IsInstalled.ico"));
        else
            return app_icon;
    }

    if( isFlagSet(DOWNLOADED) )
        return QIcon(tr(":icons/IsDL.ico"));

    if( isFlagSet(NEW) || isFlagSet(ONLY_INFO) )
        return QIcon(tr(":icons/isnew.ico"));

    return QIcon();
}


void appinfo_t::saveApplicationInfo()
{
    QFile xmldoc(tr("Info/")+Name+tr(".xml"));
    if( !xmldoc.open(QFile::ReadOnly) )
        return;

    QDomDocument doc;
    if ( !doc.setContent(xmldoc.readAll(),false) )
        return;
    xmldoc.close();

    QDomElement root = doc.documentElement();
    QDomElement node = root.firstChildElement();
    while( !node.isNull() )
    {
        if( node.tagName() == tr("APP_INFO") )
        {
            node.setAttribute(tr("DLversion"),DlVersion);
            node.setAttribute(tr("installed_version"),InstalledVersion);
            node.setAttribute(tr("id"),downloaded_id);
            node.setAttribute(tr("filename"),fileName);
            node.setAttribute(tr("version"),LatestVersion);
            node.setAttribute(tr("LastCheck"),LastVersionCheck);
            node.setAttribute(tr("isNew"),isFlagSet(NEW));
            node.setAttribute(tr("ignore_latest_version"),isFlagSet(IGNORE_LATEST));
            xmldoc.open(QFile::WriteOnly);
            xmldoc.write( doc.toString().toUtf8() );
            return;
        }
        node = node.nextSiblingElement();
    }
    //if not found add APP_INFO
    node = doc.createElement(tr("APP_INFO"));
    root.appendChild(node);

    node.setAttribute(tr("DLversion"),DlVersion);
    node.setAttribute(tr("installed_version"),InstalledVersion);
    node.setAttribute(tr("id"),downloaded_id);
    node.setAttribute(tr("filename"),fileName);
    node.setAttribute(tr("version"),LatestVersion);
    node.setAttribute(tr("LastCheck"),LastVersionCheck);
    node.setAttribute(tr("isNew"),isFlagSet(NEW));
    node.setAttribute(tr("ignore_latest_version"),isFlagSet(IGNORE_LATEST));
    xmldoc.open(QFile::WriteOnly);
    xmldoc.write( doc.toString().toUtf8() );
}

bool appinfo_t::loadFileInfo()
{
    QFile xmldoc(tr("Info/")+Name+tr(".xml"));
    if( !xmldoc.open(QFile::ReadOnly) )
    {
        qDebug() << "failed to load" << Name;
        return false;
    }

    QDomDocument doc;
    QString error;
    if ( !doc.setContent(&xmldoc,false,&error) )
    {
        qDebug()<<"failed to load" << Name << ": " << error;
        return false;
    }

    state = INVALID;

    QDomElement node = doc.documentElement();

    Description = node.attribute(tr("DESCRIPTION"));
    Info        = node.attribute(tr("INFO"));
    WebPage     = node.attribute(tr("WEBPAGE"));

    node = node.firstChildElement();

    while( !node.isNull() )
    {
        if( node.tagName() == tr("VERSION") )
        {
            version_info.url            = node.attribute(tr("url"));
            version_info.layout         = node.attribute(tr("version_layout"));
            version_info.url_find.end   = node.attribute(tr("EndString"));
            version_info.url_find.start = node.attribute(tr("StartString"));
        }
        if( node.tagName() == tr("FILELIST") )
        {
            QString url = node.attribute(tr("url"));
            QDomElement child = node.firstChildElement();

            inet_files.clear();

            while(!child.isNull())
            {
                inet_file_t tmp;
                QString url2    = child.attribute(tr("url"));
                tmp.url         = url2.isEmpty()? url : url2;
                tmp.description = child.attribute(tr("Description"));
                tmp.id          = child.attribute(tr("id")).toInt();

                QDomElement replace_str = child.firstChildElement(tr("UrlInfo"));
                while(!replace_str.isNull())
                {
                    QDomNamedNodeMap attributes = replace_str.attributes();
                    for(int i=0;i<attributes.count();i++)
                    {
                        QString replace_what = attributes.item(i).nodeName();
                        QString replace_with = attributes.item(i).nodeValue();
                        tmp.url.replace(tr("{%1}").arg(replace_what),replace_with);
                    }
                    replace_str = replace_str.nextSiblingElement(tr("UrlInfo"));
                }


                QDomElement follow_urls = child.firstChildElement(tr("FOLLOW_LINK"));
                while(!follow_urls.isNull())
                {
                    url_find_t url_find;
                    url_find.start  = follow_urls.attribute(tr("StartString"));
                    url_find.end    = follow_urls.attribute(tr("EndString"));
                    tmp.follow_urls.append(url_find);

                    follow_urls = follow_urls.firstChildElement();
                }

                inet_files.append(tmp);
                child = child.nextSiblingElement();
            }
        }

        if( node.tagName() == tr("REGISTRY") )
        {
            registry_info.icon_path              = node.attribute(tr("icon"));
            registry_info.search_term                   = node.attribute(tr("path"));
            registry_info.silent_uninstall_path  = node.attribute(tr("silent_uninstall")) ;
            registry_info.uninstall_path         = node.attribute(tr("uninstall"));
            registry_info.version_path           = node.attribute(tr("version"));
            /*path or version must be set to check if it's installed*/
        }

        if( node.tagName() == tr("INSTALL") )
        {
            install_param = node.attribute(tr("params")).trimmed();
            install_follow= node.attribute(tr("FollowPrc"));
        }
        if( node.tagName() == tr("REMOVE") )
        {
            uninstall_param = node.attribute(tr("params")).trimmed();
            uninstall_follow= node.attribute(tr("FollowPrc"));
        }

        if( node.tagName() == tr("APP_INFO") )
        {
            DlVersion           = node.attribute(tr("DLversion"));
            InstalledVersion    = node.attribute(tr("installed_version"));
            downloaded_id       = node.attribute(tr("id"),tr("-2")).toInt();
            fileName            = node.attribute(tr("filename"));
            LatestVersion       = node.attribute(tr("version"));
            LastVersionCheck    = node.attribute(tr("LastCheck"));
            bool ok = node.attribute(tr("isNew"),tr("0")).toInt();
            if(ok)setFlag(NEW);
            ok = node.attribute(tr("ignore_latest_version"),tr("0")).toInt();
            if(ok)setFlag(IGNORE_LATEST);
        }

        if( node.tagName() == tr("CATEGORYS"))
        {
            QDomElement child = node.firstChildElement();
            while(!child.isNull())
            {
                categories.append(child.attribute(tr("id")));
                child = child.nextSiblingElement();
            }
        }
        node = node.nextSiblingElement();
    }

    Path = tr("Files/");
    for (int index = 0; index<categories.count(); index++)
    {
        Path=Path + categories[index] + tr("/") ;
    }

    Path = QDir::currentPath() + tr("/") + Path + Name;

    QFileInfo qfi(Path+tr("/")+fileName);
    if(qfi.exists())
        setFlag(DOWNLOADED);

    if( ParseRegistryInfo() )
    {
        setFlag(INSTALLED);
        InstalledVersion = registry_info.version;
    }

    if( newerVersionAvailable() )
        setFlag(UPDATE_AVAIL);

    if(install_param == tr("ALWAYS"))
        setFlag(ONLY_SILENT);

    if(!isFlagSet(DOWNLOADED) && !isFlagSet(INSTALLED))
        setFlag(ONLY_INFO);

    return true;
}

bool appinfo_t::ParseRegistryInfo()
{
    QStringList regBase;
    QString path;
    if( QSysInfo::WordSize == 64 )
    {
        regBase <<  tr("HKEY_CURRENT_USER\\Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\");
        regBase <<  tr("HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\");
    }
    regBase <<  tr("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\");
    regBase <<  tr("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\");

    if( registry_info.search_term.isEmpty() && registry_info.version.isEmpty() )
        return false;

    if(registry_info.search_term.startsWith(tr("HKEY_CURRENT_USER")) || registry_info.search_term.startsWith(tr("HKEY_LOCAL_MACHINE")))
    {
        path = registry_info.search_term;
        goto next;
    }

    for(int a=0;a<regBase.count();a++)
    {
        QSettings Registry(regBase[a], QSettings::NativeFormat);
        QStringList childkeys;

        QSettings settings(registry_info.search_term.beforeLast('\\'), QSettings::NativeFormat);

        if( settings.contains( registry_info.search_term.afterLast('\\')) )
            goto next;

        if( Registry.contains( registry_info.search_term ) )
        {
            path = regBase[a] + registry_info.search_term;
            goto next;
        }

        childkeys = Registry.childGroups();
        for(int i=0;i<childkeys.count();i++)
        {
            if( childkeys.at(i).contains(registry_info.search_term) )
            {
                path = regBase[a] + childkeys[i];
                goto next;
            }
            QSettings valuesettings(regBase[a]+childkeys[i],QSettings::NativeFormat);
            QStringList valuekeys = valuesettings.childKeys();
            for(int j=0;j<valuekeys.count();j++)
            {
                if( valuesettings.value( valuekeys.at(j) ).toString().contains(registry_info.search_term) )
                {
                    path = regBase[a] + childkeys[i];
                    goto next;
                }
            }
        }

        if( !registry_info.version.isEmpty() )
        {
            QSettings settings2(registry_info.version.beforeLast('\\'), QSettings::NativeFormat);
            QString version = settings2.value(registry_info.version.afterLast('\\')).toString();

             if( !version.isEmpty())
             {
                registry_info.version = version;
                return true;
             }
        }
    }

    return false;

    next:
    path.replace(tr("/"),tr("\\"));
    QSettings values(path,QSettings::NativeFormat);

    registry_info.icon              = values.value(tr("DisplayIcon")).toString().remove(tr(",0"));
    registry_info.silent_uninstall  = values.value(tr("QuietUninstallString")).toString();
    registry_info.uninstall         = values.value(tr("UninstallString")).toString();
    registry_info.version           = values.value(tr("DisplayVersion")).toString();

    if(!registry_info.version_path.isEmpty())
    {
        QSettings values(registry_info.version_path.beforeLast('\\'),QSettings::NativeFormat);
        registry_info.version        = values.value(registry_info.version_path.afterLast('\\')).toString();
    }
    if(!registry_info.icon_path.isEmpty())
    {
        QSettings values(registry_info.icon_path.beforeLast('\\'),QSettings::NativeFormat);
        registry_info.icon        = values.value(registry_info.icon_path.afterLast('\\')).toString();
    }
    if(!registry_info.uninstall_path.isEmpty())
    {
        QSettings values(registry_info.uninstall_path.beforeLast('\\'),QSettings::NativeFormat);
        registry_info.uninstall        = values.value(registry_info.uninstall_path.afterLast('\\')).toString();
    }
    if(!registry_info.silent_uninstall_path.isEmpty())
    {
        QSettings values(registry_info.silent_uninstall_path.beforeLast('\\'),QSettings::NativeFormat);
        registry_info.silent_uninstall = values.value(registry_info.silent_uninstall_path.afterLast('\\')).toString();
    }

    if(registry_info.icon.isEmpty() && registry_info.version.isEmpty()
            && registry_info.uninstall.isEmpty() && registry_info.silent_uninstall.isEmpty() )
        return false;

    app_icon = iconprovider.icon(QFileInfo(registry_info.icon));
    return true;
}
