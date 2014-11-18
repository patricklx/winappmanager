#ifndef APPINFO_T_H
#define APPINFO_T_H

#include <QObject>
#include <QList>
#include <QIcon>
#include <QNetworkAccessManager>
#include <QMap>
#include <yaml-cpp/yaml.h>

#include "appinfo_registry.h"

class AppInfo : public QObject
{
    Q_OBJECT


public:
    explicit AppInfo(QString app_name);
    ~AppInfo();

        enum Flags{
            Invalid                 = 0x0000,
            Downloaded              = 0x0001,
            Installed               = 0x0002,
            UpdateAvailable         = 0x0004,
            SelectedRemove          = 0x0008,
            SelectedInstallDownload = 0x0010,
            NoRegistry              = 0x0020,
            OnlyInfo                = 0x0040,
            New                     = 0x0080,
            IgnoreLatest            = 0x0100,
            NoInfo                  = 0x0200,
            FilehippoBeta           = 0x0400,
            WinVersionOk            = 0x0800,
            OnlySilent              = 0x1000
        };

        struct Version{

            QString regex;
            QString url;
            QString versionRegex;
            QString versionRegexReplace;

            void load(const YAML::Node &node);
        };


        struct UrlContainer{

        private:
            friend class AppInfo;
            struct FollowLink{
                QString regex;
                FollowLink *followLink;

                void load(const YAML::Node &node);
            };
        public:
            QString description;
            QString url;
            FollowLink *followLink;
            QMap<QString,QString> parameter;
            int id;

            void load(const YAML::Node &node);
            UrlContainer():followLink(NULL),id(-1){}
        };

        struct Install{
            QString silentParameter;
            bool onlySilent;
            bool needsAdmin;
            bool needsUninstall;
            QString lastProcess;
            QString zipExeRegex;
            Install():onlySilent(false),needsAdmin(false),needsUninstall(false){}
        };

        struct Remove{
            friend class AppInfo;
            QString silentParameter;
            QString lastProcess;
            bool onlySilent;
            bool needsAdmin;
            Remove():onlySilent(false),needsAdmin(true){}
        };


        struct Registry{

            QString uninstallInfoPath;
            QString versionRegex;
            QString regexReplace;
            QString versionPath;
            QString searchKey;
        };

        struct UserData{

            QString downloadedVersion;
            QString latestVersion;
            QString lastCheck;
            QString filename;
            QMap<QString,QString> choice;
            int downloadedId;
            UserData():downloadedId(-1){}
        };

    struct RegistryInfo{
        QString path;
        QString versionPath;
        QString version;
        QString icon;
        QString silentUninstall;
        QString uninstall;
        QString displayName;
    };


    QNetworkReply* updateVersion(QNetworkAccessManager *qnam = 0);
    void updateRegistryVersion();
    bool load();
    QString getDownloadURL(UrlContainer &fileLocation);
    QIcon getIcon();
    bool forceRegistryToLatestVersion();

    bool isFlagSet(Flags flag);
    void setFlag(Flags flag);
    void unSetFlag(Flags flag);
    bool parseRegistryInfo();
    void saveApplicationInfo();
    void updateAppInfo();

    bool contains(QStringList &strings);

    bool newerVersionAvailable();

    void removeDownloadedFile();
    void ignoreNewerVersions(bool ignore = true);

    void setRegistryInfo(RegistryGroup *reg_group);

    void setFilehippoBeta(bool enable);

    void checkMinWinVersion(QString min_version);
    void checkMaxWinVersion(QString max_version);

    bool loadYaml();
    void load(const YAML::Node &node);

    void updateDownloadVersion();
    QString path();
    QString fileName();
    void setDownloaded(int id, QString name);
    void setChoice(QString key, QString value);

    void setExtractedFilename(QString name);


    QString lastError();
    QString latestVersion();
    QString name();
    QString description();
    QString webpage();
    QString info();
    AppInfo::Version version();
    QList<UrlContainer> urls();
    QList<QString> categories();
    AppInfo::Install getInstallInfo();
    AppInfo::Remove getRemoveInfo();
    AppInfo::UserData getUserData();
    AppInfo::Registry registry();
    AppInfo::RegistryInfo registryInfo();
    AppInfo::UrlContainer getDownloadedFileUrl();
private:

    QString _error;
    QString _path;

    Flags _state;
    QIcon _appIcon;
    QString _name;
    QString _description;
    QString _webpage;
    QString _info;
    Version _version;
    QList<UrlContainer> _urlContainers;
    QList<QString> _categories;
    Install _install;
    Remove _remove;
    Registry _registry;
    UserData _userData;

    RegistryInfo _registryInfo;

    void loadUserData();
    QList<QNetworkCookie> getFileHippoCookies();

signals:
    void infoUpdated(AppInfo *info);
    void versionUpdated(AppInfo *info,bool ok);
    void appInfoUpdated(AppInfo *info,bool ok);

private slots:
    void onHtmlVersionDownloaded();
    void onHtmlAppInfoDownloaded();

    void delayedSetIcon();
};

#endif // APPINFO_T_H
