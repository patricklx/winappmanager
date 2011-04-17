#include <QSettings>
#include <QDate>
#include <QMessageBox>
#include <QDir>
#include "settingsdialog.h"
#include "ui_settingsdialog.h"

QSettings *settings;


SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    ui->checklatestversion->setChecked(shouldCheckVersions());
    ui->checkupdates->setChecked(shouldCheckAppInfo());
    ui->checkstartwithsystem->setChecked(startsWithSystem());
    ui->maxtaskCount->setValue(simulDownloadCount());

    switch(getInstallMode())
    {
    case ASK:
        ui->checkAskInstall->setChecked(true);
        break;
    case ATTENDED:
        ui->checkAttendeInstall->setChecked(true);
        break;
    case SILENT:
        ui->checkSilentInstall->setChecked(true);
        break;
    }
    switch(getUpgradeMode())
    {
    case ASK:
        ui->checkUpgradeAsk->setChecked(true);
        break;
    case ATTENDED:
        ui->checkAttendedUpgrade->setChecked(true);
        break;
    case SILENT:
        ui->checkSilentUpgrade->setChecked(true);
        break;
    }

    switch(getCloseMode())
    {
    case ASK:
        ui->checkAskClose->setChecked(true);
        break;
    case CLOSE:
        ui->checkCloseClsoe->setChecked(true);
        break;
    case MINIMIZE:
        ui->checkCloseMinimize->setChecked(true);
        break;
    }

    switch(getInstallTaskMode())
    {
    case ATTENDED:
        ui->installmodeAttended->setChecked(true);
        break;
    case SILENT:
        ui->installModeSilent->setChecked(true);
        break;
    }
}

SettingsDialog::~SettingsDialog()
{
    delete ui;

}

void SettingsDialog::on_btApply_clicked()
{
    ui->maxtaskCount->setValue(simulDownloadCount());

    settings->setValue(tr("CHECK_VERSIONS"),ui->checklatestversion->isChecked());
    settings->setValue(tr("CHECK_INFO"),ui->checkupdates->isChecked());
    settings->setValue(tr("DOWNLOAD_COUNT"),ui->maxtaskCount->value());

    if( !setStartWithSystem(ui->checkstartwithsystem->isChecked()) )
    {
        QMessageBox::information(this,tr("Setting Start With System"),tr("Failed to add/remove Winapp_manager to/from the startup!\n retry as admin"));
    }


    if( ui->checkAskInstall->isChecked() )
        settings->setValue(tr("INSTALL_MODE"),ASK);
    if( ui->checkAttendeInstall->isChecked() )
        settings->setValue(tr("INSTALL_MODE"),ATTENDED);
    if( ui->checkSilentInstall->isChecked() )
        settings->setValue(tr("INSTALL_MODE"),SILENT);

    if( ui->checkUpgradeAsk->isChecked() )
        settings->setValue(tr("UPGRADE_MODE"),ASK);
    if( ui->checkAttendedUpgrade->isChecked() )
        settings->setValue(tr("UPGRADE_MODE"),ATTENDED);
    if( ui->checkSilentUpgrade->isChecked() )
        settings->setValue(tr("UPGRADE_MODE"),SILENT);

    if( ui->checkAskClose->isChecked() )
        settings->setValue(tr("CLOSE_MODE"),ASK);
    if( ui->checkCloseClsoe->isChecked() )
        settings->setValue(tr("CLOSE_MODE"),CLOSE);
    if( ui->checkCloseMinimize->isChecked() )
        settings->setValue(tr("CLOSE_MODE"),MINIMIZE);

    if( ui->installmodeAttended->isChecked() )
        settings->setValue(tr("INSTALL_TASK_MODE"),ATTENDED);
    if( ui->installModeSilent->isChecked() )
        settings->setValue(tr("INSTALL_TASK_MODE"),SILENT);
    close();
}

void SettingsDialog::on_btCancel_clicked()
{
    close();
}


void SettingsDialog::loadSettings()
{
    settings = new QSettings(QString("WinAppM_settings.ini"),QSettings::IniFormat);

}

void SettingsDialog::saveSettings()
{
    settings->sync();
}


bool SettingsDialog::setStartWithSystem(bool start_with_system)
{
    QSettings registry("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run\\", QSettings::NativeFormat);
    if(start_with_system==true)
    {
        QString path = QDir::currentPath()+tr("/WinApp_Manager.exe");
        path = tr("\"")+path.replace('/','\\')+tr("\" hidden");
        registry.setValue(tr("WinApp_Manager"),path);
    }
    else
        registry.remove(tr("WinApp_Manager"));

    if(registry.status() != QSettings::NoError)
        return false;
    return true;
}

bool SettingsDialog::startsWithSystem()
{
    QSettings registry("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run\\", QSettings::NativeFormat);
    if( registry.contains(tr("WinApp_Manager")) )
    {
        return true;
    }else
        return false;
}
int SettingsDialog::simulDownloadCount()
{
    return settings->value(tr("DOWNLOAD_COUNT"),3).toInt();
}

QDate SettingsDialog::lastInfoUpdate()
{
    return settings->value(tr("LAST_INFO_CHECK"),0).toDate();
}

void SettingsDialog::setLastInfoUpdate(QDate date)
{
    settings->setValue(tr("LAST_INFO_CHECK"),date);
}

QDate SettingsDialog::lastVersionUpdate()
{
    return settings->value(tr("LAST_VERSION_CHECK"),0).toDate();
}

void SettingsDialog::setLastVersionUpdate(QDate date)
{
    settings->setValue(tr("LAST_VERSION_CHECK"),date);
}

int SettingsDialog::getInstallMode()
{
    return settings->value(tr("INSTALL_MODE")).toInt();
}

int SettingsDialog::getUpgradeMode()
{
    return settings->value(tr("UPGRADE_MODE")).toInt();
}

int SettingsDialog::getCloseMode()
{
    return settings->value(tr("CLOSE_MODE")).toInt();
}

int SettingsDialog::getInstallTaskMode()
{
    return settings->value(tr("INSTALL_TASK_MODE"),SILENT).toInt();
}

bool SettingsDialog::shouldCheckVersions()
{
    return settings->value(tr("CHECK_VERSIONS"),true).toBool();
}

bool SettingsDialog::shouldCheckAppInfo()
{
    return settings->value(tr("CHECK_INFO"),true).toBool();
}
