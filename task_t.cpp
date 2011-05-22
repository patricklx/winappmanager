#include "task_t.h"

#include <QEventLoop>
#include <QProcess>
#include <QDir>
#include <QThread>
#include <windows.h>
#include <Psapi.h>
#include <tlhelp32.h>
#include <settingsdialog.h>
#include <QApplication>

task_t::task_t(appinfo_t *appinfo,int flags):
    QThread(),
    should_quit(false)
{
    m_appinfo = appinfo;
    m_flags = flags;
}


task_t::task_t(appinfo_t *appinfo, appinfo_t::inet_file_t inet_file,int flags):
    QThread(),
    should_quit(false)
{
    m_appinfo = appinfo;
    m_inet_file = inet_file;
    m_flags = flags;
}


void task_t::waitForExternalProcess(QString PrcName)
{
    bool process_found = true;
    while(process_found)
    {
        PROCESSENTRY32* processInfo=new PROCESSENTRY32;
        HANDLE hSnapShot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
        processInfo->dwSize=sizeof(PROCESSENTRY32);

        if( Process32First(hSnapShot,processInfo) )
        {
            process_found = false;
            do
            {
                QString ProcessName;
                ProcessName = QString::fromStdWString( processInfo->szExeFile ); //Name
                if( ProcessName == PrcName)
                {
                    process_found = true;
                    break;
                }
            }while(Process32Next(hSnapShot,processInfo)!=FALSE);
        }
        QThread::msleep(250);
    }
}

