#include <QMessageBox>
#include <QTimerEvent>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMetaType>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QUrl>
#include <QString>
#include <QFile>
#include "settingsdialog.h"
#include "choosedialog.h"
#include <applist_t.h>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    move(SettingsDialog::screenCenter()-rect().bottomRight()/2);

    connect(ui->Page1Applist,SIGNAL(taskChosen(task_t*)),ui->Page2TaskList,SLOT(addTask(task_t*)));
    connect(ui->Page1Applist,SIGNAL(unSelected(appinfo_t*)),ui->Page2TaskList,SLOT(removeTask(appinfo_t*)));
    connect(ui->Page2TaskList,SIGNAL(onTaskRemoved(appinfo_t*)),ui->Page1Applist,SLOT(onRemovedFromTasks(appinfo_t*)));
    connect(ui->Page2TaskList,SIGNAL(updateAppInfo(appinfo_t*)),ui->Page1Applist,SLOT(updateItem(appinfo_t*)));
    timer_update.setInterval(1000*60*60*2);//check every 2 hour
    connect(&timer_update,SIGNAL(timeout()),SLOT(timerEvent()));

    trayicon = new QSystemTrayIcon(QIcon(":icons/WinApp_Manager.ico"),this);
    connect(trayicon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),SLOT(onTrayDoubleClicked(QSystemTrayIcon::ActivationReason)));
    connect(trayicon,SIGNAL(messageClicked()),SLOT(onTrayDoubleClicked()));

    QMenu *menu = new QMenu(this);
    QAction *act = menu->addAction("Quit");
    connect(act,SIGNAL(triggered()),SLOT(close()));

    act = menu->addAction("Show");
    connect(act,SIGNAL(triggered()),SLOT(show()));
    connect(act,SIGNAL(triggered()),SLOT(raise()));
    connect(act,SIGNAL(triggered()),&timer_update,SLOT(stop()));
    QAction *action = menu->addAction("Get latest version");
    action->setVisible(false);
    connect(action,SIGNAL(triggered()),&updater,SLOT(download()));
    connect(trayicon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),SLOT(trayContextMenuFix()));

    trayicon->show();
    trayicon->setContextMenu(menu);

    connect(ui->Page1Applist,SIGNAL(versions_available()),SLOT(newVersionMessage()));

    connect(ui->Page1Applist,SIGNAL(status(QString)),SLOT(setStatus(QString)));

    connect(ui->Page1Applist,SIGNAL(reload_clicked()),SLOT(onReload()));

    QTimer::singleShot(60*1000,this,SLOT(resetTaskBarIcon()));

    QFile checkPortable("portableUpdater.bat");
    if(checkPortable.exists())
    {
        if(SettingsDialog::shouldCheckVersions())
            updater.activate();
    }else
        ui->actionCheck_for_update->deleteLater();

    connect(&updater,SIGNAL(newer_version_available()),SLOT(showNewVersionAvailable()));
    connect(&updater,SIGNAL(no_new_version(QString)),SLOT(showNoNewVersionAvailable(QString)));
    showMessageBox = false;
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::trayContextMenuFix()
{
    trayicon->contextMenu()->setWindowOpacity(1.0);
}

void MainWindow::show()
{
    move(SettingsDialog::screenCenter()-rect().bottomRight()/2);
    QMainWindow::show();
}

void MainWindow::resetTaskBarIcon()
{
    trayicon->hide();
    trayicon->show();
}

void MainWindow::onTrayDoubleClicked(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::DoubleClick)
    {
        show();
        raise();
    }
}

void MainWindow::setStatus(QString text)
{

}


void MainWindow::newVersionMessage()
{
    if(!this->isVisible())
    {
        if(ui->Page1Applist->version_updates_avail)
            trayicon->showMessage("New versions","There are new versions available for download");
    }
}

void MainWindow::timerEvent()
{
    ui->Page1Applist->on_btUpdate_clicked();
}

