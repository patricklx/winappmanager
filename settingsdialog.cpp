#include <QSettings>
#include <QDate>
#include <QMessageBox>
#include <QDir>
#include <QNetworkProxy>
#include <QDesktopWidget>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QCoreApplication>
#include <QFileDialog>
#include "settingsdialog.h"
#include "ui_settingsdialog.h"

QSettings *SettingsDialog::settings=NULL;
QMap<int,QString> SettingsDialog::settingNames = QMap<int,QString>();
QMap<int,QVariant> SettingsDialog::settingsDefaults = QMap<int,QVariant>();

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    qDebug()<<"application name: "<<QCoreApplication::applicationName();

    move(SettingsDialog::screenCenter()-rect().bottomRight()/2);

    ui->checklatestversion->setChecked(value<bool>(CheckWinappManagerVersion));
    ui->checkupdates->setChecked(value<bool>(CheckInfo));
    ui->checkstartwithsystem->setChecked(startsWithSystem());
    ui->maxtaskCount->setValue(value<int>(MaxDownloads));
    ui->ckShowAllApps->setChecked(value<bool>(ShowAllApps));
    ui->ckdlPackage->setChecked(value<bool>(DlPackages));
    ui->saveDownloadedCheck->setChecked(value<bool>(SaveDownloaded));


    switch(value<int>(InstallMode))
    {
    case Ask:
        ui->checkAskInstall->setChecked(true);
        break;
    case Attended:
        ui->checkAttendeInstall->setChecked(true);
        break;
    case Silent:
        ui->checkSilentInstall->setChecked(true);
        break;
    }

    switch(value<int>(UninstallMode))
    {
    case Ask:
        ui->checkUninstallAsk->setChecked(true);
        break;
    case Attended:
        ui->checkAttendedUninstall->setChecked(true);
        break;
    case Silent:
        ui->checkSilentUninstall->setChecked(true);
        break;
    }

    switch(value<int>(UpgradeMode))
    {
    case Ask:
        ui->checkUpgradeAsk->setChecked(true);
        break;
    case Attended:
        ui->checkAttendedUpgrade->setChecked(true);
        break;
    case Silent:
        ui->checkSilentUpgrade->setChecked(true);
        break;
    }

    switch(value<int>(CloseMode))
    {
    case Ask:
        ui->checkAskClose->setChecked(true);
        break;
    case Close:
        ui->checkCloseClsoe->setChecked(true);
        break;
    case Minimize:
        ui->checkCloseMinimize->setChecked(true);
        break;
    }

    switch(value<int>(InstallTaskMode))
    {
    case Attended:
        ui->installmodeAttended->setChecked(true);
        break;
    case Silent:
        ui->installModeSilent->setChecked(true);
        break;
    }

    if(value<bool>(ProxyEnabled))
        ui->proxy_settings->setChecked(true);

    switch(value<int>(ProxyType))
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

    setValue(SaveDownloaded,ui->saveDownloadedCheck->isChecked());

    if( !setStartWithSystem(ui->checkstartwithsystem->isChecked()) )
    {
        QMessageBox::information(this,"Setting Start With System","Failed to add/remove Winapp_manager to/from the startup!\n retry as admin");
    }

    if( ui->checkAskInstall->isChecked() )
        settings->setValue("INSTALL_MODE",Ask);
    if( ui->checkAttendeInstall->isChecked() )
        settings->setValue("INSTALL_MODE",Attended);
    if( ui->checkSilentInstall->isChecked() )
        settings->setValue("INSTALL_MODE",Silent);

    if( ui->checkUninstallAsk->isChecked() )
        settings->setValue("UNINSTALL_MODE",Ask);
    if( ui->checkAttendedUninstall->isChecked() )
        settings->setValue("UNINSTALL_MODE",Attended);
    if( ui->checkSilentUninstall->isChecked() )
        settings->setValue("UNINSTALL_MODE",Silent);

    if( ui->checkUpgradeAsk->isChecked() )
        settings->setValue("UPGRADE_MODE",Ask);
    if( ui->checkAttendedUpgrade->isChecked() )
        settings->setValue("UPGRADE_MODE",Attended);
    if( ui->checkSilentUpgrade->isChecked() )
        settings->setValue("UPGRADE_MODE",Silent);

    if( ui->checkAskClose->isChecked() )
        settings->setValue("CLOSE_MODE",Ask);
    if( ui->checkCloseClsoe->isChecked() )
        settings->setValue("CLOSE_MODE",Close);
    if( ui->checkCloseMinimize->isChecked() )
        settings->setValue("CLOSE_MODE",Minimize);

    if( ui->installmodeAttended->isChecked() )
        settings->setValue("INSTALL_TASK_MODE",Attended);
    if( ui->installModeSilent->isChecked() )
        settings->setValue("INSTALL_TASK_MODE",Silent);

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


