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
#include <QSettings>
#include <QDir>
#include <QRegExp>
#include <QProcessEnvironment>
#include <windows.h>
#include <shlwapi.h>
#include <stdio.h>

task_t::task_t(appinfo_t *appinfo,int flags):
    QThread(),
    should_quit(false)
{
    m_appinfo = appinfo;
    m_flags = flags;
}


task_t::task_t(appinfo_t *appinfo, appinfo_t::file_info_t inet_file,int flags):
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
        PROCESSENTRY32 processInfo;
        HANDLE hSnapShot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
        processInfo.dwSize=sizeof(PROCESSENTRY32);

        if( Process32First(hSnapShot,&processInfo) )
        {
            process_found = false;
            do
            {
                QString ProcessName;
                ProcessName = QString::fromStdWString( processInfo.szExeFile ); //Name
                if( ProcessName == PrcName)
                {
                    process_found = true;
                    break;
                }
            }while(Process32Next(hSnapShot,&processInfo)!=FALSE);
        }
        QThread::msleep(250);
    }
}

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof((arr)[0]))
QString getDefaultBrowser()
{
    HRESULT hr;
    TCHAR szExe[MAX_PATH + 100];
    DWORD cchExe = ARRAY_SIZE(szExe);

    if (SUCCEEDED(hr = AssocQueryString((ASSOCF)0, ASSOCSTR_EXECUTABLE,
                                        TEXT("http"), TEXT("open"), szExe, &cchExe)))
    {
        return QString::fromStdWString(szExe).afterLast("\\");
    }else
        return "";
}


bool existsInEnvironment(QString file)
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment ();
    QString PATH = env.value("PATH");
    QStringList list = PATH.split(';');

    for(int i=0;i < list.count();i++)
    {
        QString path = list.at(i);
        if(!path.endsWith('/'))
            path.append("/");

        QFile qfi(path+file);
        if( qfi.exists() )
        {
            return true;
        }
    }
    return false;
}

