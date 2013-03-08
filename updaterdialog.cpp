
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
#include <QProcess>
#include <QDebug>
#include <QDomDocument>
#include <QDesktopWidget>
#include <yaml-cpp/yaml.h>
#include <QMessageBox>
#include <QWidget>
#include "settingsdialog.h"
#include "updaterdialog.h"
#include "ui_updaterdialog.h"
#include "applist.h"
#include "utils/qstringext.h"

UpdaterDialog::UpdaterDialog(QWidget *parent, QList<AppList::Fileinfo> &list,enum DownloadType type ) :
    QDialog(parent),
    ui(new Ui::UpdaterDialog),
    m_list(list)
{
    ui->setupUi(this);
    downloadFile.setFileName("all_apps.zip");
    m_type = type;
    if(m_type == UpdateVersion)
    {
        m_count = m_max = list.count();
        qDebug()<<tr("updating version: %1").arg(m_count);
        ui->progressBar->setMaximum(m_max);
        ui->label->setText(tr("updating versions %1/%2").arg(m_max-m_count).arg(m_max));
        connect(&qnam, SIGNAL(finished(QNetworkReply*)),SLOT(ondownloadFinished(QNetworkReply*)));
        for(int i=0;i<list.count();i++)
        {
            qDebug("%d",i);
            QNetworkReply *reply = list[i].appInfo->updateVersion(&qnam);
            connections.append(reply);
        }
    }
    if(m_type == DownloadAll)
    {
        QDir dir;
        if( !dir.exists("Info") || (dir.cd("Info") && dir.entryList(QDir::NoDotAndDotDot).isEmpty()) )
        {
            if( !dir.exists("Info") && !dir.mkdir("Info") )
            {
                QMessageBox::warning(this,"WinApp_Manager error",
                                     "failed to create subdir \"Info\"");
                QTimer::singleShot(1000,this,SLOT(close()));
                return;
            }
        }
        ui->label->setText("downloading all packages");
        downloadAll();
    }
    if( m_type == UpdateAppinfo)
    {
        ui->label->setText("fetching application list...");

        QNetworkReply *reply;
        QString url;

        qDebug()<<"downloading app list";
        QDateTime date = SettingsDialog::value<QDateTime>(SettingsDialog::InfoDate);
        QString dateFormated = date.toString("yyyy.MM.dd:hh.mm.ss");
        qDebug()<<"date:"<<dateFormated;
        url = "https://winappmanager-patrick.dotcloud.com/apps/list2/"+dateFormated+"/";
        qDebug()<<"update url: "<< url;

        QNetworkRequest request = QNetworkRequest(url);


        reply = qnam.get(request);
        connect(reply,SIGNAL(finished()),SLOT(onAppListDownloaded()));
    }

    connect(&timeout,SIGNAL(timeout()),SLOT(on_btCancel_clicked()));
    timeout.start(30000);
}

