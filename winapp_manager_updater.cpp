#include "winapp_manager_updater.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <settingsdialog.h>
#include <QProcess>
#include <QDir>
#include <QApplication>


/*
This class updates the winapp_manager portable version
*/

QString toReadableSpeed(double size)
{
    double floatSize = size;
    QString filesize;
        if (floatSize<1000)
        {
            filesize.sprintf("%.2fB/s", floatSize);
                return( filesize );
        }
        floatSize = floatSize / 1000.0;
        if (floatSize<1000)
        {
            filesize.sprintf("%.2fKb/s", floatSize);
                return( filesize );
        }
        floatSize = floatSize / 1000.0;
        if (floatSize<1000)
        {
            filesize.sprintf("%.2fMb/s", floatSize);
                return( filesize );
        }
        floatSize = floatSize / 1000.0;

        if (floatSize<1000)
        {
            filesize.sprintf("%.2fGb/s", floatSize);
                return( filesize );
        }
        floatSize = floatSize / 1000.0;

        if (floatSize<1000)
        {
            filesize.sprintf("%.2fTb/s", floatSize);
                return( filesize );
        }

        return "invalid speed";
}

QString toReadableSize(double size)
{
    double floatSize = size;
    QString filesize;
        if (floatSize<1000)
        {
            filesize.sprintf("%.3fB", floatSize);
                return( filesize );
        }
        floatSize = floatSize / 1000.0;
        if (floatSize<1000)
        {
            filesize.sprintf("%.3fKb", floatSize);
                return( filesize );
        }
        floatSize = floatSize / 1000.0;
        if (floatSize<1000)
        {
            filesize.sprintf("%.3fMb", floatSize);
                return( filesize );
        }
        floatSize = floatSize / 1000.0;

        if (floatSize<1000)
        {
            filesize.sprintf("%.3fGb", floatSize);
                return( filesize );
        }

        floatSize = floatSize / 1000.0;

        if (floatSize<1000)
        {
            filesize.sprintf("%.3fTb", floatSize);
                return( filesize );
        }

        return "invalid filesize";
}



winapp_manager_updater::winapp_manager_updater()
    :QDialog(),ui(new Ui_UpdaterDialog)
{
    ui->setupUi(this);
    this->setModal(true);
    reply = NULL;

    QFont font;
    font.setFamily(QString::fromUtf8("Courier New"));
    font.setPointSize(10);
    font.setBold(false);
    font.setItalic(false);
    ui->label->setFont(font);

    QFile::remove("WinApp_Manger.zip");

    update_timer.setInterval(3*60*60*1000);
    connect(&update_timer,SIGNAL(timeout()),SLOT(check()));
    connect(ui->btCancel,SIGNAL(clicked()),SLOT(cancel()));
}


winapp_manager_updater::~winapp_manager_updater()
{
    delete ui;
    close();
}

void winapp_manager_updater::check()
{
    QUrl url = QUrl("http://appdriverupdate.sourceforge.net/Files/Updates/WinApp_Manager.txt");
    if(reply!=NULL)
        return;
    reply = qnam.get(QNetworkRequest(url));
    connect(reply,SIGNAL(finished()),SLOT(check_version()));
}

void winapp_manager_updater::check_version()
{
    QString html = reply->readAll();
    reply->deleteLater();
    reply->disconnect();
    reply = NULL;

    QRegExp regex("Version< ([^ ]*) >");
    regex.setMinimal(true);

    regex.indexIn(html);
    QString version = regex.cap(1);

    if(version.isEmpty())
    {
        emit no_new_version("failed to get latest version info, check you internet connection");
        return;
    }


    QString act_version = SettingsDialog::currentVersion();
    qDebug() << act_version;
    latest_version = version;
    if(version == act_version)
    {
        qDebug("same version found");
        emit no_new_version("no new version available");
        return;
    }

    emit newer_version_available();
}

QUrl redirectUrl(const QUrl& possibleRedirectUrl, const QUrl& oldRedirectUrl)
{
    QUrl redirectUrl;
    if(!possibleRedirectUrl.isEmpty() && possibleRedirectUrl != oldRedirectUrl)
        redirectUrl = possibleRedirectUrl;
    return redirectUrl;
}

