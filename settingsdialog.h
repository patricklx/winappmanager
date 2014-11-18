#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QDate>
#include <QNetworkProxy>

namespace Ui {
    class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    enum settings_values{
        Ask,
        Attended,
        Silent,
        Close,
        Minimize
    };

    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

    bool setStartWithSystem(bool start_with_system);
    bool startsWithSystem();

    static void setNetworkProxy();
    static QNetworkProxy getProxySettings();

    static void saveSettings();

    static void loadSettings();
    static void unLoadSettings();

    static bool showAllApps();

    static QPoint screenCenter();

    static bool isSetDownloadPackages();

    static bool updatedToLatestVersion();
    static void setUpdatedToLatestVersion();

    static QString currentVersion();

    static QVariant value(QString key);
    static void setValue(QString key, QVariant value);

private slots:
    void on_btApply_clicked();

    void on_btCancel_clicked();

private:
    Ui::SettingsDialog *ui;
    static QMap<QString,QVariant> defaults;
};

#endif // SETTINGSDIALOG_H
