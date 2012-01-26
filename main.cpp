
#include <QtGui/QApplication>
#include "mainwindow.h"
#include "settingsdialog.h"
#include <qplugin.h>
#include <QDir>
#include <QString>

#if defined(QT_NO_DEBUG)
Q_IMPORT_PLUGIN(qico)
#endif


void myMessageHandler(QtMsgType type, const char *msg)
{
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
    QDir::setCurrent(QString(argv[0]).beforeLast('\\'));
    QApplication a(argc, argv);


    QStringList cmdline_args = a.arguments();
    if( cmdline_args.contains("debug") )
    {
        QFile outFile("log.txt");
        outFile.remove();
        qDebug("setting qMsgHandler");
        qInstallMsgHandler(myMessageHandler);
        qDebug("qMsgHandler set");
    }

    a.setQuitOnLastWindowClosed(false);
    SettingsDialog::loadSettings();

    MainWindow w;
    if(!(argc>0 && QString(argv[1])==QString("hidden")))
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

changes made to qstring.h:

    QString afterLast(QChar word)
    {
        int index = lastIndexOf(word)+1;
        return right(size()-index);
    }

    QString afterLast(QString word)
    {
        int index = lastIndexOf(word)+word.length();
        return right(size()-index);
    }

    QString beforeLast(QChar word)
    {
        int index = lastIndexOf(word);
        return left(index);
    }

    QString beforeLast(QString word)
    {
        int index = lastIndexOf(word);
        return left(index);
    }

    QString afterFirst(QString word)
    {
        int index = indexOf(word)+word.length();
        return right(size()-index);
    }
    QString afterFirst(QChar word)
    {
        int index = indexOf(word)+1;
        return right(size()-index);
    }

    QString beforeFirst(QString word)
    {
        int index = indexOf(word);
        return left(index);
    }
    QString beforeFirst(QChar word)
    {
        int index = indexOf(word);
        return left(index);
    }
*/
