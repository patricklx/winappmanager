#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QListWidgetItem>
#include <QTreeWidgetItem>
#include <QSystemTrayIcon>
#include "tasklist_t.h"

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

public slots:
    void show();
    void trayContextMenuFix();
    void setStatus(QString text);

};

#endif // MAINWINDOW_H