bool task_t::uninstall()
{
    QProcess proc;
    QStringList args;
    QString launchcommand;
    QString arguments;
    m_current_task = UNINSTALL;
    if( isSet(SILENT) )
    {
        if(m_appinfo->registry_info.silent_uninstall.isEmpty())
        {
            if(!m_appinfo->uninstall_param.isEmpty())
            {
                launchcommand = m_appinfo->registry_info.uninstall + tr(" ") + m_appinfo->uninstall_param;;
            }else
                launchcommand = m_appinfo->registry_info.uninstall;
        }else
            launchcommand = m_appinfo->registry_info.silent_uninstall;
    }
    else
        launchcommand = m_appinfo->registry_info.uninstall;


    /*try to seperate application from parameters by seeing if the file exists*/
    if(launchcommand.contains(""""))
        launchcommand = launchcommand.remove('\"');
    {
        QString launchcmd = launchcommand;
        while(1)
        {
            QFile qfi(launchcmd);
            if( !qfi.exists() )
            {
                if(launchcmd.beforeLast(' ') == launchcmd)
                {
                    int count = launchcmd.count();
                    arguments = launchcommand.remove(0,count).trimmed();
                    launchcommand = launchcmd;
                    break;
                }

                launchcmd  = launchcmd.beforeLast(' ');
            }else
            {
                int count = launchcmd.count();
                arguments = launchcommand.remove(0,count).trimmed();
                launchcommand = launchcmd;
                break;
            }
        }
    }


    qDebug(launchcommand.toAscii());
    qDebug(arguments.toAscii());
    if(!arguments.isEmpty())
        args << tr("/c") << tr("start") << tr("""") << tr("/wait") << launchcommand << arguments.split(' ');
    else
        args << tr("/c") << tr("start") << tr("""") << tr("/wait") << launchcommand;


    proc.start(tr("cmd"),args);
    emit progress(this,-1,tr("uninstalling..."));

    //proc.start(launchcommand);
    if( !proc.waitForFinished(-1) )
    {
        emit progress(this,0,proc.errorString());
        return false;
    }

    QThread::msleep(1000);
    if(!m_appinfo->uninstall_follow.isEmpty())
        waitForExternalProcess(m_appinfo->uninstall_follow);

    QThread::msleep(1000);
    if( !m_appinfo->ParseRegistryInfo() )
    {
        m_appinfo->unSetFlag(appinfo_t::INSTALLED);
        m_appinfo->saveApplicationInfo();
        emit progress(this,100,tr("successfully uninstalled"));
        qDebug("uninstall finished");
        return true;
    }

    emit progress(this,0,tr("uninstall failed"));
    return false;
}

bool task_t::install()
{
    QProcess proc;
    QStringList args;
    QString launchCommand = m_appinfo->Path + tr("/") + m_appinfo->fileName;
    m_current_task = INSTALL;
    if( isSet(SILENT) && m_appinfo->install_param!="ALWAYS")
        args << tr("/c") << tr("start") << tr("""") << tr("/wait") << launchCommand << m_appinfo->install_param.split(" ",QString::SkipEmptyParts);
    else
        args << tr("/c") << tr("start") << tr("""") << tr("/wait") << launchCommand;

    qDebug(launchCommand.toAscii());
    qDebug(m_appinfo->install_param.toAscii());
    emit progress(this,-1,tr("installing..."));

    if(m_appinfo->Name == tr("WinApp_Manager"))
        proc.startDetached(launchCommand+tr(" ")+m_appinfo->install_param);
    else
        proc.start(tr("cmd"),args);

    if(m_appinfo->Name == tr("WinApp_Manager"))
    {
        QApplication::quit();
        emit progress(this,50,tr("updating WinApp_Manager"));
        return true;
    }

    if( !proc.waitForFinished(-1) )
    {
        emit progress(this,0,proc.errorString());
        return false;
    }

    QThread::msleep(1000);
    if(!m_appinfo->install_follow.isEmpty())
        waitForExternalProcess(m_appinfo->install_follow);
    QThread::msleep(1000);

    if( m_appinfo->ParseRegistryInfo() )
    {
        m_appinfo->setFlag( appinfo_t::INSTALLED );
        m_appinfo->saveApplicationInfo();

        if(m_appinfo->newerVersionAvailable())
        {
            emit progress(this,0,tr("install failed, wrong version - if the latest version is installed, right click and select:'force tolatest'"));
            return false;
        }

        m_appinfo->unSetFlag( appinfo_t::UPDATE_AVAIL );
        emit progress(this,100,tr("successfully installed"));
        qDebug("install finished");
        return true;
    }

    if( m_appinfo->registry_info.search_term.isEmpty() && m_appinfo->registry_info.version_path.isEmpty())
        emit progress(this,0,tr("can't check if installation was successfull!"));
    else
        emit progress(this,0,tr("install failed"));
    return false;
}

bool task_t::download()
{
    emit progress(this,0,tr("Checking latest version first..."));
    QEventLoop loop;
    connect(m_appinfo,SIGNAL(infoUpdated(appinfo_t*,bool)),&loop,SLOT(quit()));

    m_appinfo->updateVersion();
    loop.exec();

    emit progress(this,0,tr("Starting Download..."));
    m_current_task = DOWNLOAD;


    QString path = m_appinfo->Path;
    QProcess proc;
    QDir dir;
    QString url = m_appinfo->getDownloadURL(m_inet_file);
    qDebug("got url");

    if( url.isEmpty() )
    {
        emit progress(this,0,tr("Failed to resolve url: ")+m_inet_file.url);
        return false;
    }

    if( !dir.mkpath(path) )
    {
        emit progress(this,0,tr("Failed to create dir: ")+path);
        return false;
    }

    QString launchCommand =  tr("aria2c.exe --dir=\"") + path + tr("\" --seed-time=0  --human-readable --bt-stop-timeout=2   --allow-overwrite=true --auto-save-interval=10 --summary-interval=0 ");
    if(SettingsDialog::proxyEnabled())
    {
        QNetworkProxy proxy = SettingsDialog::getProxySettings();
        //[http://][USER:PASSWORD@]HOST[:PORT]
        QString proxy_settings;

        if(proxy.user().isEmpty())
        {
            proxy_settings = tr("http://%1:%2").arg(proxy.hostName()).arg(proxy.port());
        }else
            proxy_settings = tr("http://%1:%2@%3:%4 ").arg(proxy.user()).arg(proxy.password()).arg(proxy.hostName()).arg(proxy.port());
        launchCommand += proxy_settings;
    }
    launchCommand += tr("\"") + url + tr("\" ");

    proc.start(launchCommand);
    if( proc.error()==QProcess::FailedToStart || !proc.waitForStarted() )
    {
        emit progress(this,0,proc.errorString());
        return false;
    }
    if( proc.state() == QProcess::Running )
    {
        QString fullinfo;

        while ( proc.state() == QProcess::Running )
        {
            QString info;
            proc.waitForReadyRead();

            info = proc.readAll();


            fullinfo = fullinfo + info;

            if (!fullinfo.isEmpty() )
            {
                QString filename;
                int i;
                if ((i=fullinfo.indexOf(tr("Status Legend")))!=-1)
                {
                    info = fullinfo;
                    info = info.remove(0,i);
                    info = info.beforeFirst('.');
                    if (info.indexOf(tr("OK"))!=-1)
                    {

                        filename = fullinfo.afterLast('|');
                        filename = filename.beforeFirst('\n');
                        filename = filename.afterLast('/');
                        filename = filename.trimmed();

                        //delete old file
                        if( !m_appinfo->fileName.isEmpty() && m_appinfo->fileName != filename )
                        {
                            QFile file(path+tr("/")+m_appinfo->fileName);
                            if(file.exists())
                                file.remove();
                        }

                        emit progress(this,100,tr("Downloaded: ")+filename);

                        m_appinfo->fileName = filename;
                        m_appinfo->setFlag(appinfo_t::DOWNLOADED);
                        m_appinfo->unSetFlag(appinfo_t::ONLY_INFO);
                        if(!isSet(INSTALL))
                            m_appinfo->unSetFlag( appinfo_t::UPDATE_AVAIL );
                        proc.waitForFinished();
                        return true;
                    }
                    else
                    {
                        emit progress(this,0,tr("Download failed!"));
                        proc.waitForFinished();
                        return false;
                    }
                }

                info=info.afterLast('\r');

                QString prcstr;
                int t = 0;
                bool ok;

                prcstr = info.beforeFirst('%');

                if (prcstr.length()>2)
                    prcstr.remove(0,prcstr.length()-2 );

                t = prcstr.toInt(&ok);
                if( !ok )
                    prcstr.remove(0,1 );

                t = prcstr.toInt(&ok);
                if(!info.contains('\n'))
                    emit progress(this,t,info);
            }

            if( this->testDestroy() )
            {
                proc.kill();
                emit progress(this,0,tr("Download stopped by user"));
                proc.waitForFinished();
                return false;
            }
        }
    }
    return false;
}

void task_t::run()
{
    qDebug("thread started");
    if( isSet(DOWNLOAD) )
    {
        qDebug("download");
        if( download() )
        {
            unSet(DOWNLOAD);
            emit finish_success();
        }
        m_current_task = 0;
        return;
    }
    if( isSet(INSTALL) )
    {
        qDebug("install");
        if( install() )
        {
            unSet(INSTALL);
            emit finish_success();
        }
        m_current_task = 0;
        return;
    }
    if( isSet(UNINSTALL) )
    {
        qDebug("install");
        if( uninstall() )
        {
            unSet(UNINSTALL);
            emit finish_success();
        }
        m_current_task = 0;
        return;
    }
}
