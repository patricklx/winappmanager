#include <QMessageBox>
#include <QTimerEvent>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMetaType>
#include <QCloseEvent>
#include <QDesktopWidget>
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
    timer_update.setInterval(1000*60*60);//check every hour
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
    connect(trayicon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),SLOT(trayContextMenuFix()));

    trayicon->show();
    trayicon->setContextMenu(menu);

    connect(ui->Page1Applist,SIGNAL(versions_available()),SLOT(newVersionMessage()));

    connect(ui->Page1Applist,SIGNAL(status(QString)),SLOT(setStatus(QString)));

    QTimer::singleShot(60*1000,this,SLOT(resetTaskBarIcon()));
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
        evt->accept();
        QApplication::quit();
    }

}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::quit();
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
    QDate buildDate = QLocale(QLocale::C).toDate(QString(__DATE__).simplified(), QLatin1String("MMM d yyyy"));
    QString about;
    about = tr("<h3>WinApp_Manager - %1</h3>\n").arg(buildDate.toString("yy.M.d")) +
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
