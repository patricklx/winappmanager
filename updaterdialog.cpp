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
#include <QMessageBox>
#include "updaterdialog.h"
#include "ui_updaterdialog.h"
#include "applist_t.h"

UpdaterDialog::UpdaterDialog(QList<applist_t::fileinfo_t> &list,enum download_type type ) :
    QDialog(NULL),
    ui(new Ui::UpdaterDialog),
    m_list(list)
{
    ui->setupUi(this);
    m_type = type;
    if(m_type == update_version)
    {
        m_count = m_max = list.count();
        qDebug(tr("updating version: %1").arg(m_count).toAscii().data());
        ui->progressBar->setMaximum(m_max);
        ui->label->setText(tr("updating versions %1/%2").arg(m_max-m_count).arg(m_max));
        connect(&qnam, SIGNAL(finished(QNetworkReply*)),SLOT(ondownloadFinished(QNetworkReply*)));
        for(int i=0;i<list.count();i++)
        {
            qDebug("%d",i);
            list[i].info->updateVersion(&qnam);
        }
    }else
    {
        QDir dir;
        if( !dir.exists(tr("Info")) && !dir.mkdir(tr("Info")) )
        {
            QMessageBox::warning(this,tr("WinApp_Manager error"),
                                 tr("failed to create subdir \"Info\""));
            QTimer::singleShot(1000,this,SLOT(close()));
            return;
        }
        ui->label->setText(tr("fetching application list..."));

        QNetworkReply *reply;
        QString url;

        url = tr("http://appdriverupdate.sourceforge.net/Files/TreeInfo.xml");
        reply = qnam.get(QNetworkRequest(url));
        connect(reply,SIGNAL(finished()),SLOT(onCategoryTreeDownloaded()));

        url = tr("http://appdriverupdate.sourceforge.net/Files/PkgList.xml");
        reply = qnam.get(QNetworkRequest(url));
        connect(reply,SIGNAL(downloadProgress(qint64,qint64)),SLOT(onProgress(qint64,qint64)));
        connect(reply,SIGNAL(finished()),SLOT(onAppListDownloaded()));
    }
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

void UpdaterDialog::onCategoryTreeDownloaded()
{
    m_count++;
    QNetworkReply *reply = (QNetworkReply*)sender();
    QString html = reply->readAll();
    if(html.isEmpty())
    {
        qDebug("failed to get treeInfo.xml");
        return;
    }

    QFile file(tr("Info/TreeInfo.xml"));
    if( !file.open(QFile::WriteOnly) )
        qDebug("failed to open file TreeInfo.xml");

    file.write(html.toAscii());
}

void UpdaterDialog::onAppListDownloaded()
{
    qDebug("downloaded app list");
    QNetworkReply *reply = (QNetworkReply*)sender();
    reply->deleteLater();
    QString html = reply->readAll();

    QDomDocument doc;
    if( doc.setContent(html,false) == false )
    {
        qDebug("failed to process downloaded applicaion list");
        return;
    }


    QDomElement node = doc.documentElement();
    node = node.firstChildElement();

    while( !node.isNull() )
    {
        applist_t::fileinfo_t new_fileinfo;
        new_fileinfo.name = node.attribute(tr("name"));
        new_fileinfo.lastUpdate = node.attribute(tr("LastUpdate"));
        int index = m_list.indexOf(new_fileinfo);
        if( index != -1 )
        {
            if( m_list.at(index).lastUpdate != new_fileinfo.lastUpdate )
            {
                qDebug(new_fileinfo.name.toAscii());
                dlAppinfo.append(new_fileinfo);
            }
        }else
        {
            qDebug(new_fileinfo.name.toAscii());
            dlAppinfo.append(new_fileinfo);
        }
        node = node.nextSiblingElement();
    }

    m_count = m_max = dlAppinfo.count();

    if( m_type == update_check_appinfo )
    {
        done(m_max>0);
        return;
    }

    ui->progressBar->setMaximum(m_max);
    ui->label->setText(tr("updating info %1/%2").arg(m_max-m_count).arg(m_max));
    connect(&qnam, SIGNAL(finished(QNetworkReply*)),SLOT(ondownloadFinished(QNetworkReply*)));

    for(int i=0;i<dlAppinfo.count();i++)
    {
        QString url = tr("http://appdriverupdate.sourceforge.net/Files/")+dlAppinfo[i].name+tr(".xml");
        QNetworkReply *reply;
        reply = qnam.get(QNetworkRequest(url));
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
        qDebug(tr("new file info empty: %1").arg(reply->url().toString()).toAscii());
        return;
    }

    QFile file(tr("Info/")+reply->url().toString().afterLast('/'));
    if( !file.open(QFile::WriteOnly) )
    {
        qDebug(tr("unable to open file %1").arg(tr("Info/")+reply->url().toString().afterLast('/')).toAscii());
        return;
    }
    file.write(html.toAscii());
    file.close();

    applist_t::fileinfo_t f;
    QDomDocument doc;
    if( !doc.setContent(html,false) )
    {
        qDebug("failed to parse html");
        return;
    }

    f.name = doc.documentElement().attribute(tr("NAME"));
    qDebug(tr("got name:%2").arg(f.name).toAscii());

    int index = dlAppinfo.indexOf(f);
    if(index==-1)
        return;
    f = dlAppinfo[index];
    qDebug("got app info");
    newFileInfoUpdated(f.name,f.lastUpdate);
}

void UpdaterDialog::ondownloadFinished(QNetworkReply *reply)
{
    m_count--;
    reply->deleteLater();
    ui->progressBar->setValue(m_max-m_count);
    if(m_type == update_version)
        ui->label->setText(tr("updating versions %1/%2").arg(m_max-m_count).arg(m_max));
    if(m_type == update_appinfo)
        ui->label->setText(tr("updating info %1/%2").arg(m_max-m_count).arg(m_max));

    if(m_count<=0)
        QTimer::singleShot(1000,this,SLOT(accept()));
}

void UpdaterDialog::on_btCancel_clicked()
{
    close();
}
