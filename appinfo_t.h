#ifndef APPINFO_T_H
#define APPINFO_T_H

#include <QObject>
#include <QList>
#include <QIcon>
#include <QNetworkAccessManager>

#include "appinfo_registry_t.h"

class appinfo_t : public QObject
{
        Q_OBJECT


    public:
        explicit appinfo_t(QString app_name);
        ~appinfo_t();

        enum appinfo_flags{
            INVALID                 = 0x0000,
            DOWNLOADED              = 0x0001,
            INSTALLED               = 0x0002,
            UPDATE_AVAIL            = 0x0004,
            SELECTED_REM            = 0x0008,
            SELECTED_INST_DL        = 0x0010,
            NO_REGISTRY             = 0x0020,
            ONLY_SILENT             = 0x0040, //Ex: Chrome,uTorrent
            ONLY_INFO               = 0x0080,
            NEW                     = 0x0100,
            IGNORE_LATEST           = 0x0200,
            ADMIN                   = 0x0400,  //Ex: uTorrent
            NEEDS_UNINSTALL         = 0x0800,  //Ex: Firefox,CDburnerXP sometimes don't remove old registry info
            NO_INFO                 = 0x1000,
            FILEHIPPO_BETA          = 0x2000,
            ONLY_SILENT_UNINSTALL   = 0x4000,
            WIN_VERSION_OK          = 0x8000
        };

        struct registry_info_t{
                QString regex;
                QString regexReplace;
                QString path;
                QString seachValue;
                QString version_path;
                QString version;
                QString icon;
                QString silent_uninstall;
                QString uninstall;
                QString displayName;
        };

        struct inet_version_t{
                QString url,
                urlRegEx;

                QString versionRegEx,
                regExReplace;
        };

        struct file_info_t{
                int id;
                QString url;
                QString description;
                QStringList follow_urls_regex;
                file_info_t():id(-1){}
        };

        QList<QString> categories;
        QString Name;
        QString LatestVersion,
        DlVersion,
        LastVersionCheck,
        InstalledVersion,
        Path,
        Description,
        Info,
        WebPage,
        fileName,
        zipExeRegex;
        QString install_param;
        QString uninstall_param;
        QString uninstall_lastProcessName;
        QString install_lastProcessName;
        int downloaded_id;
        QList<file_info_t> inet_files;
        registry_info_t registry_info;
        inet_version_t version_info;

        QNetworkReply* updateVersion(QNetworkAccessManager *qnam = 0);
        void updateRegistryVersion();
        bool loadFileInfo();
        QString getDownloadURL(file_info_t &inet_file);
        QIcon getIcon();
        file_info_t getFileInfoByDlId();
        bool forceRegistryToLatestVersion();

        bool isFlagSet(enum appinfo_flags flag);
        void setFlag(enum appinfo_flags flag);
        void unSetFlag(appinfo_t::appinfo_flags flag);
        bool ParseRegistryInfo();
        bool checkInstall();
        void saveApplicationInfo(bool filehippo_beta_change=false);
        void updateAppInfo();

        bool contains(QStringList &strings);

        bool newerVersionAvailable();

        void setRegistryInfo(registry_group_t *reg_group);

        void setFilehippoBeta(bool enable);

        void checkMinWinVersion(QString min_version);
        void checkMaxWinVersion(QString max_version);

    private:

        enum appinfo_flags state;
        QIcon app_icon;


    signals:
        void infoUpdated(appinfo_t *info,bool ok);

    private slots:
        void onHtmlVersionDownloaded();
        void onHtmlAppInfoDownloaded();

        void delayedSetIcon();
};

#endif // APPINFO_T_H
