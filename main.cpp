
#include <QtGui/QApplication>
#include "mainwindow.h"
#include "settingsdialog.h"
#include <qplugin.h>
#include <QDir>

#if defined(QT_NO_DEBUG)
Q_IMPORT_PLUGIN(qico)
#endif


int main(int argc, char *argv[])
{
    QDir::setCurrent(QString(argv[0]).beforeLast('\\'));
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);
    SettingsDialog::loadSettings();

    MainWindow w;
    if(!(argc>0 && QString(argv[1])==QString("hidden")))
    {
        w.show();
    }

    return a.exec();
    SettingsDialog::unLoadSettings();
}
