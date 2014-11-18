#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QListWidgetItem>
#include <QTreeWidgetItem>
#include <QSystemTrayIcon>
#include "tasklist.h"
#include "winapp_manager_updater.h"

namespace Ui {
    class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QTimer timer_update;
    QSystemTrayIcon *trayicon;
    winapp_manager_updater updater;
    bool showMessageBox;

private slots:
    void on_actionQuit_triggered();
    void on_actionSettings_triggered();
    void on_actionAbout_triggered();

    void on_actionAboutQt_triggered();

    void closeEvent(QCloseEvent *);
    void timerEvent();

    void onTrayDoubleClicked(QSystemTrayIcon::ActivationReason reason = QSystemTrayIcon::DoubleClick);
    void newVersionMessage();
    void resetTaskBarIcon();

    void on_actionRequest_Software_Support_triggered();

    void on_actionCheck_for_update_triggered();

    void showNewVersionAvailable();
    void showNoNewVersionAvailable(QString message);

    public slots:
    void show();
    void trayContextMenuFix();
    void setStatus(QString text);

};

#endif // MAINWINDOW_H
