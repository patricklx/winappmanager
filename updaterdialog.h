#ifndef UPDATERDIALOG_H
#define UPDATERDIALOG_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QTimer>
#include "applist.h"

namespace Ui {
    class UpdaterDialog;
}

class UpdaterDialog : public QDialog
{
    Q_OBJECT

public:
    enum DownloadType{
        UpdateVersion,
        UpdateAppinfo,
        DownloadAll
    };

    QList<QString> getNewAppsList();

    explicit UpdaterDialog(QWidget *parent,QList<AppList::Fileinfo> &list,enum DownloadType type);
    ~UpdaterDialog();


signals:
    void newFileInfoUpdated(QString name,QString lastUpdate);

private slots:
    void on_btCancel_clicked();

private:
    Ui::UpdaterDialog *ui;
    QList<AppList::Fileinfo> &m_list;
    QList<QString> newapps;
    QList<QString> toDownload;
    QList<QNetworkReply*> connections;
    QTimer timeout;
    int m_count;
    int m_max;

    QNetworkAccessManager qnam;
    enum DownloadType m_type;
private slots:
    void ondownloadFinished(QNetworkReply *reply);
    void onNewFileInfo();
    void onAppListDownloaded();
    void onProgress(qint64 rec,qint64 tot);
    void onAllDownloaded();
};

#endif // UPDATERDIALOG_H
