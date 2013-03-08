#ifndef UPDATERDIALOG_H
#define UPDATERDIALOG_H


#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QTimer>
#include <QFile>
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

    explicit UpdaterDialog(QWidget *parent,QList<AppList::Fileinfo> &list,enum DownloadType type);
    QStringList getNewAppsList();
    ~UpdaterDialog();



signals:
    void allFilesDownloaded();

private slots:
    void on_btCancel_clicked();

private:
    Ui::UpdaterDialog *ui;
    QList<AppList::Fileinfo> &m_list;
    QList<AppList::Fileinfo> dlAppinfo;
    QList<QNetworkReply*> connections;
    QTimer timeout;
    QFile downloadFile;
    int m_count;
    int m_max;
    QStringList newList;

    QNetworkAccessManager qnam;
    enum DownloadType m_type;

    void downloadAll();
    void downloadAppInfo(QString name);

private slots:
    void ondownloadFinished(QNetworkReply *reply);
    void onNewFileInfo();
    void onAppListDownloaded();
    void onProgress(qint64 rec,qint64 tot);
    void onZipDownloaded();
    void fileDownloadProgress(qint64 recv, qint64 tot);
};

#endif // UPDATERDIALOG_H
