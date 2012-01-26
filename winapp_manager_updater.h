#ifndef WINAPP_MANAGER_UPDATER_H
#define WINAPP_MANAGER_UPDATER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QProgressDialog>
#include <QElapsedTimer>
#include <QFile>
#include <QUrl>
#include <QTimer>
#include <QDialog>
#include "ui_updaterdialog.h"

class winapp_manager_updater : public QDialog
{
        Q_OBJECT
        QString latest_version;
        QNetworkAccessManager qnam;

        QNetworkReply *reply;
        QElapsedTimer timer;
        QFile *WinApp_Manager_File;
        QTimer update_timer;
        Ui_UpdaterDialog *ui;
    public:
        winapp_manager_updater();
        ~winapp_manager_updater();

        void activate();

    private slots:
        void downloaded();
        void check_version();
        void setProgress(qint64 recieved,qint64 total);
        void cancel();
        void getRedirect();
        void showProgress();


    public slots:
        void check();
        void download();

    signals:
        void newer_version_available();
        void no_new_version(QString message);
        
};

#endif // WINAPP_MANAGER_UPDATER_H
