
#include <QtGui/QGuiApplication>
#include "mainwindow.h"
#include "settingsdialog.h"
#include <qplugin.h>
#include <QDir>
#include <QString>
#include <QDebug>
#include <qplatformdefs.h>
#include <QStandardPaths>
#include "utils/qstringext.h"
#include <iostream>
#include <string>
#include <list>
#include <windows.h>
#include "yaml-cpp/yaml.h"

#if defined(QT_NO_DEBUG)
//Q_IMPORT_PLUGIN(QICOPlugin)
//Q_IMPORT_PLUGIN(AccessibleFactory)
//Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
#endif


void myMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context)
    QString txt;
    switch (type) {
        case QtDebugMsg:
            txt = QString("%1").arg(msg);
        break;
        case QtWarningMsg:
            txt = QString("Warning: %1").arg(msg);
        break;
        case QtCriticalMsg:
            txt = QString("Critical: %1").arg(msg);
        break;
        case QtFatalMsg:
            txt = QString("Fatal: %1").arg(msg);
            abort();
    }

    QFile outFile("log.txt");
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << txt << endl;
}


int main(int argc, char *argv[])
{
    QDir::setCurrent(QStringExt(argv[0]).beforeLast('\\'));
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName("WinAppManager");
    //QApplication::setStyle("Fusion");

    QStringList cmdline_args = a.arguments();
    if( cmdline_args.contains("debug") )
    {
        QFile outFile("log.txt");
        outFile.remove();
        qDebug("setting qMsgHandler");
        qInstallMessageHandler(myMessageHandler);
        qDebug("qMsgHandler set");
    }

    a.setQuitOnLastWindowClosed(false);
    SettingsDialog::loadSettings();


    MainWindow w;
    if(!(argc>1 && QString(argv[1])==QString("hidden")))
    {
        w.show();
    }

    int ret =  a.exec();
    SettingsDialog::unLoadSettings();
    return ret;
}


/*

 made public: QModelIndexList selectedIndexes() const;


 added to xmlstream.h:
    void writeAttribute(const QString &qualifiedName,QByteArray &array)
    {
        writeAttribute(qualifiedName,QString(array));
    }

    void writeAttribute(const QString &qualifiedName,QVariant value)
    {
        writeAttribute(qualifiedName,value.toString());
    }
*/
