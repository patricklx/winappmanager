#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QDate>

namespace Ui {
    class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    enum settings_values{
        ASK,
        ATTENDED,
        SILENT,
        CLOSE,
        MINIMIZE
    };

    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

    bool setStartWithSystem(bool start_with_system);
    bool startsWithSystem();

    static QDate lastVersionUpdate();
    static void setLastVersionUpdate(QDate date);
    static QDate lastInfoUpdate();
    static void setLastInfoUpdate(QDate date);

    static int getInstallMode();
    static int getUpgradeMode();
    static int getCloseMode();
    static int getInstallTaskMode();

    static bool shouldCheckVersions();
    static bool shouldCheckAppInfo();

    static int simulDownloadCount();

    static void saveSettings();
    static void loadSettings();
private slots:
    void on_btApply_clicked();

    void on_btCancel_clicked();

private:
    Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
