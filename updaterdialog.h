#ifndef UPDATERDIALOG_H
#define UPDATERDIALOG_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QTimer>
#include "applist_t.h"

namespace Ui {
    class UpdaterDialog;
}

class UpdaterDialog : public QDialog
{
    Q_OBJECT

public:
    enum download_type{
        update_version,
        update_appinfo,
        update_check_appinfo
    };

    explicit UpdaterDialog(QList<applist_t::fileinfo_t> &list,enum download_type type);
    ~UpdaterDialog();


signals:
    void newFileInfoUpdated(QString name,QString lastUpdate);

private slots:
    void on_btCancel_clicked();

private:
    Ui::UpdaterDialog *ui;
    QList<applist_t::fileinfo_t> &m_list;
    QList<applist_t::fileinfo_t> dlAppinfo;
    QList<QNetworkReply*> connections;
    QTimer timeout;
    int m_count;
    int m_max;

    QNetworkAccessManager qnam;
    enum download_type m_type;
private slots:
    void ondownloadFinished(QNetworkReply *reply);
    void onNewFileInfo();
    void onAppListDownloaded();
    void onProgress(qint64 rec,qint64 tot);
};

#endif // UPDATERDIALOG_H