void winapp_manager_updater::showProgress()
{
    WinApp_Manager_File = new QFile("Winapp_Manager_portable.zip");
    if(!WinApp_Manager_File->open(QFile::WriteOnly))
    {
        qDebug("can't open file");
        this->hide();
        reply->abort();
        reply->deleteLater();
        reply->disconnect();
        reply = NULL;
        return;
    }

    connect(reply,SIGNAL(error(QNetworkReply::NetworkError)),SLOT(cancel()));

    connect(reply,SIGNAL(downloadProgress(qint64,qint64)),SLOT(setProgress(qint64,qint64)));
    connect(reply,SIGNAL(finished()),SLOT(downloaded()));

    timer.start();
    show();

}

void winapp_manager_updater::getRedirect()
{
    if(reply==NULL)
        return;
    QVariant possibleRedirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if(!possibleRedirectUrl.toString().isEmpty())
    {
        qDebug("redirect");
        qDebug() << possibleRedirectUrl;
        reply->deleteLater();
        reply->disconnect();
        reply = qnam.get(QNetworkRequest(possibleRedirectUrl.toString()));
        showProgress();
        return;
    }
    QString html = reply->readAll();
    if(html.isEmpty())
        cancel();
    QRegExp regex("url=([^\"]*)\"");
    regex.indexIn(html);

    QString url = regex.cap(1);
    url.replace("&amp;","&");
    qDebug("follow");
    qDebug() << url;

    reply->deleteLater();
    reply->disconnect();
    reply = qnam.get(QNetworkRequest(url));
    connect(reply,SIGNAL(finished()),SLOT(getRedirect()));
}

void winapp_manager_updater::download()
{
    QString link ;
    link = "http://sourceforge.net/projects/appdriverupdate/files/WinApp_Manager/winapp_manager_portable_32_64_{ver}.zip/download";
    link.replace("{ver}",latest_version);

    if(reply!=NULL)
        return;

    qDebug() << link;
    QUrl url = QUrl(link);
    reply = qnam.get(QNetworkRequest(url));
    connect(reply,SIGNAL(finished()),SLOT(getRedirect()));

}

void winapp_manager_updater::downloaded()
{
    qDebug("downloaded");

    QProcess proc;
    QStringList args;
    QString launchCommand = QDir::currentPath()+"/portableUpdater.bat";


    args << "/c" << "start" << """" << launchCommand;
    proc.startDetached("cmd",args);
    if(reply == NULL)
        return;

    if(WinApp_Manager_File==NULL)
        return;

    WinApp_Manager_File->close();
    WinApp_Manager_File->deleteLater();
    WinApp_Manager_File = NULL;

    hide();
    QApplication::exit();
    reply->deleteLater();
    reply = NULL;
}

void winapp_manager_updater::cancel()
{
    qDebug("abort");
    this->hide();
    if(reply == NULL)
        return;

    reply->disconnect();
    reply->abort();
    reply->deleteLater();
    reply->disconnect();
    reply=NULL;

    if(WinApp_Manager_File==NULL)
        return;

    WinApp_Manager_File->close();
    WinApp_Manager_File->deleteLater();
    WinApp_Manager_File = NULL;
}

void winapp_manager_updater::setProgress(qint64 recieved, qint64 total)
{
    if(reply==NULL || reply->isFinished())
    {
        qDebug("reply is NULL??");
        return;
    }

    QByteArray array = reply->readAll();
    if(array.isEmpty())
    {
        qDebug("nothing recieved");
        return;
    }
    WinApp_Manager_File->write(array,array.count());

    int time = timer.elapsed();

    QString speed = toReadableSpeed(recieved/time*1000);
    QString done = toReadableSize(recieved);
    QString size = toReadableSize(total);

    ui->label->setText(tr("Downloading update... [%1 / %2] ( %3 )").arg(done).arg(size).arg(speed));

    ui->progressBar->setValue(100.0*recieved/total);
    qDebug()<<"progress:"<<100.0*recieved/total;
}


void winapp_manager_updater::activate()
{
    update_timer.start();
    check();
}
