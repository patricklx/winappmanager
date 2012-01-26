#include <QSettings>
#include <QDate>
#include <QMessageBox>
#include <QDir>
#include <QNetworkProxy>
#include <QDesktopWidget>
#include "settingsdialog.h"
#include "ui_settingsdialog.h"

QSettings *settings;


SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    move(SettingsDialog::screenCenter()-rect().bottomRight()/2);

    ui->checklatestversion->setChecked(shouldCheckVersions());
    ui->checkupdates->setChecked(shouldCheckAppInfo());
    ui->checkstartwithsystem->setChecked(startsWithSystem());
    ui->maxtaskCount->setValue(simulDownloadCount());
    ui->ckShowAllApps->setChecked(showAllApps());
    ui->ckdlPackage->setChecked(isSetDownloadPackages());

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

    switch(getUninstallMode())
    {
    case ASK:
	ui->checkUninstallAsk->setChecked(true);
	break;
    case ATTENDED:
	ui->checkAttendedUninstall->setChecked(true);
	break;
    case SILENT:
	ui->checkSilentUninstall->setChecked(true);
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

    if(proxyEnabled())
        ui->proxy_settings->setChecked(true);

    switch(settings->value("PROXY_TYPE").toInt())
    {
    case QNetworkProxy::Socks5Proxy:
        ui->proxy_type->setCurrentIndex(0);
        break;
    case QNetworkProxy::HttpProxy:
        ui->proxy_type->setCurrentIndex(1);
        break;
    case QNetworkProxy::HttpCachingProxy:
        ui->proxy_type->setCurrentIndex(2);
        break;
    case QNetworkProxy::FtpCachingProxy:
        ui->proxy_type->setCurrentIndex(3);
        break;
    }

    ui->proxy_hostname->setText( settings->value("PROXY_HOSTNAME").toString() );
    ui->proxy_username->setText( settings->value("PROXY_USERNAME").toString() );
    ui->proxy_port->setText( settings->value("PROXY_PORT").toString() );
    ui->proxy_password->setText( settings->value("PROXY_PASSWORD").toString());

}

SettingsDialog::~SettingsDialog()
{
    delete ui;

}

void SettingsDialog::on_btApply_clicked()
{
    settings->setValue("CHECK_VERSIONS",ui->checklatestversion->isChecked());
    settings->setValue("CHECK_INFO",ui->checkupdates->isChecked());
    settings->setValue("DOWNLOAD_COUNT",ui->maxtaskCount->value());
    settings->setValue("SHOW_ALL_APPS",ui->ckShowAllApps->isChecked());
    settings->setValue("DL_PACKAGES",ui->ckdlPackage->isChecked());

    if( !setStartWithSystem(ui->checkstartwithsystem->isChecked()) )
    {
        QMessageBox::information(this,"Setting Start With System","Failed to add/remove Winapp_manager to/from the startup!\n retry as admin");
    }

    if( ui->checkAskInstall->isChecked() )
        settings->setValue("INSTALL_MODE",ASK);
    if( ui->checkAttendeInstall->isChecked() )
        settings->setValue("INSTALL_MODE",ATTENDED);
    if( ui->checkSilentInstall->isChecked() )
        settings->setValue("INSTALL_MODE",SILENT);

    if( ui->checkUninstallAsk->isChecked() )
	settings->setValue("UNINSTALL_MODE",ASK);
    if( ui->checkAttendedUninstall->isChecked() )
	settings->setValue("UNINSTALL_MODE",ATTENDED);
    if( ui->checkSilentUninstall->isChecked() )
	settings->setValue("UNINSTALL_MODE",SILENT);

    if( ui->checkUpgradeAsk->isChecked() )
        settings->setValue("UPGRADE_MODE",ASK);
    if( ui->checkAttendedUpgrade->isChecked() )
        settings->setValue("UPGRADE_MODE",ATTENDED);
    if( ui->checkSilentUpgrade->isChecked() )
        settings->setValue("UPGRADE_MODE",SILENT);

    if( ui->checkAskClose->isChecked() )
        settings->setValue("CLOSE_MODE",ASK);
    if( ui->checkCloseClsoe->isChecked() )
        settings->setValue("CLOSE_MODE",CLOSE);
    if( ui->checkCloseMinimize->isChecked() )
        settings->setValue("CLOSE_MODE",MINIMIZE);

    if( ui->installmodeAttended->isChecked() )
        settings->setValue("INSTALL_TASK_MODE",ATTENDED);
    if( ui->installModeSilent->isChecked() )
        settings->setValue("INSTALL_TASK_MODE",SILENT);

    if( ui->proxy_settings->isChecked() )
    {
        settings->setValue("PROXY_ENABLED",true);
        QString value;
        value = ui->proxy_hostname->text();
        settings->setValue("PROXY_HOSTNAME",value);

        value = ui->proxy_password->text();
        settings->setValue("PROXY_PASSWORD",value);

        value = ui->proxy_port->text();
        settings->setValue("PROXY_PORT",value);

        value = ui->proxy_type->currentText();
        if(value == "Socks5Proxy")
            settings->setValue("PROXY_TYPE",QNetworkProxy::Socks5Proxy);
        if( value == "HttpProxy")
            settings->setValue("PROXY_TYPE",QNetworkProxy::HttpProxy);
        if( value == "HttpCachingProxy" )
            settings->setValue("PROXY_TYPE",QNetworkProxy::HttpCachingProxy);
        if( value == "FtpCachingProxy" )
            settings->setValue("PROXY_TYPE",QNetworkProxy::FtpCachingProxy);

        value = ui->proxy_username->text();
        settings->setValue("PROXY_USERNAME",value);
        setNetworkProxy();
    }else
        settings->setValue("PROXY_ENABLED",false);

    saveSettings();
    close();
}