QStringList UpdaterDialog::getNewAppsList()
{
    return newList;
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

void UpdaterDialog::onZipDownloaded()
{
    QNetworkReply *reply = (QNetworkReply*)sender();
    QByteArray content = reply->readAll();
    QList<QByteArray> list = reply->rawHeaderList();
    foreach(QByteArray header,list)
    {
        qDebug()<<header<<": "<<reply->rawHeader(header);
    }


    if(reply->error()!=QNetworkReply::NoError){
        qDebug()<<reply->errorString();
        return;
    }

    this->downloadFile.write(content);
    onProgress(100,100);
    this->downloadFile.close();
    QProcess process;
    QString filename = "all_apps.zip";
    QString cmdline = QString("7za.exe x -y -o\"%1\" \"%2\"").arg("Info/").arg(filename);
    qDebug()<<cmdline;
    process.start(cmdline);

    process.waitForStarted();
    process.waitForReadyRead();
    qDebug(process.readAll());
    process.waitForFinished(-1);

    emit this->allFilesDownloaded();
    SettingsDialog::setValue(SettingsDialog::InfoDate,QDateTime::currentDateTime());
    QTimer::singleShot(1000,this,SLOT(accept()));

}

void UpdaterDialog::fileDownloadProgress(qint64 recv,qint64 tot)
{
    QNetworkReply *reply = (QNetworkReply*)sender();
    QByteArray content = reply->read(reply->bytesAvailable());
    this->downloadFile.write(content);
    qDebug()<<"Downloading "<<recv<<"/"<<tot;
    onProgress(recv,tot);
}

void UpdaterDialog::downloadAll()
{
    bool ok = this->downloadFile.open(QFile::WriteOnly);
    if( !ok ){
        QMessageBox::warning(this,"Error","Failed to open download file all_apps.zip");
        return;
    }
    QString url = "https://winappmanager-patrick.dotcloud.com/apps/all_apps_zipped/";
    QNetworkRequest request = QNetworkRequest(url);
    QNetworkReply *reply = qnam.get(request);

    connect(reply,SIGNAL(downloadProgress(qint64,qint64)),SLOT(fileDownloadProgress(qint64,qint64)));
    connect(reply,SIGNAL(finished()),SLOT(onZipDownloaded()));
}

void UpdaterDialog::downloadAppInfo(QString name)
{
    QString url = "https://winappmanager-patrick.dotcloud.com/apps/"+name+"/raw/";
    QNetworkReply* reply = qnam.get(QNetworkRequest(url));
    connect(reply,SIGNAL(finished()),SLOT(onNewFileInfo()));
}


void UpdaterDialog::onAppListDownloaded()
{
    qDebug("downloaded app list");
    QNetworkReply *reply = (QNetworkReply*)sender();
    reply->deleteLater();
    if( reply->error() != QNetworkReply::NoError){
        qDebug()<<"error: "<<reply->errorString();
        ui->label->setText("An error occurred:\n "+reply->errorString());
        QTimer::singleShot(5000,this,SLOT(reject()));
        return;
    }

    QString html = reply->readAll();
    if(html.isEmpty()){
        qDebug()<<reply->rawHeaderList();
        ui->label->setText("No Updates available");
        QTimer::singleShot(2000,this,SLOT(reject()));
        return;
    }

    qDebug()<<html;
    QStringList apps = html.split("\n");
    qDebug()<<apps;

    QString date = apps.takeFirst();
    qDebug()<<"date from server:"<<date;
    QDateTime datetime = QDateTime::fromString(date,"yyyy.MM.dd:hh.mm.ss");
    SettingsDialog::setValue(SettingsDialog::InfoDate,datetime);

    if(apps.isEmpty())
    {
        QTimer::singleShot(2000,this,SLOT(reject()));
        return;
    }

    connect(&qnam, SIGNAL(finished(QNetworkReply*)),SLOT(ondownloadFinished(QNetworkReply*)));
    m_count = 0;
    m_max=0;
    foreach(QString app, apps){

        this->m_count++;
        m_max++;
        downloadAppInfo(app);
    }


}

void UpdaterDialog::onNewFileInfo()
{
    QNetworkReply *reply = (QNetworkReply*)sender();
    QString html = reply->readAll();
    reply->deleteLater();

    if(html.isEmpty())
    {
        qDebug()<<tr("new file info empty: %1").arg(reply->url().toString());
        return;
    }

    QString name = ext(reply->url().toString()).afterLast("apps/").beforeLast("/raw");
    newList.append(name);
    name += ".yaml";
    qDebug()<<"Downloaded "<<name;
    QFile file("Info/"+name);
    if( !file.open(QFile::WriteOnly) )
    {
        qDebug()<<tr("unable to open file %1").arg("Info/"+ext(reply->url().toString()).afterLast('/'));
        return;
    }
    file.write(html.toUtf8());
    file.close();
}

void UpdaterDialog::ondownloadFinished(QNetworkReply *reply)
{
    m_count--;
    connections.removeOne(reply);
    reply->deleteLater();
    ui->progressBar->setValue(m_max-m_count);
    if(m_type == UpdateVersion)
        ui->label->setText(tr("updating versions %1/%2").arg(m_max-m_count).arg(m_max));
    if(m_type == UpdateAppinfo)
        ui->label->setText(tr("updating info %1/%2").arg(m_max-m_count).arg(m_max));

    if(m_count<=0)
        QTimer::singleShot(1000,this,SLOT(accept()));

    timeout.start();
}

void UpdaterDialog::on_btCancel_clicked()
{
    qDebug()<<"closed";
    while(!connections.isEmpty())
    {
        QNetworkReply *reply = connections.takeFirst();
        reply->abort();
    }
    close();
}