bool task_t::followProcessChilds(int pid)
{
    bool process_found = false;
    QTime watch;
    watch.start();
    qDebug("searching for: %d",pid);
    int foundProcessPID = -1;
    while(watch.elapsed()<2000)
    {
	if(should_quit)
	    break;
        PROCESSENTRY32 processInfo;
        HANDLE hSnapShot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
        processInfo.dwSize=sizeof(PROCESSENTRY32);

        if( Process32First(hSnapShot,&processInfo) )
        {
            do
            {
		if(should_quit)
		    break;
                int ppid = processInfo.th32ParentProcessID;
                if(ppid == pid)
                {
                    watch.restart();
                    watch.addMSecs(1800);

                    QString foundProcessName = QString::fromStdWString( processInfo.szExeFile ); //Name
                    if(foundProcessPID == -1)
                    {
                        foundProcessPID = processInfo.th32ProcessID;
                    }else
                    {
                        if(foundProcessPID != (int)processInfo.th32ProcessID)
                        {
                            qDebug("process pid changed %d -> %d",foundProcessPID, (int)processInfo.th32ProcessID);
                            return process_found;
                        }
                    }
                    QString ProcessName = foundProcessName;
                    qDebug("found process: %d %s ",foundProcessPID,foundProcessName.toAscii().data());
                    emit progress(this,-1,"waiting for process: "+ProcessName);


                    process_found = true;

                    if(isSet(task_t::INSTALL) && !m_appinfo->install_lastProcessName.isEmpty())
                    {
                        if(foundProcessName.contains(m_appinfo->install_lastProcessName) )
                        {
                            continue;
                        }
                    }

                    if(isSet(task_t::UNINSTALL) && !m_appinfo->uninstall_lastProcessName.isEmpty())
                    {
                        if(foundProcessName.contains(m_appinfo->uninstall_lastProcessName))
                        {
                            continue;
                        }
                    }

                    QStringList words_continue;
                    words_continue << "uninstall" << "install" << "setup";

                    QStringList words;
                    if(m_current_task==INSTALL)
                        words << m_appinfo->Name.toUpper().split(" ");

                    words << getDefaultBrowser().toUpper();

                    for(int i=0;i<words_continue.count();i++ )
                    {
                        if(ProcessName.toLower().contains(words_continue[i]))
                        {
                            qDebug("search again");
                            goto follow_child;
                        }
                    }



                    for(int i=0;i<words.count();i++ )
                    {
                        if(ProcessName.toUpper().contains(words[i]))
                        {
                            qDebug("found last process");
                            return process_found;
                        }
                    }

                    follow_child:
                    followProcessChilds(processInfo.th32ProcessID);

                    watch.restart();
                    watch.addMSecs(1800);
                }
            }while(Process32Next(hSnapShot,&processInfo)!=FALSE);
        }
        QThread::msleep(100);
    }
    qDebug("timed out, found: %d",process_found);
    return process_found;
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
                launchcommand = m_appinfo->registry_info.uninstall + " " + m_appinfo->uninstall_param;;
            }else
                launchcommand = m_appinfo->registry_info.uninstall;
        }else
            launchcommand = m_appinfo->registry_info.silent_uninstall;
    }
    else
        launchcommand = m_appinfo->registry_info.uninstall;

    if( launchcommand.isEmpty() )
    {
        emit progress(this,0,"failed to uninstall: no uninstaller found!");
        return false;
    }


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

    QFileInfo qfi(launchcommand);
    if(( !qfi.exists() && !existsInEnvironment(launchcommand)) || qfi.isDir())
    {
        emit progress(this,100,"is already uninstalled");
        return true;
    }


    if(m_appinfo->isFlagSet(appinfo_t::ADMIN))
    {
	if( !setRunAsAdmin(launchcommand) )
	{
	    emit progress(this,0,"failed to start process as admin");
	    return false;
	}
    }


    qDebug(launchcommand.toAscii());
    qDebug(arguments.toAscii());
    if(!arguments.isEmpty())
        args << "/c" << "start" << """" << "/wait" << launchcommand << arguments.split(' ');
    else
        args << "/c" << "start" << """" << "/wait" << launchcommand;


    proc.start("cmd",args);
    emit progress(this,-1,"uninstalling...");

    //proc.start(launchcommand);
    if( !proc.waitForStarted() )
    {
        emit progress(this,0,proc.errorString());
	if(m_appinfo->isFlagSet(appinfo_t::ADMIN))
	    unSetRunAsAdmin(launchcommand);
        return false;
    }

    _PROCESS_INFORMATION *pid = proc.pid();

    while(!proc.waitForFinished(0) && proc.error()==QProcess::Timedout)
    {
	if(should_quit)
	    break;
        if((!m_appinfo->uninstall_lastProcessName.isEmpty()) && launchcommand.contains(m_appinfo->uninstall_lastProcessName))
        {
            QThread::msleep(500);
        }else
        {
            followProcessChilds(pid->dwProcessId);
            QThread::msleep(150);
        }
    }

    if(m_appinfo->isFlagSet(appinfo_t::ADMIN))
	unSetRunAsAdmin(launchcommand);

    qDebug(proc.errorString().toAscii());

    if(!should_quit)
    {
	if( !proc.waitForFinished(-1) && proc.state()==QProcess::Running )
	{
	    emit progress(this,0,proc.errorString());
	    return false;
	}
    }



    /*QThread::msleep(1000);
    if(!m_appinfo->uninstall_follow.isEmpty())
        waitForExternalProcess(m_appinfo->uninstall_follow);*/

    QThread::msleep(1000);
    if( !m_appinfo->ParseRegistryInfo() )
    {
        m_appinfo->unSetFlag(appinfo_t::INSTALLED);
	m_appinfo->unSetFlag(appinfo_t::UPDATE_AVAIL);
        m_appinfo->saveApplicationInfo();
        emit progress(this,100,"successfully uninstalled");
        qDebug("uninstall finished");
        return true;
    }

    emit progress(this,0,"uninstall failed");
    return false;
}

bool task_t::setRunAsAdmin(QString path)
{
    HKEY key;
    LONG result = RegCreateKey(HKEY_CURRENT_USER,
                                      L"Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers\\",
                                      &key);

    if( result != ERROR_SUCCESS)
        return false;

    path.replace("/","\\");
    result = RegSetValueExA(key,path.toAscii(),0,REG_SZ,
                            (const BYTE*)"RUNASADMIN",
                            11);

    RegCloseKey(key);
    if( result != ERROR_SUCCESS)
        return false;

    return true;
}

void task_t::unSetRunAsAdmin(QString path)
{
    HKEY key;
    LONG result = RegOpenKeyExA(HKEY_CURRENT_USER,"Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers\\",
                                       0,KEY_WRITE,
                                       &key);

    if( result != ERROR_SUCCESS)
    {
        qDebug("failed to delete Key: %d",(int)result);
        return;
    }

    path.replace("/","\\");
    result = RegDeleteValueA(key,path.toAscii());
    RegCloseKey(key);

    if( result != ERROR_SUCCESS)
        qDebug("failed to delete Key: %d",(int)result);
}


