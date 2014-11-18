#include <QSettings>
#include <QDate>
#include <QMessageBox>
#include <QDir>
#include <QNetworkProxy>
#include <QDesktopWidget>
#include "settingsdialog.h"
#include "ui_settingsdialog.h"

QSettings *settings;

/*All
 *
 *CHECK_VERSIONS
 *CHECK_INFO
 *DOWNLOAD_COUNT
 *SHOW_ALL_APPS
 *DL_PACKAGES
 *INSTALL_MODE
 *UNINSTALL_MODE
 *UPGRADE_MODE
 *CLOSE_MODE
 *SAVE_DOWNLOADED
 *TASK_INSTALL_MODE
 *
 *PROXY_ENABLED
 *PROXY_TYPE
 *PROXY_HOSTNAME
 *PROXY_USERNAME
 *PROXY_PORT
 *PROXY_PASSWORD
*/


QMap<QString,QVariant> SettingsDialog::defaults = QMap<QString,QVariant>();

SettingsDialog::SettingsDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::SettingsDialog)
{
	ui->setupUi(this);

	move(SettingsDialog::screenCenter()-rect().bottomRight()/2);

	ui->checklatestversion->setChecked(this->value("CHECK_VERSIONS").toBool());
	ui->checkupdates->setChecked(this->value("CHECK_UPDATES").toBool());
	ui->checkstartwithsystem->setChecked(startsWithSystem());
	ui->maxtaskCount->setValue(this->value("DOWNLOAD_COUNT").toInt());
	ui->ckShowAllApps->setChecked(this->value("SHOW_ALL_APPS").toBool());
	ui->ckdlPackage->setChecked(this->value("DL_PACKAGES").toBool());
	ui->saveDownloadedCheck->setChecked(this->value("SAVE_DOWNLOADED").toBool());

	switch(this->value("INSTALL_MODE").toInt())
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

	switch(this->value("UNINSTALL_MODE").toInt())
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

	switch(this->value("UPGRADE_MODE").toInt())
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

	switch(this->value("CLOSE_MODE").toInt())
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

	switch(this->value("TASK_INSTALL_MODE").toInt())
	{
	case Attended:
		ui->installmodeAttended->setChecked(true);
		break;
	case Silent:
		ui->installModeSilent->setChecked(true);
		break;
	}

	if(this->value("PROXY_ENABLED").toBool())
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
	settings->setValue("DOWNLOAD_COUNT",ui->maxtaskCount->value());

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
	return (settings->value("latestVersion","1.0").toString() == "4.0");
}


void SettingsDialog::setUpdatedToLatestVersion()
{
	settings->setValue("latestVersion","4.0");
}

void SettingsDialog::loadSettings()
{
	settings = new QSettings("WinAppM_settings.ini",QSettings::IniFormat);
	setNetworkProxy();


	SettingsDialog::defaults["CHECK_VERSIONS"] = true;
	SettingsDialog::defaults["CHECK_INFO"] = true;
	SettingsDialog::defaults["SHOW_ALL_APPS"] = true;
	SettingsDialog::defaults["DL_PACKAGES"] = true;
	SettingsDialog::defaults["INSTALL_MODE"] = SettingsDialog::Silent;
	SettingsDialog::defaults["UNINSTALL_MODE"] = SettingsDialog::Silent;
	SettingsDialog::defaults["UPGRADE_MODE"] = SettingsDialog::Silent;
	SettingsDialog::defaults["CLOSE_MODE"] = SettingsDialog::Ask;
	SettingsDialog::defaults["CHECK_VERSIONS"] = true;
	SettingsDialog::defaults["PROXY_ENABLED"] = false;
	SettingsDialog::defaults["DOWNLOAD_COUNT"] = 3;
	SettingsDialog::defaults["TASK_INSTALL_MODE"] = SettingsDialog::Silent;

}

void SettingsDialog::unLoadSettings()
{
	saveSettings();
	delete settings;
}

void SettingsDialog::setNetworkProxy()
{
	if(value("PROXY_ENABLED").toBool())
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

QVariant SettingsDialog::value(QString key)
{
	return settings->value(key,defaults[key]);
}

void SettingsDialog::setValue(QString key, QVariant value)
{
	settings->setValue(key,value);
}



