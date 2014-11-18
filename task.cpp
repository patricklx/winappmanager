#include "task.h"

#include <QEventLoop>
#include <QProcess>
#include <QDir>
#include <QThread>
#include <windows.h>
#include <Psapi.h>
#include <tlhelp32.h>
#include <shlwapi.h>
#include <stdio.h>
#include <settingsdialog.h>
#include <QApplication>
#include <QSettings>
#include <QDir>
#include <QRegExp>
#include <QProcessEnvironment>

#include "utils/qstringext.h"
#include "utils/util.h"

Task::Task(AppInfo *appinfo,int flags):
	QThread(),
	should_quit(false)
{
	m_appinfo = appinfo;
	m_flags = flags;
}


Task::Task(AppInfo *appinfo, AppInfo::UrlContainer fileLocation, int flags):
	QThread(),
	should_quit(false)
{
	m_appinfo = appinfo;
	m_fileLocation = fileLocation;
	m_flags = flags;
}

bool Task::followProcessChilds(int pid)
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
					qDebug("found process: %d %s ",foundProcessPID,foundProcessName.toLatin1().data());
					emit progress(this,-1,"waiting for process: "+ProcessName);


					process_found = true;

					if(isSet(Task::INSTALL) && !m_appinfo->getInstallInfo().lastProcess.isEmpty())
					{
						if(foundProcessName.contains(m_appinfo->getInstallInfo().lastProcess, Qt::CaseInsensitive) )
						{
							continue;
						}
					}

					if(isSet(Task::UNINSTALL) && !m_appinfo->getRemoveInfo().lastProcess.isEmpty())
					{
						if(foundProcessName.contains(m_appinfo->getRemoveInfo().lastProcess, Qt::CaseInsensitive))
						{
							continue;
						}
					}

					QStringList words_continue;
					words_continue << "uninstall" << "install" << "setup";

					QStringList words;
					if(m_current_task==INSTALL)
						words << m_appinfo->name().split(" ");

					words << Utils::getDefaultBrowser();

					for(int i=0;i<words_continue.count();i++ )
					{
						if(ProcessName.contains(words_continue[i], Qt::CaseInsensitive))
						{
							qDebug("search again");
							goto follow_child;
						}
					}



					for(int i=0;i<words.count();i++ )
					{
						if(ProcessName.contains(words[i], Qt::CaseInsensitive))
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

AppInfo* Task::appInfo()
{
	return m_appinfo;
}

AppInfo::UrlContainer Task::fileLocation(){
	return m_fileLocation;
}

QTreeWidgetItem *Task::item()
{
	return m_item;
}

void Task::setItem(QTreeWidgetItem *item)
{
	m_item = item;
}

bool Task::uninstall()
{
	QProcess proc;
	QStringList args;
	QString launchcommand;
	QString arguments;
	m_current_task = UNINSTALL;
	AppInfo::RegistryInfo regInfo = m_appinfo->registryInfo();
	if( isSet(SILENT) )
	{
		if(regInfo.silentUninstall.isEmpty())
		{
			if(!m_appinfo->getRemoveInfo().silentParameter.isEmpty())
			{
				launchcommand =regInfo.uninstall + " "
						+ m_appinfo->getRemoveInfo().silentParameter;
			}else
				launchcommand = regInfo.uninstall;
		}else
			launchcommand = regInfo.silentUninstall;
	}
	else
		launchcommand = regInfo.uninstall;

	if( launchcommand.isEmpty() )
	{
		emit progress(this,0,"failed to uninstall: no uninstaller found!");
		return false;
	}


	/*try to seperate application from parameters by seeing if the file exists*/
	if(launchcommand.contains(""""))
		launchcommand = launchcommand.remove('\"');
	{
		QStringExt launchcmd = launchcommand;
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
	if(( !qfi.exists() && !Utils::existsInEnvironment(launchcommand)) || qfi.isDir())
	{
		emit progress(this,100,"is already uninstalled");
		return true;
	}


	if(m_appinfo->getRemoveInfo().needsAdmin)
	{
		if( !Utils::setRunAsAdmin(launchcommand) )
		{
			emit progress(this,0,"failed to start process as admin");
			return false;
		}
	}


	qDebug(launchcommand.toLatin1());
	qDebug(arguments.toLatin1());
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
		if(m_appinfo->getRemoveInfo().needsAdmin)
			Utils::unSetRunAsAdmin(launchcommand);
		return false;
	}

	_PROCESS_INFORMATION *pid = proc.pid();

	while(!proc.waitForFinished(0) && proc.error()==QProcess::Timedout)
	{
		if(should_quit)
			break;
		if((!m_appinfo->getRemoveInfo().lastProcess.isEmpty())
				&& launchcommand.contains(m_appinfo->getRemoveInfo().lastProcess, Qt::CaseInsensitive))
		{
			QThread::msleep(500);
		}else
		{
			followProcessChilds(pid->dwProcessId);
			QThread::msleep(150);
		}
	}

	if(m_appinfo->getRemoveInfo().needsAdmin)
		Utils::unSetRunAsAdmin(launchcommand);

	qDebug(proc.errorString().toLatin1());

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
	if( !m_appinfo->parseRegistryInfo() )
	{
		m_appinfo->unSetFlag(AppInfo::Installed);
		m_appinfo->unSetFlag(AppInfo::UpdateAvailable);
		m_appinfo->saveApplicationInfo();
		emit progress(this,100,"successfully uninstalled");
		qDebug("uninstall finished");
		return true;
	}

	emit progress(this,0,"uninstall failed");
	return false;
}

bool Task::unzip()
{
	QProcess process;
	QString filename = m_appinfo->path()+"/"+m_appinfo->fileName();
	if(m_appinfo->getInstallInfo().zipExeRegex.isEmpty()){
		QString path;
		emit askUserUnzipPath(path);
		QString cmdline = QString("7za.exe x -y -o\"%1\" ").arg(path);
		qDebug()<<cmdline;
		process.start(cmdline);

		emit progress(this,-1,tr("Extracting files from archive %1 ...").arg(m_appinfo->fileName()));
		process.waitForStarted();
		process.waitForReadyRead();
		qDebug(process.readAll());
		process.waitForFinished(-1);
		return true;
	}
	QString cmdline = QString("7za.exe x -y -o\"%1\" \"%2\"").arg(m_appinfo->path()).arg(filename);
	qDebug()<<cmdline;
	process.start(cmdline);

	emit progress(this,-1,tr("Extracting files from archive %1 ...").arg(m_appinfo->fileName()));
	process.waitForStarted();
	process.waitForReadyRead();
	qDebug(process.readAll());
	process.waitForFinished(-1);

	QDir dir(m_appinfo->path());

	QStringList namefilter;namefilter << "*.exe";
	QStringList list = dir.entryList(namefilter);
	qDebug()<<list;

	if(!m_appinfo->getInstallInfo().zipExeRegex.isEmpty() && list.isEmpty())
	{
		emit progress(this,0,"failed to extract files");
		qDebug("executable list is empty");
		return false;
	}

	if(list.count()==1)
	{
		m_appinfo->setDownloaded(m_fileLocation.id,list[0]);
		qDebug()<<m_appinfo->fileName();
		return true;
	}

	if(list.count()>1)
	{
		if(!m_appinfo->getInstallInfo().zipExeRegex.isEmpty())
		{
			int i = list.indexOf(m_appinfo->getInstallInfo().zipExeRegex);
			if(i>-1)
			{
				m_appinfo->setDownloaded(m_fileLocation.id,list[i]);
				return true;
			}
		}
	}

	int answer=-1;
	emit askUserToExecute(list,&answer);

	if(answer>-1)
	{
		m_appinfo->setDownloaded(m_fileLocation.id,list[answer]);
		return true;
	}
	emit progress(this,0,"install canceled by user");
	return false;
}


bool Task::install()
{
	QProcess proc;
	QStringList args;


	if(m_appinfo->fileName().endsWith(".zip"))
	{
		if(!unzip())
		{
			return false;
		}
	}


	QString launchCommand = m_appinfo->path() + "/" + m_appinfo->fileName();
	if(m_appinfo->getInstallInfo().needsAdmin)
	{
		if( !Utils::setRunAsAdmin(launchCommand) )
		{
			emit progress(this,0,"failed to start process as admin");
			return false;
		}
	}

	m_current_task = INSTALL;
	AppInfo::Install install = m_appinfo->getInstallInfo();
	if( isSet(SILENT) )
		args << "/c" << "start" << """" << "/wait" << launchCommand << install.silentParameter.split(" ",QString::SkipEmptyParts);
	else
		args << "/c" << "start" << """" << "/wait" << launchCommand;

	qDebug(launchCommand.toLatin1());
	qDebug(install.silentParameter.toLatin1());
	emit progress(this,-1,"installing...");

	if(m_appinfo->name() == "WinApp_Manager")
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
		if((!m_appinfo->getInstallInfo().lastProcess.isEmpty())
				&& launchCommand.contains(m_appinfo->getInstallInfo().lastProcess, Qt::CaseInsensitive))
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
			if(m_appinfo->getInstallInfo().needsAdmin)
				Utils::unSetRunAsAdmin(launchCommand);

			emit progress(this,0,proc.errorString());
			return false;
		}
	}

	if(m_appinfo->getInstallInfo().needsAdmin)
		Utils::unSetRunAsAdmin(launchCommand);

	followProcessChilds(pid->dwProcessId);
	/*QThread::msleep(1000);
	if(!m_appinfo->install_follow.isEmpty())
		waitForExternalProcess(m_appinfo->install_follow);*/
	QThread::msleep(5000);

	if( m_appinfo->parseRegistryInfo() )
	{
		m_appinfo->setFlag( AppInfo::Installed );
		m_appinfo->saveApplicationInfo();

		if(m_appinfo->newerVersionAvailable())
		{
			emit progress(this,0,"install failed: wrong version - if the latest version is installed, right click and select:'force to latest'");
			return false;
		}

		m_appinfo->unSetFlag( AppInfo::UpdateAvailable );
		emit progress(this,100,"successfully installed");
		qDebug("install finished");
		return true;
	}

	if( m_appinfo->registryInfo().displayName.isEmpty() && m_appinfo->registryInfo().versionPath.isEmpty() )
		emit progress(this,0,"can't check if installation was successfull!");
	else
		emit progress(this,0,"install failed");
	return false;
}

bool Task::download()
{
	emit progress(this,0,"Checking latest version first...");
	QEventLoop loop;
	QNetworkAccessManager manager;
	connect(m_appinfo,SIGNAL(versionUpdated(AppInfo*,bool)),&loop,SLOT(quit()));

	m_appinfo->updateVersion(&manager);
	loop.exec();

	qDebug()<<"is alread downloaded: "<<m_appinfo->isFlagSet(AppInfo::Downloaded);
	qDebug()<<"latest version: "<< m_appinfo->latestVersion()<<" downloaded: "<<m_appinfo->getUserData().downloadedVersion;

	if( m_appinfo->isFlagSet(AppInfo::Downloaded) &&
			m_appinfo->latestVersion()==m_appinfo->getUserData().downloadedVersion){
		emit progress(this,0,"Already downloaded latest version");
		return true;
	}

	emit progress(this,0,"Starting Download...");
	m_current_task = DOWNLOAD;


	QString path = m_appinfo->path();
	QProcess proc;
	QDir dir;
	QString url = m_appinfo->getDownloadURL(m_fileLocation);
	qDebug("got url");

	if( url.isEmpty() )
	{
		emit progress(this,0,"error: "+m_appinfo->lastError());
		return false;
	}

	if( !dir.mkpath(path) )
	{
		emit progress(this,0,"Failed to create dir: "+path);
		return false;
	}

	QStringList parameters;
	parameters << "--dir=\"" + path + "\"";
	parameters << "--seed-time=0";
	parameters << "--human-readable";
	parameters << "--bt-stop-timeout=2";
	parameters << "--allow-overwrite=true";
	parameters << "--auto-save-interval=10";
	parameters << "--summary-interval=0";
	parameters << "--check-certificate=false";


	if(SettingsDialog::value("PROXY_ENABLED").toBool())
	{
		QNetworkProxy proxy = SettingsDialog::getProxySettings();
		//[http://][USER:PASSWORD@]HOST[:PORT]
		QString proxy_settings;

		if(proxy.user().isEmpty())
		{
			proxy_settings = tr("http://%1:%2").arg(proxy.hostName()).arg(proxy.port());
		}else
			proxy_settings = tr("http://%1:%2@%3:%4 ").arg(proxy.user()).arg(proxy.password()).arg(proxy.hostName()).arg(proxy.port());
		parameters << proxy_settings;
	}
	parameters << "\"" + url + "\" ";

	QString launchCommand =  "aria2c.exe "+parameters.join(" ");
	qDebug()<<launchCommand;

	proc.start(launchCommand);
	if( proc.error()==QProcess::FailedToStart || !proc.waitForStarted() )
	{
		emit progress(this,0,proc.errorString());
		return false;
	}
	if( proc.state() == QProcess::Running )
	{
		QStringExt fullinfo;

		while ( proc.state() == QProcess::Running )
		{
			QStringExt info;
			proc.waitForReadyRead();

			info = proc.readAll();

			fullinfo = fullinfo + info;

			if (!fullinfo.isEmpty() )
			{
				QStringExt filename;
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
						if( !m_appinfo->fileName().isEmpty() && m_appinfo->fileName() != filename )
						{
							QFile file(path+"/"+m_appinfo->fileName());
							if(file.exists())
								file.remove();
						}

						emit progress(this,100,"Downloaded: "+filename);

						m_appinfo->setDownloaded(m_fileLocation.id,filename);
						if(!isSet(INSTALL))
							m_appinfo->unSetFlag( AppInfo::UpdateAvailable );
						proc.waitForFinished();
						return true;
					}
					else
					{
						emit progress(this,0,"Download failed! url: "+url+" ,error: "+m_appinfo->lastError());
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

void Task::run()
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
		if( m_appinfo->isFlagSet(AppInfo::Installed) && m_appinfo->getInstallInfo().needsUninstall )
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