bool task_t::unzip()
{
    QProcess process;
    QString filename = m_appinfo->Path+"/"+m_appinfo->fileName;
    QString cmdline = QString("7za.exe x -y -o\"%1\" \"%2\"").arg(m_appinfo->Path).arg(filename);
    qDebug()<<cmdline;
    process.start(cmdline);

    emit progress(this,-1,tr("Extracting files from archive %1 ...").arg(m_appinfo->fileName));
    process.waitForStarted();
    process.waitForReadyRead();
    qDebug(process.readAll());
    process.waitForFinished(-1);

    QDir dir(m_appinfo->Path);

    QStringList namefilter;namefilter << "*.exe";
    QStringList list = dir.entryList(namefilter);
    qDebug()<<list;

    if(list.isEmpty())
    {
	emit progress(this,0,"failed to extract files");
	qDebug("executable list is empty");
	return false;
    }

    if(list.count()==1)
    {
	m_appinfo->fileName = list[0];
	qDebug()<<m_appinfo->fileName;
	return true;
    }

    if(list.count()>1)
    {
	if(!m_appinfo->zipExeRegex.isEmpty())
	{
	    int i = list.indexOf(m_appinfo->zipExeRegex);
	    if(i>-1)
	    {
		m_appinfo->fileName = list[i];
		return true;
	    }
	}
    }

    int answer=-1;
    emit askUserToExecute(list,&answer);

    if(answer>-1)
    {
	m_appinfo->fileName = list[answer];
	return true;
    }
    emit progress(this,0,"install caneled by user");
    return false;
}


bool task_t::install()
{
    QProcess proc;
    QStringList args;


    if(m_appinfo->fileName.endsWith(".zip"))
    {
	if(!unzip())
	{
	    return false;
	}
    }


    QString launchCommand = m_appinfo->Path + "/" + m_appinfo->fileName;
    if(m_appinfo->isFlagSet(appinfo_t::ADMIN))
    {
        if( !setRunAsAdmin(launchCommand) )
        {
            emit progress(this,0,"failed to start process as admin");
            return false;
        }
    }

    m_current_task = INSTALL;
    if( isSet(SILENT) && m_appinfo->install_param!="ALWAYS")
        args << "/c" << "start" << """" << "/wait" << launchCommand << m_appinfo->install_param.split(" ",QString::SkipEmptyParts);
    else
        args << "/c" << "start" << """" << "/wait" << launchCommand;

    qDebug(launchCommand.toAscii());
    qDebug(m_appinfo->install_param.toAscii());
    emit progress(this,-1,"installing...");

    if(m_appinfo->Name == "WinApp_Manager")
    {
        args.removeOne("/wait");
        if( proc.startDetached("cmd",args) )
        {
            QApplication::quit();
            return true;
        }else
        {
            emit progress(this,0,"failed to start the WinApp_Manager Setup: "+proc.errorString());
            return false;
        }
    }
    else
        proc.start("cmd",args);

    if( !proc.waitForStarted() )
    {
        emit progress(this,0,proc.errorString());
        return false;
    }
    _PROCESS_INFORMATION *pid = proc.pid();

    while(!proc.waitForFinished(0) && proc.error()==QProcess::Timedout)
    {
	if(should_quit)
	    break;
        if((!m_appinfo->install_lastProcessName.isEmpty()) && launchCommand.contains(m_appinfo->install_lastProcessName))
        {
            QThread::msleep(500);
        }else
        {
            followProcessChilds(pid->dwProcessId);
            QThread::msleep(150);
        }
    }

    if(!should_quit)
    {
	if(proc.state()==QProcess::Running && !proc.waitForFinished(-1) )
	{
	    if(m_appinfo->isFlagSet(appinfo_t::ADMIN))
		unSetRunAsAdmin(launchCommand);

	    emit progress(this,0,proc.errorString());
	    return false;
	}
    }

    if(m_appinfo->isFlagSet(appinfo_t::ADMIN))
        unSetRunAsAdmin(launchCommand);

    followProcessChilds(pid->dwProcessId);
    /*QThread::msleep(1000);
    if(!m_appinfo->install_follow.isEmpty())
        waitForExternalProcess(m_appinfo->install_follow);*/
    QThread::msleep(1000);

    if( m_appinfo->ParseRegistryInfo() )
    {
        m_appinfo->setFlag( appinfo_t::INSTALLED );
        m_appinfo->saveApplicationInfo();

        if(m_appinfo->newerVersionAvailable())
        {
            emit progress(this,0,"install failed: wrong version - if the latest version is installed, right click and select:'force to latest'");
            return false;
        }

        m_appinfo->unSetFlag( appinfo_t::UPDATE_AVAIL );
        emit progress(this,100,"successfully installed");
        qDebug("install finished");
        return true;
    }

    if( m_appinfo->registry_info.seachValue.isEmpty() && m_appinfo->registry_info.version_path.isEmpty())
        emit progress(this,0,"can't check if installation was successfull!");
    else
        emit progress(this,0,"install failed");
    return false;
}