void MainWindow::closeEvent(QCloseEvent *evt)
{
    if(evt->spontaneous())
    {
        if( SettingsDialog::getCloseMode() == SettingsDialog::ASK )
        {
            int ans = QMessageBox::information(this,"Closing Winapp_Manager",
                                               "Close Winapp_Manager?\n If not, Winapp_Manager will minimize to tray and check every hour for updates",
                                               QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel
                                               );
            if(ans == QMessageBox::Cancel)
                evt->ignore();
            if(ans == QMessageBox::Yes)
            {
		if(!ui->Page2TaskList->isEmpty())
		{
		    QMessageBox::warning(this,"Can't close","There are still running tasks in the tasklist!");
		    evt->ignore();
		    return;
		}
                evt->accept();
                QApplication::quit();
            }
            if(ans == QMessageBox::No)
            {
                evt->ignore();
                this->hide();
                timer_update.start();
            }
        }
        if( SettingsDialog::getCloseMode() == SettingsDialog::MINIMIZE )
        {
            evt->ignore();
            this->hide();
            timer_update.start();
        }

        if( SettingsDialog::getCloseMode() == SettingsDialog::CLOSE )
        {
            evt->accept();
            QApplication::quit();
        }
    }else
    {
	if(!ui->Page2TaskList->isEmpty())
	{
	    QMessageBox::warning(this,"Can't close","There are still running tasks in the tasklist!");
	    evt->ignore();
	    return;
	}
        evt->accept();
        QApplication::quit();
    }

}

void MainWindow::on_actionQuit_triggered()
{
    close();
}

void MainWindow::on_actionSettings_triggered()
{
    SettingsDialog sdlg(this);
    sdlg.exec();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox aboutmsg;
    aboutmsg.setIconPixmap(QPixmap(":icons/WinApp_Manager.ico"));
    QString about;
    about = tr("<h3>WinApp_Manager - %1</h3>\n").arg(SettingsDialog::currentVersion()) +
	    "WinApp_Manager is a free program. We are grateful to SourceForge.net for our project hosting.\nThis Program is available for Windows 98 and later.\n If you want to help keeping the information up to date just go to our homepage and do it :"+
            "\n\n<h3>Licence:</h3>GPL\n"+
	    "<h3>Website:</h3> <a href=\"http://appdriverupdate.sourceforge.net/\">http://appdriverupdate.sourceforge.net</a>";
    aboutmsg.setText(about);
    aboutmsg.exec();

}

void MainWindow::on_actionAboutQt_triggered()
{
    QMessageBox::aboutQt(this,"aboutQt");
}

void MainWindow::onReload()
{
    if(ui->Page2TaskList->isEmpty())
        ui->Page1Applist->loadList();
    else
        QMessageBox::information(this,"Unable to reload","Can't reload list while there are running tasks!");
}


void MainWindow::on_actionRequest_Software_Support_triggered()
{
    QDesktopServices::openUrl(QString("http://appdriverupdate.sourceforge.net/requests/RequestApps.php"));
}

void MainWindow::on_actionCheck_for_update_triggered()
{
    showMessageBox = true;
    updater.check();
}

void MainWindow::showNewVersionAvailable()
{
    trayicon->contextMenu()->actions().last()->setVisible(true);
    if(showMessageBox)
    {
        int answer = QMessageBox::information(this,"New version available","A new version of Winapp_Manager portable is available. Download now?\n You can also click 'Get latest version' in the icontray context-menu to download the new version",QMessageBox::Yes|QMessageBox::No);
        if(answer==QMessageBox::Yes)
            updater.download();
        showMessageBox = false;
    }else
        trayicon->showMessage("New Version Available!","A new version of winapp_manager is available.\n To download click 'Get latest version' in the icontray context-menu");
}


void MainWindow::showNoNewVersionAvailable(QString message)
{
    if(showMessageBox)
    {
        QMessageBox::information(this,"No newer version available",message);
        showMessageBox = false;
    }
}
