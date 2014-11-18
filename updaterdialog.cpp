#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QStringList>
#include <QTimer>
#include <QUrl>
#include <QDir>
#include <QDomDocument>
#include <QDesktopWidget>
#include <QDebug>
#include <QDate>
#include <QProcess>
#include <QMessageBox>
#include "utils/qstringext.h"
#include "updaterdialog.h"
#include "ui_updaterdialog.h"
#include "applist.h"
#include "settingsdialog.h"

UpdaterDialog::UpdaterDialog(QWidget *parent, QList<AppList::Fileinfo> &list, DownloadType type ) :
    QDialog(parent),
    ui(new Ui::UpdaterDialog),
    m_list(list)
{
    ui->setupUi(this);
    m_type = type;
    if(m_type == UpdateVersion)
    {
        m_count = m_max = list.count();
        qDebug()<< QString("updating version: %1").arg(m_count);
        ui->progressBar->setMaximum(m_max);
        ui->label->setText(tr("updating versions %1/%2").arg(m_max-m_count).arg(m_max));
        connect(&qnam, SIGNAL(finished(QNetworkReply*)),SLOT(ondownloadFinished(QNetworkReply*)));
        for(int i=0;i<list.count();i++)
        {
            qDebug("%d",i);
            QNetworkReply *reply = list[i].appInfo->updateVersion(&qnam);
            connections.append(reply);
        }
    }else if( m_type == DownloadAll ){

        qDebug() << "download all";
        QString url = "http://winappmanager.herokuapp.com/apps/all_apps_zipped/";
        QNetworkReply *reply =  qnam.get(QNetworkRequest(url));
        qDebug()<<reply->errorString();
        connect(reply,SIGNAL(downloadProgress(qint64,qint64)),SLOT(onProgress(qint64,qint64)));
        connect(reply,SIGNAL(finished()),SLOT(onAllDownloaded()));
    }
    else
    {
        QDir dir;
        if( !dir.exists("Info") && !dir.mkdir("Info") )
        {
            QMessageBox::warning(this,"WinApp_Manager error",
                                 "failed to create subdir \"Info\"");
            QTimer::singleShot(1000,this,SLOT(close()));
            return;
        }
        ui->label->setText("fetching application list...");

        QNetworkReply *reply;
        QString url;

        int seconds = SettingsDialog::value("LAST_INFO_CHECK").toInt();
        qDebug() << "dowload list chances since: " << seconds;
        QString timestamp; timestamp.setNum(seconds);
        url = "http://winappmanager.herokuapp.com/apps/list3/" + timestamp+"/";
        reply = qnam.get(QNetworkRequest(url));
        connections.append(reply);
        connect(reply,SIGNAL(downloadProgress(qint64,qint64)),SLOT(onProgress(qint64,qint64)));
        connect(reply,SIGNAL(finished()),SLOT(onAppListDownloaded()));
    }
    connect(&timeout,SIGNAL(timeout()),SLOT(on_btCancel_clicked()));
    timeout.start(30000);
}

UpdaterDialog::~UpdaterDialog()
{
    delete ui;
}

void UpdaterDialog::onProgress(qint64 recv,qint64 tot)
{
    ui->progressBar->setRange(0,tot);
    ui->progressBar->setValue(recv);
}

void UpdaterDialog::onAllDownloaded()
{
    QNetworkReply *reply = (QNetworkReply*)sender();
    qDebug() << reply->errorString();
    qDebug() << reply->rawHeaderPairs();
    reply->deleteLater();
    QByteArray fileContent = reply->readAll();
    qDebug()<<fileContent;
    QFile file("all.zip");
    if( !file.open(QFile::WriteOnly) )
    {
        qDebug() << QString("unable to open file %1").arg("Info/"+reply->objectName()+".yaml");
        return;
    }
    file.write(fileContent);
    file.close();

    QProcess process;
    QStringList argumentes;
    argumentes << "e" << "all.zip" << "-oInfo/";
    process.start("7za.exe",argumentes);
    process.waitForFinished();
    done(1);
    SettingsDialog::setValue("LAST_INFO_CHECK",QDateTime::currentMSecsSinceEpoch()/1000);
}


void UpdaterDialog::onAppListDownloaded()
{
    qDebug("downloaded app list");
    QNetworkReply *reply = (QNetworkReply*)sender();
    connections.removeOne(reply);
    reply->deleteLater();
    QString content = reply->readAll();
    qDebug() << content;

    QStringList list = content.split('\n');
    QString date = list.takeFirst();

    foreach(QString name,list) {
        toDownload.append(name);
        AppList::Fileinfo temp;
        temp.name = name;
        if (m_list.indexOf(temp)==-1){
            newapps.append(name);
        }
    }


    m_count = m_max = toDownload.count();
    qDebug("downloading %d files",m_count);

    ui->progressBar->setMaximum(m_max);
    ui->label->setText(tr("updating info %1/%2").arg(m_max-m_count).arg(m_max));
    connect(&qnam, SIGNAL(finished(QNetworkReply*)),SLOT(ondownloadFinished(QNetworkReply*)));

    QString url;

    for(int i=0;i<toDownload.count();i++)
    {
        url = "http://winappmanager.herokuapp.com/apps/"+toDownload[i]+"/raw/";
        reply = qnam.get(QNetworkRequest(url));
        reply->setObjectName(toDownload[i]);
        connect(reply,SIGNAL(finished()),SLOT(onNewFileInfo()));
    }
    if(m_max == 0)
    {
        done(0);
        return;
    }
    show();
}

void UpdaterDialog::onNewFileInfo()
{
    QNetworkReply *reply = (QNetworkReply*)sender();
    QString html = reply->readAll();
    reply->deleteLater();

    if(html.isEmpty())
    {
        qDebug() << QString("new file info empty: %1").arg(reply->url().toString());
        return;
    }

    QFile file("Info/"+reply->objectName()+".yaml");
    if( !file.open(QFile::WriteOnly) )
    {
        qDebug() << QString("unable to open file %1").arg("Info/"+reply->objectName()+".yaml");
        return;
    }
    file.write(html.toLocal8Bit());
    file.close();
}

void UpdaterDialog::ondownloadFinished(QNetworkReply *reply)
{
    qDebug()<<"download finished";
    m_count--;
    connections.removeOne(reply);
    reply->deleteLater();
    ui->progressBar->setValue(m_max-m_count);
    if(m_type == UpdateVersion)
        ui->label->setText(tr("updating versions %1/%2").arg(m_max-m_count).arg(m_max));
    if(m_type == UpdateAppinfo)
        ui->label->setText(tr("updating info %1/%2").arg(m_max-m_count).arg(m_max));

    if(m_count<=0){
        QTimer::singleShot(1000,this,SLOT(accept()));
        SettingsDialog::setValue("LAST_INFO_CHECK",QDateTime::currentMSecsSinceEpoch()/1000);
    }

    timeout.start();
}

void UpdaterDialog::on_btCancel_clicked()
{
    while(!connections.isEmpty())
    {
        QNetworkReply *reply = connections.takeFirst();
        reply->abort();
    }
    close();
}


QList<QString> UpdaterDialog::getNewAppsList()
{
    return newapps;
}
