#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QDate>
#include <QNetworkProxy>
#include <QSettings>
#include <QMap>

namespace Ui {
    class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    enum SettingVals{
        Ask,
        Attended,
        Silent,
        Close,
        Minimize
    };

    enum Keys{
        CheckWinappManagerVersion,
        CheckVersions,
        CheckInfo,
        DlPackages,
        MaxDownloads,
        ShowAllApps,
        InstallMode,
        UninstallMode,
        UpgradeMode,
        CloseMode,
        InstallTaskMode,
        ProxyEnabled,
        ProxyHostname,
        ProxyPort,
        ProxyType,
        ProxyUsername,
        LastVersionCheck,
        LastInfoCheck,
        InfoDate,
        SaveDownloaded
    };


    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

    bool setStartWithSystem(bool start_with_system);
    bool startsWithSystem();


    static QNetworkProxy getProxySettings();


    static void saveSettings();

    static void loadSettings();
    static void unLoadSettings();

    static QPoint screenCenter();

    static bool isSetDownloadPackages();

    static bool updatedToLatestVersion();
    static void setUpdatedToLatestVersion();

    static QString currentVersion();

    struct SettingsVals{
            QString name;
            QVariant value;
    };
    static QMap<int,QVariant> settingsDefaults;
    static QMap<int,QString> settingNames;
    template <class T>
    static T value(Keys key)
    {
        QString name = settingNames[key];
        QVariant defaultVal = settingsDefaults[key];
        return SettingsDialog::settings->value(name,defaultVal).value<T>();
    }

    static void setNetworkProxy();
    static void setValue(Keys key, QVariant value);

private slots:
    void on_btApply_clicked();

    void on_btCancel_clicked();

private:
    Ui::SettingsDialog *ui;
    static QSettings *settings;
};

#endif // SETTINGSDIALOG_H
