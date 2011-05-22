#ifndef APPINFO_T_H
#define APPINFO_T_H

#include <QObject>
#include <QList>
#include <QIcon>
#include <QNetworkAccessManager>

class appinfo_t : public QObject
{
    Q_OBJECT


public:
    explicit appinfo_t(QString app_name);
    ~appinfo_t();

    enum appinfo_flags{
        INVALID             = 0x0000,
        DOWNLOADED          = 0x0001,
        INSTALLED           = 0x0002,
        UPDATE_AVAIL        = 0x0004,
        SELECTED_REM        = 0x0008,
        SELECTED_INST_DL    = 0x0010,
        NO_REGISTRY         = 0x0020,
        SILENT              = 0x0040,
        ONLY_INFO           = 0x0080,
        NEW                 = 0x0100,
        IGNORE_LATEST       = 0x0200,
        ONLY_SILENT         = 0x0400  //Ex: Chrome
    };

    struct url_find_t{QString start;QString end;};
    struct inet_file_t{
        int id;
        QString url;
        QString description;
        QList<url_find_t> follow_urls;
    };

    QList<QString> categories;
    QString Name;
    QString LatestVersion,
            DlVersion,
            LastVersionCheck,
            Path,
            Description,
            Info,
            WebPage,
            fileName;
    int downloaded_id;
    QList<inet_file_t> inet_files;

    void updateVersion(QNetworkAccessManager *qnam = 0);
    bool loadFileInfo();
    QString getDownloadURL(inet_file_t &inet_file);
    QIcon getIcon();
    inet_file_t getFileInfoByDlId();
    bool forceRegistryToLatestVersion();

    bool isFlagSet(enum appinfo_flags flag);
    void setFlag(enum appinfo_flags flag);
    void unSetFlag(appinfo_t::appinfo_flags flag);
    bool ParseRegistryInfo();
    void saveApplicationInfo();
    void updateAppInfo();

    bool contains(QStringList &strings);

    bool newerVersionAvailable();

    struct registry_info_t{
        QString path;
        QString search_term;
        QString version_path;
        QString version;
        QString icon_path;
        QString icon;
        QString silent_uninstall;
        QString silent_uninstall_path;
        QString uninstall_path;
        QString uninstall;
    };
    struct registry_info_t registry_info;

    QString install_param;
    QString uninstall_param;
    QString uninstall_follow;
    QString install_follow;

private:

    struct inet_version_t{
        QString url,
                layout;
        url_find_t url_find;
    };

    enum appinfo_flags state;
    inet_version_t version_info;
    QIcon app_icon;


signals:
    void infoUpdated(appinfo_t *info,bool ok);

private slots:
    void onHtmlVersionDownloaded();
    void onHtmlAppInfoDownloaded();
};

#endif // APPINFO_T_H