bool task_t::download()
{
    emit progress(this,0,"Checking latest version first...");
    QEventLoop loop;
    QNetworkAccessManager manager;
    connect(m_appinfo,SIGNAL(infoUpdated(appinfo_t*,bool)),&loop,SLOT(quit()));

    m_appinfo->updateVersion(&manager);
    loop.exec();

    emit progress(this,0,"Starting Download...");
    m_current_task = DOWNLOAD;


    QString path = m_appinfo->Path;
    QProcess proc;
    QDir dir;
    QString url = m_appinfo->getDownloadURL(m_inet_file);
    qDebug("got url");

    if( url.isEmpty() )
    {
        emit progress(this,0,"Failed to resolve url: "+m_inet_file.url);
        return false;
    }

    if( !dir.mkpath(path) )
    {
        emit progress(this,0,"Failed to create dir: "+path);
        return false;
    }

    QString launchCommand =  "aria2c.exe --dir=\"" + path + "\" --seed-time=0  --human-readable --bt-stop-timeout=2   --allow-overwrite=true --auto-save-interval=10 --summary-interval=0 ";

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
    launchCommand += "\"" + url + "\" ";

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
                if ((i=fullinfo.indexOf("Status Legend"))!=-1)
                {
                    info = fullinfo;
                    info = info.remove(0,i);
                    info = info.beforeFirst('.');
                    if (info.indexOf("OK")!=-1)
                    {

                        filename = fullinfo.afterLast('|');
                        filename = filename.beforeFirst('\n');
                        filename = filename.afterLast('/');
                        filename = filename.trimmed();

                        //delete old file
                        if( !m_appinfo->fileName.isEmpty() && m_appinfo->fileName != filename )
                        {
                            QFile file(path+"/"+m_appinfo->fileName);
                            if(file.exists())
                                file.remove();
                        }

                        emit progress(this,100,"Downloaded: "+filename);

                        m_appinfo->fileName = filename;
			m_appinfo->DlVersion = m_appinfo->LatestVersion;
                        m_appinfo->setFlag(appinfo_t::DOWNLOADED);
                        m_appinfo->unSetFlag(appinfo_t::ONLY_INFO);
                        if(!isSet(INSTALL))
                            m_appinfo->unSetFlag( appinfo_t::UPDATE_AVAIL );
                        proc.waitForFinished();
                        return true;
                    }
                    else
                    {
                        emit progress(this,0,"Download failed!");
                        proc.waitForFinished(3000);
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
                    prcstr.remove(0,1);

                t = prcstr.toInt(&ok);
                if(!info.contains('\n'))
                    emit progress(this,t,info);
            }

            if( this->testDestroy() )
            {
                proc.kill();
                emit progress(this,0,"Download stopped by user");
                proc.waitForFinished();
                return false;
            }
        }
    }
    emit progress(this,0,"an unkown error occurred! Delete the downloaded files in the containing folder and try again");
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
        }
        m_current_task = 0;
        should_quit = false;
        return;
    }

    if( isSet(UNINSTALL) )
    {
        qDebug("Uninstall");
        if( uninstall() )
        {
            unSet(UNINSTALL);
        }
        m_current_task = 0;
        return;
    }

    if( isSet(INSTALL) )
    {
        if( m_appinfo->isFlagSet(appinfo_t::INSTALLED) && m_appinfo->isFlagSet(appinfo_t::NEEDS_UNINSTALL) )
            uninstall();
        qDebug("install");
        if( install() )
        {
            unSet(INSTALL);
        }
        m_current_task = 0;
        return;
    }

}