void SettingsDialog::setNetworkProxy()
{
    if(value<bool>(ProxyEnabled))
    {
        QNetworkProxy proxy = getProxySettings();
        QNetworkProxy::setApplicationProxy(proxy);
    }
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
    return (settings->value("latestVersion").toString() == "4.0");
}


void SettingsDialog::setUpdatedToLatestVersion()
{
    settings->setValue("latestVersion","4.0");
}

void SettingsDialog::loadSettings()
{
    settingsDefaults[CheckWinappManagerVersion]=true;
    settingsDefaults[CheckVersions]=true;
    settingsDefaults[CheckInfo]=true;
    settingsDefaults[DlPackages]=true;
    settingsDefaults[MaxDownloads]=true;
    settingsDefaults[ShowAllApps]=true;
    settingsDefaults[InstallMode]=Silent;
    settingsDefaults[UninstallMode]=Silent;
    settingsDefaults[UpgradeMode]=Silent;
    settingsDefaults[CloseMode]=Ask;
    settingsDefaults[InstallTaskMode]=Silent;
    settingsDefaults[ProxyEnabled]=false;
    settingsDefaults[ProxyType]=QNetworkProxy::DefaultProxy;
    settingsDefaults[SaveDownloaded]=true;

    settingNames[CheckWinappManagerVersion]="CHECK_WINAPP_MANAGER_VERSION";
    settingNames[CheckVersions]="CHECK_VERSIONS";
    settingNames[CheckInfo]="CHECK_INFO";
    settingNames[DlPackages]="DL_PACKAGES";
    settingNames[MaxDownloads]="DOWNLOAD_COUNT";
    settingNames[ShowAllApps]="SHOW_ALL_APPS";
    settingNames[InstallMode]="INSTALL_MODE";
    settingNames[UninstallMode]="UNINSTALL_MODE";
    settingNames[UpgradeMode]="UPGRADE_MODE";
    settingNames[CloseMode]="CLOSE_MODE";
    settingNames[InstallTaskMode]="INSTALL_TASK_MODE";
    settingNames[ProxyEnabled]="PROXY_ENABLED";
    settingNames[ProxyHostname]="PROXY_HOSTNAME";
    settingNames[ProxyPort]="PROXY_PORT";
    settingNames[ProxyType]="PROXY_TYPE";
    settingNames[ProxyUsername]="PROXY_USERNAME";
    settingNames[LastVersionCheck]="LAST_VERSION_CHECK";
    settingNames[LastInfoCheck]="LAST_INFO_CHECK";
    settingNames[InfoDate]="INFO_DATE";
    settingNames[SaveDownloaded]="SAVE_DOWNLOADED";

    settings = new QSettings("WinAppM_settings.ini",QSettings::IniFormat);
    setNetworkProxy();
}

void SettingsDialog::unLoadSettings()
{
    saveSettings();
    delete settings;
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


void SettingsDialog::setValue(Keys key, QVariant value)
{
    QString str = settingNames[key];
    settings->setValue(str,value);
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


