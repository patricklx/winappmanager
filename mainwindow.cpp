#include <QMessageBox>
#include <QTimerEvent>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMetaType>
#include <QCloseEvent>
#include <QDesktopWidget>
#include "settingsdialog.h"
#include "choosedialog.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->move(QApplication::desktop()->rect().center()/2);

    connect(ui->Page1Applist,SIGNAL(taskChosen(task_t*)),ui->Page2TaskList,SLOT(addTask(task_t*)));
    connect(ui->Page1Applist,SIGNAL(unSelected(appinfo_t*)),ui->Page2TaskList,SLOT(removeTask(appinfo_t*)));
    connect(ui->Page2TaskList,SIGNAL(onTaskRemoved(appinfo_t*)),ui->Page1Applist,SLOT(onRemovedFromTasks(appinfo_t*)));
    timer_update.setInterval(1000*60*60);//check every hour
    connect(&timer_update,SIGNAL(timeout()),SLOT(timerEvent()));

    trayicon = new QSystemTrayIcon(QIcon(tr(":icons/WinApp_Manager.ico")),this);
    connect(trayicon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),SLOT(onTrayDoubleClicked(QSystemTrayIcon::ActivationReason)));
    connect(trayicon,SIGNAL(messageClicked()),SLOT(onTrayDoubleClicked()));

    QMenu *menu = new QMenu(this);
    QAction *act = menu->addAction(tr("Quit"));
    connect(act,SIGNAL(triggered()),SLOT(close()));

    act = menu->addAction(tr("Show"));
    connect(act,SIGNAL(triggered()),SLOT(show()));
    connect(act,SIGNAL(triggered()),SLOT(raise()));
    connect(act,SIGNAL(triggered()),&timer_update,SLOT(stop()));

    trayicon->show();
    trayicon->setContextMenu(menu);
    timerId_tray = this->startTimer(30*1000);

    timerId_update = timer_update.timerId();
}

MainWindow::~MainWindow()
{
    SettingsDialog::saveSettings();
    delete ui;
}

void MainWindow::onTrayDoubleClicked(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::DoubleClick)
    {
        show();
        raise();
    }
}

void MainWindow::timerEvent(QTimerEvent *evt)
{
    if(evt == NULL)
    {
        ui->Page1Applist->on_btUpdate_clicked();
        if(ui->Page1Applist->version_updates_avail)
            trayicon->showMessage(tr("New versions"),tr("There are new versions available for download"));
    }else
    {
        trayicon->hide();
        trayicon->show();
    }
}

void MainWindow::closeEvent(QCloseEvent *evt)
{
    if(evt->spontaneous())
    {
        if( SettingsDialog::getCloseMode() == SettingsDialog::ASK )
        {
            int ans = QMessageBox::information(this,tr("Closing Winapp_Manager"),
                                               tr("Close Winapp_Manager?\n If not, Winapp_Manager will minimize to tray and check every hour for updates"),
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
    about = tr("<h3>WinApp_Manager - 11.04</h3>\n") +
            tr("\nWinApp_Manager is a free program. We are grateful\n to SourceForge.net for our project hosting.\nThis Program is available for Windows 98 and later.\n If you want to help keeping the information up to date just go to our homepage and do it :)")+
            tr("\n\n<h3>Licence:</h3>GPL\n")+
            tr("<h3>Website:</h3> <a href=\"http://sourceforge.net/apps/phpbb/appdriverupdate/\">sourceforge.net/apps/phpbb/appdriverupdate</a>");
    aboutmsg.setText(about);
    aboutmsg.exec();

}

void MainWindow::on_actionAboutQt_triggered()
{
    QMessageBox::aboutQt(this,tr("aboutQt"));
}
