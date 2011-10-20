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
#include <QApplication>
#include <appinfo_registry_t.h>
#include <QTimer>
#include <QDomCDATASection>

#if !defined(QT_DEBUG)
QFileIconProvider iconprovider;
#endif

appinfo_t::appinfo_t(QString app_name):
    QObject(NULL)
{
    Name = app_name;
    state = INVALID;
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
    QString InstalledVersion = registry_info.version;

    QString v1 = LatestVersion;
    QString v2 = InstalledVersion.isEmpty()?DlVersion:InstalledVersion;


    if(v1.isEmpty() || v2.isEmpty())
        return false;

    if(isFlagSet(IGNORE_LATEST))
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

bool appinfo_t::forceRegistryToLatestVersion()
{
    QSettings values(registry_info.path,QSettings::NativeFormat);
    qDebug(registry_info.path.toAscii().data());
    values.setValue("DisplayVersion",LatestVersion);
    if( values.status() == QSettings::NoError )
    {
        registry_info.version = LatestVersion;
        unSetFlag(UPDATE_AVAIL);
        return true;
    }
    else
        return false;
}


QNetworkReply* appinfo_t::updateVersion(QNetworkAccessManager *qnam)
{
    QString url = version_info.url;
    QNetworkReply *reply;
    QNetworkRequest request = QNetworkRequest(url);
    if(url.contains("filehippo") && !isFlagSet(FILEHIPPO_BETA))
        request.setRawHeader("Cookie","Filter=NOBETA=1&NODEMO=0");

    if(qnam == NULL)
    {
        qnam = new QNetworkAccessManager;
        reply= qnam->get(request);

    }else
    {
        reply = qnam->get(request);
        reply->setParent(NULL);
        qDebug(reply->url().toString().toAscii());
    }

    connect(reply,SIGNAL(finished()),SLOT(onHtmlVersionDownloaded()));
    return reply;
}

void appinfo_t::updateAppInfo()
{
    QString url = "http://appdriverupdate.sourceforge.net/Apps/Files/"+Name+".xml";

    QNetworkAccessManager *qnam = new QNetworkAccessManager;

    QNetworkRequest request = QNetworkRequest(url);

    QNetworkReply *reply = qnam->get(request);
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
    if(reply->manager())
        reply->manager()->deleteLater();

    QFile file("Info/"+Name+".xml");

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
    QString version;
    QString regexString = version_info.urlRegEx;
    QRegExp regex(regexString);
    regex.setMinimal(true);


    html = reply->readAll();

    qDebug(tr("updating version of: %1").arg(Name).toAscii().data());

    if( reply->parent() != NULL )
    {
        reply->deleteLater();
        if(reply->manager())
            reply->manager()->deleteLater();
    }

    if(html.isEmpty())
        qDebug("html empty "+reply->url().toString().toAscii());

    qDebug(regexString.toAscii());

    regex.indexIn(html);
    version = regex.cap(1);

    if(version.isEmpty())
    {
        qDebug("no version found: %s",regex.errorString().toAscii().data());
        infoUpdated(this,false);
        return;
    }


    regexString = version_info.versionRegEx;
    qDebug(regexString.toAscii());
    qDebug(version.toAscii());

    regex.setPattern(regexString);
    regex.setMinimal(false);

    version = version.replace(regex,version_info.regExReplace);

    LatestVersion = version;

    if( !InstalledVersion.isEmpty() && newerVersionAvailable())
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

    LastVersionCheck = QDate::currentDate().toString("dd/MM/yy");
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

void appinfo_t::setFilehippoBeta(bool enable)
{
    if(enable)
        setFlag(FILEHIPPO_BETA);
    else
        unSetFlag(FILEHIPPO_BETA);
    saveApplicationInfo();
}

appinfo_t::file_info_t appinfo_t::getFileInfoByDlId()
{
    qDebug("downloaded_id:%d",downloaded_id);
    for(int i=0;i<inet_files.count();i++)
    {
        qDebug("id:%d",inet_files[i].id);

        if( inet_files[i].id == downloaded_id )
            return inet_files[i];
    }
    file_info_t f;
    return f;
}

QString appinfo_t::getDownloadURL(file_info_t &inet_file)
{
    QString url = inet_file.url;
    url.replace("{version}",LatestVersion);

    for(int i=0;i<inet_file.follow_urls_regex.count();i++)
    {
        QUrl baseurl(url);
        QString html;
        QEventLoop eventloop;
        QString regexString = inet_file.follow_urls_regex[i];
        QRegExp regex(regexString);
        regex.setMinimal(true);
        qDebug(regexString.toAscii());

        QNetworkAccessManager qnam;
        QNetworkRequest request = QNetworkRequest(url);
        if(url.contains("filehippo") && !isFlagSet(FILEHIPPO_BETA))
            request.setRawHeader("Cookie","Filter=NOBETA=1&NODEMO=0");
        QNetworkReply *reply = qnam.get(request);
        connect(reply,SIGNAL(finished()),&eventloop,SLOT(quit()));

        qDebug(baseurl.toString().toAscii());

        eventloop.exec();

        html = reply->readAll();

        if(!html.isEmpty())
        {
            int index = regex.indexIn(html);
            if( index == -1)
            {
                qDebug(html.toAscii());
                qDebug("regex on url failed");
                return "";
            }

            QString urlstr = regex.cap(1);

            QUrl url_resolve(urlstr);
            qDebug(urlstr.toAscii());

            if ( url_resolve.isRelative() )
                url_resolve = baseurl.resolved(url_resolve);

            qDebug(url_resolve.toString().toAscii());

            url = url_resolve.toString();
        }
        else
        {
            qDebug("html is empty");
            return "";
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
        return QIcon(":icons/Download.ico");

    if( isFlagSet(SELECTED_REM) )
        return QIcon(":icons/Remove.ico");

    if( isFlagSet(UPDATE_AVAIL) )
        return QIcon(":icons/hasupdate.ico");

    if( isFlagSet(INSTALLED) )
    {
        if( registry_info.icon.isEmpty() )
            return QIcon(":icons/IsInstalled.ico");
        if( app_icon.isNull() )
            return QIcon(":icons/IsInstalled.ico");
        else
            return app_icon;
    }

    if( isFlagSet(DOWNLOADED) )
        return QIcon(":icons/IsDL.ico");

    if( isFlagSet(NEW) || isFlagSet(ONLY_INFO) )
        return QIcon(":icons/isnew.ico");

    return QIcon(":icons/isnew.ico");
}


void appinfo_t::saveApplicationInfo()
{
    QFile xmldoc("Info/"+Name+".xml");
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
        if( node.tagName() == "APP_INFO" )
        {
            node.setAttribute("DLversion",DlVersion);
            node.setAttribute("id",downloaded_id);
            node.setAttribute("filename",fileName);
            node.setAttribute("version",LatestVersion);
            node.setAttribute("LastCheck",LastVersionCheck);
            node.setAttribute("isNew",isFlagSet(NEW));
            node.setAttribute("ignore_latest_version",isFlagSet(IGNORE_LATEST));
            node.setAttribute("filehippoBeta",isFlagSet(FILEHIPPO_BETA));
            xmldoc.open(QFile::WriteOnly);
            xmldoc.write( doc.toString().toUtf8() );
            return;
        }
        node = node.nextSiblingElement();
    }
    //if not found add APP_INFO
    node = doc.createElement("APP_INFO");
    root.appendChild(node);

    node.setAttribute("DLversion",DlVersion);
    node.setAttribute("id",downloaded_id);
    node.setAttribute("filename",fileName);
    node.setAttribute("version",LatestVersion);
    node.setAttribute("LastCheck",LastVersionCheck);
    node.setAttribute("isNew",isFlagSet(NEW));
    node.setAttribute("ignore_latest_version",isFlagSet(IGNORE_LATEST));
    node.setAttribute("filehippoBeta",isFlagSet(FILEHIPPO_BETA));
    xmldoc.open(QFile::WriteOnly);
    xmldoc.write( doc.toString().toUtf8() );
}





bool appinfo_t::loadFileInfo()
{
    QFile xmldoc("Info/"+Name+".xml");
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

    Description = node.firstChildElement("DESCRIPTION").firstChild().nodeValue();
    Info        = node.firstChildElement("INFO").firstChild().nodeValue();
    WebPage     = node.firstChildElement("WEBPAGE").firstChild().nodeValue();

    node = node.firstChildElement();

    while( !node.isNull() )
    {
        if( node.tagName() == "FILELIST" )
        {
            QString url = node.firstChildElement("url").firstChild().nodeValue();
            QDomElement child = node.firstChildElement("FILE");

            inet_files.clear();

            while(!child.isNull())
            {
                file_info_t tmp;
                QString url2    = child.firstChildElement("url").firstChild().nodeValue();

                tmp.url         = url.isEmpty()? url2 : url;
                tmp.description = child.attribute("Description");
                tmp.id          = child.attribute("id").toInt();

                QDomElement replace_str = child.firstChildElement("UrlInfo");
                while(!replace_str.isNull())
                {
                    QDomNamedNodeMap attributes = replace_str.attributes();
                    for(int i=0;i<attributes.count();i++)
                    {
                        QString replace_what = attributes.item(i).nodeName();
                        QString replace_with = attributes.item(i).nodeValue();
                        tmp.url.replace(tr("{%1}").arg(replace_what),replace_with);
                    }
                    replace_str = replace_str.nextSiblingElement("UrlInfo");
                }


                QDomElement follow_urls = child.lastChildElement("FOLLOW_LINK");
                while(!follow_urls.isNull())
                {
                    QString regex =  follow_urls.lastChildElement("regex").firstChild().nodeValue();

                    if(regex.isEmpty())
                    {
                        QString StartString = follow_urls.lastChildElement("StartString").firstChild().nodeValue();
                        QString EndString   = follow_urls.lastChildElement("EndString").firstChild().nodeValue();
                        regex               = QRegExp::escape(StartString) + "(\\S*)" + QRegExp::escape(EndString);
                    }
                    tmp.follow_urls_regex.append(regex);
                    follow_urls = follow_urls.lastChildElement("FOLLOW_LINK");
                }

                inet_files.append(tmp);
                child = child.nextSiblingElement("FILE");
            }
        }

        if( node.tagName() == "VERSION" )
        {
            version_info.url = node.lastChildElement("url").firstChild().nodeValue();
            version_info.urlRegEx = node.lastChildElement("regex").firstChild().nodeValue();


            if(version_info.urlRegEx.isEmpty())
            {
                qDebug("%s empty regex",Name.toAscii().data());
                QString StartString       = node.lastChildElement("StartString").firstChild().nodeValue();
                QString EndString         = node.lastChildElement("EndString").firstChild().nodeValue();

                version_info.urlRegEx     = QRegExp::escape(StartString) + "(.{1,20})" + QRegExp::escape(EndString);
            }
            qDebug("'%s'",version_info.urlRegEx.toAscii().data());

            version_info.versionRegEx = node.lastChildElement("versionRegEx").firstChild().nodeValue();
            if(version_info.versionRegEx.isEmpty())
                version_info.versionRegEx = "(\\S*)(.*)";

            version_info.regExReplace = node.lastChildElement("versionRegExReplace").firstChild().nodeValue();
            if(version_info.regExReplace.isEmpty())
                version_info.regExReplace = "\\1";

            if(node.attribute("filehippoBeta",0).toInt()==1)
                setFlag(FILEHIPPO_BETA);
        }

        if( node.tagName() == "REGISTRY" )
        {
            registry_info.seachValue.clear();
            registry_info.version_path.clear();
            registry_info.path.clear();
            registry_info.version.clear();

            if( QSysInfo::WordSize == 64 )
            {
                registry_info.seachValue  = node.attribute("search64").toUpper();
                registry_info.version_path = node.attribute("version64").toUpper();
            }

            if(registry_info.seachValue.isEmpty())
                registry_info.seachValue  = node.attribute("search").toUpper();


            if(registry_info.version_path.isEmpty())
                registry_info.version_path = node.attribute("version").toUpper();

            registry_info.regex        = node.attribute("versionRegEx","(\\S*)(.*)");
            registry_info.regexReplace = node.attribute("regexReplace","\\1");
            /*path or version must be set to check if it's installed*/
        }

        if( node.tagName() == "INSTALL" )
        {
            node.removeAttribute("time");
            if( node.attribute("silent").toInt()==1 )
                setFlag(ONLY_SILENT);
            if( node.attribute("admin").toInt()==1 )
                setFlag(ADMIN);
            if( node.attribute("needs_uninstall").toInt() == 1)
                setFlag(NEEDS_UNINSTALL);

            install_param = node.attribute("params").trimmed();
            install_lastProcessName = node.attribute("lastProcessName");
        }
        if( node.tagName() == "REMOVE" )
        {
            node.removeAttribute("time");
            uninstall_param = node.attribute("params").trimmed();
            uninstall_lastProcessName = node.attribute("lastProcessName");
        }

        if( node.tagName() == "APP_INFO" )
        {
            DlVersion           = node.attribute("DLversion");
            downloaded_id       = node.attribute("id","-2").toInt();
            fileName            = node.attribute("filename");
            LatestVersion       = node.attribute("version");
            LastVersionCheck    = node.attribute("LastCheck");
            bool ok = node.attribute("isNew","0").toInt();
            if(ok)setFlag(NEW);
            ok = node.attribute("ignore_latest_version","0").toInt();
            if(ok)setFlag(IGNORE_LATEST);
            ok = node.attribute("filehippoBeta","0").toInt();
            if(ok)setFlag(FILEHIPPO_BETA);
        }

        if( node.tagName() == "CATEGORYS")
        {
            QDomElement child = node.firstChildElement();
            while(!child.isNull())
            {
                QString category = child.firstChild().nodeValue();
                categories.append(category);
                child = child.nextSiblingElement();
            }
        }
        node = node.nextSiblingElement();
    }

    Path = "Files/";
    for (int index = 0; index<categories.count(); index++)
    {
        Path=Path + categories[index] + "/" ;
    }

    Path = QDir::currentPath() + "/" + Path + Name;

    if(!fileName.isEmpty())
    {
        QFileInfo qfi(Path+"/"+fileName);
        if(qfi.exists())
            setFlag(DOWNLOADED);
        else
            DlVersion.clear();
    }else
        DlVersion.clear();

    if( registry_info.seachValue.contains("\\") || registry_info.version_path.contains("\\") )
    {
        qDebug("parse registry for: %s",Name.toAscii().data());
        if(ParseRegistryInfo())
            setFlag(INSTALLED);
    }

    if( newerVersionAvailable() )
        setFlag(UPDATE_AVAIL);

    if(install_param == "ALWAYS")
        setFlag(ONLY_SILENT);

    if(!isFlagSet(DOWNLOADED))
        setFlag(ONLY_INFO);

    /*xmldoc.close();
    if( xmldoc.open(QFile::WriteOnly) )
    {
        QTextStream stream(&xmldoc);
        stream.setCodec("UTF-8");
        stream << doc.toString();
    }*/
    return true;
}


void appinfo_t::delayedSetIcon()
{
#if defined(QT_DEBUG)
    QFileIconProvider iconprovider;
#endif
    if(!QFile::exists(registry_info.icon))
        return;
    QPixmap pixmap;
    if( pixmap.load(registry_info.icon) )
        app_icon = QIcon(pixmap);
    else
        app_icon = iconprovider.icon(QFileInfo(registry_info.icon));

    emit infoUpdated(this,false);
}

void appinfo_t::setRegistryInfo(registry_group_t *reg_group)
{
    registry_info.path = reg_group->group_base+"\\"+reg_group->group_name;

    qDebug("%s: %s",Name.toAscii().data(),registry_info.path.toAscii().data());

    registry_info.icon              = reg_group->getValue("DisplayIcon").beforeLast(",");
    registry_info.silent_uninstall  = reg_group->getValue("QuietUninstallString");
    registry_info.uninstall         = reg_group->getValue("UninstallString");
    registry_info.version           = reg_group->getValue("DisplayVersion");
    registry_info.displayName	    = reg_group->displayed_name;
    if(registry_info.seachValue.isEmpty())
        registry_info.seachValue    = reg_group->displayed_name.toUpper();

    if(registry_info.version.isEmpty() && registry_info.version_path=="DISPLAYNAME")
    {
        registry_info.version = registry_info.displayName;
    }

    if(!registry_info.version_path.isEmpty() && registry_info.version_path!="DISPLAYNAME")
    {
        QSettings values(registry_info.version_path.beforeLast('\\'),QSettings::NativeFormat);
        registry_info.version        = values.value(registry_info.version_path.afterLast('\\')).toString();
    }

    if(registry_info.icon.isEmpty() && registry_info.version.isEmpty()
       && registry_info.uninstall.isEmpty() && registry_info.silent_uninstall.isEmpty() )
    {
        qDebug("no version,icon,uninstall,silentuninstall found");
        return;
    }

    QString regexString = registry_info.regex;
    QRegExp regex(regexString);
    registry_info.version = registry_info.version.replace(regex,registry_info.regexReplace);

    QTimer::singleShot((qrand()%30+5)*1000,this,SLOT(delayedSetIcon()));
    setFlag(INSTALLED);
    if( newerVersionAvailable() )
        setFlag(UPDATE_AVAIL);
    else
        unSetFlag(UPDATE_AVAIL);

    unSetFlag(ONLY_INFO);
}

bool appinfo_t::ParseRegistryInfo()
{
    QStringList regBase;
    QString path;
    if( QSysInfo::WordSize == 64 )
    {
        regBase <<  "HKEY_CURRENT_USER\\Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\";
        regBase <<  "HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\";
    }
    regBase <<  "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\";
    regBase <<  "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\";

    qDebug("search registry for: %s",registry_info.seachValue.toAscii().data());

    if( registry_info.seachValue.isEmpty() && registry_info.version.isEmpty() )
        return false;

    if(registry_info.seachValue.startsWith("HKEY_CURRENT_USER") || registry_info.seachValue.startsWith("HKEY_LOCAL_MACHINE"))
    {
        path = registry_info.seachValue;
        goto next;
    }

    for(int a=0;a<regBase.count();a++)
    {
        QSettings Registry(regBase[a], QSettings::NativeFormat);
        QStringList childkeys;

        QSettings settings(registry_info.seachValue.beforeLast('\\'), QSettings::NativeFormat);

        if( settings.contains( registry_info.seachValue.afterLast('\\')) )
            goto next;

        if( Registry.contains( registry_info.seachValue ) )
        {
            path = regBase[a] + registry_info.seachValue;
            goto next;
        }

        childkeys = Registry.childGroups();
        for(int i=0;i<childkeys.count();i++)
        {
            if( childkeys.at(i).toUpper().contains(registry_info.seachValue) )
            {
                path = regBase[a] + childkeys[i];
                goto next;
            }
            QSettings valuesettings(regBase[a]+childkeys[i],QSettings::NativeFormat);
            QStringList valuekeys = valuesettings.childKeys();
            for(int j=0;j<valuekeys.count();j++)
            {
                if( valuesettings.value( valuekeys.at(j) ).toString().toUpper().contains(registry_info.seachValue) )
                {
                    path = regBase[a] + childkeys[i];
                    goto next;
                }
            }
        }

        if( !registry_info.version_path.isEmpty() )
        {
            QSettings settings2(registry_info.version_path.beforeLast('\\'), QSettings::NativeFormat);
            QString version = settings2.value(registry_info.version_path.afterLast('\\')).toString();

            if( !version.isEmpty())
            {
                QString regexString = registry_info.regex;
                QRegExp regex(regexString);
                version = version.replace(regex,registry_info.regexReplace);
                registry_info.version = version;

                setFlag(INSTALLED);
                if( newerVersionAvailable() )
                    setFlag(UPDATE_AVAIL);
                else
                    unSetFlag(UPDATE_AVAIL);

                unSetFlag(ONLY_INFO);
                return true;
            }
        }
    }

    return false;

next:
    path.replace("/","\\");
    qDebug("registry found: %s",path.toAscii().data());
    registry_info.path = path;
    QSettings values(path,QSettings::NativeFormat);

    registry_info.icon              = values.value("DisplayIcon").toString().remove(",0");
    registry_info.silent_uninstall  = values.value("QuietUninstallString").toString();
    registry_info.uninstall         = values.value("UninstallString").toString();
    registry_info.version           = values.value("DisplayVersion").toString();
    registry_info.displayName	    = values.value("DisplayName").toString();

    if(registry_info.version.isEmpty() && registry_info.version_path=="DisplayName")
    {
        registry_info.version = registry_info.displayName;
    }

    if(!registry_info.version_path.isEmpty() && registry_info.version_path!="DisplayName")
    {
        QSettings values(registry_info.version_path.beforeLast('\\'),QSettings::NativeFormat);
        registry_info.version        = values.value(registry_info.version_path.afterLast('\\')).toString();
    }

    if(registry_info.icon.isEmpty() && registry_info.version.isEmpty()
       && registry_info.uninstall.isEmpty() && registry_info.silent_uninstall.isEmpty() )
        return false;

    QString regexString = registry_info.regex;
    QRegExp regex(regexString);
    registry_info.version = registry_info.version.replace(regex,registry_info.regexReplace);

#if defined(QT_DEBUG)
    QFileIconProvider iconprovider;
#endif

    QTimer::singleShot((qrand()%30+5)*1000,this,SLOT(delayedSetIcon()));
    setFlag(INSTALLED);
    if( newerVersionAvailable() )
        setFlag(UPDATE_AVAIL);
    else
        unSetFlag(UPDATE_AVAIL);

    unSetFlag(ONLY_INFO);
    return true;
}