bool SettingsDialog::showAllApps()
{
    return settings->value("SHOW_ALL_APPS","true").toBool();
}

bool SettingsDialog::proxyEnabled()
{
    return settings->value("PROXY_ENABLED",false).toBool();
}

bool SettingsDialog::isSetDownloadPackages()
{
    return settings->value("DL_PACKAGES",true).toBool();
}

QNetworkProxy SettingsDialog::getProxySettings()
{
    QNetworkProxy proxy;
    proxy.setType((QNetworkProxy::ProxyType)settings->value("PROXY_TYPE").toInt());
    proxy.setHostName(settings->value("PROXY_HOSTNAME").toString());
    proxy.setUser(settings->value("PROXY_USERNAME").toString());
    proxy.setPassword(settings->value("PROXY_PASSWORD").toString());
    proxy.setPort(settings->value("PROXY_PORT").toInt());
    return proxy;
}

bool SettingsDialog::updatedToLatestVersion()
{
    return (settings->value("latestVersion","1.0").toString() == "2.0");
}


void SettingsDialog::setUpdatedToLatestVersion()
{
    settings->setValue("latestVersion","2.0");
}

void SettingsDialog::loadSettings()
{
    settings = new QSettings("WinAppM_settings.ini",QSettings::IniFormat);
    setNetworkProxy();
}

void SettingsDialog::unLoadSettings()
{
    saveSettings();
    delete settings;
}

void SettingsDialog::setNetworkProxy()
{
    if(proxyEnabled())
    {
        QNetworkProxy proxy = getProxySettings();
        QNetworkProxy::setApplicationProxy(proxy);
    }
}

void SettingsDialog::on_btCancel_clicked()
{
    close();
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
        QString path = QDir::currentPath()+"/WinApp_Manager.exe";
        path = "\""+path.replace('/','\\')+"\" hidden";
        registry.setValue("WinApp_Manager",path);
    }
    else
        registry.remove("WinApp_Manager");

    if(registry.status() != QSettings::NoError)
        return false;
    return true;
}

bool SettingsDialog::startsWithSystem()
{
    QSettings registry("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run\\", QSettings::NativeFormat);
    if( registry.contains("WinApp_Manager") )
    {
        return true;
    }else
        return false;
}
int SettingsDialog::simulDownloadCount()
{
    return settings->value("DOWNLOAD_COUNT",3).toInt();
}

QDate SettingsDialog::lastInfoUpdate()
{
    return settings->value("LAST_INFO_CHECK",0).toDate();
}

void SettingsDialog::setLastInfoUpdate(QDate date)
{
    settings->setValue("LAST_INFO_CHECK",date);
}

QDate SettingsDialog::lastVersionUpdate()
{
    return settings->value("LAST_VERSION_CHECK",0).toDate();
}

void SettingsDialog::setLastVersionUpdate(QDate date)
{
    settings->setValue("LAST_VERSION_CHECK",date);
}

int SettingsDialog::getUninstallMode()
{
    return settings->value("UNINSTALL_MODE").toInt();
}

int SettingsDialog::getInstallMode()
{
    return settings->value("INSTALL_MODE").toInt();
}

int SettingsDialog::getUpgradeMode()
{
    return settings->value("UPGRADE_MODE").toInt();
}

int SettingsDialog::getCloseMode()
{
    return settings->value("CLOSE_MODE").toInt();
}

int SettingsDialog::getInstallTaskMode()
{
    return settings->value("INSTALL_TASK_MODE",SILENT).toInt();
}

bool SettingsDialog::shouldCheckVersions()
{
    return settings->value("CHECK_VERSIONS",true).toBool();
}

bool SettingsDialog::shouldCheckAppInfo()
{
    return settings->value("CHECK_INFO",true).toBool();
}

QPoint SettingsDialog::screenCenter()
{
    QPoint pos = QCursor::pos();
    int desktop = QApplication::desktop()->screenNumber(pos);
    QDesktopWidget *monitor = QApplication::desktop();
    pos = monitor->availableGeometry(desktop).center();
    return pos;
}

QString SettingsDialog::currentVersion()
{
    QDate buildDate = QLocale(QLocale::C).toDate(QString(__DATE__).simplified(), QLatin1String("MMM d yyyy"));
    return buildDate.toString("yy.MM.d");
}



