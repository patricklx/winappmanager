#include "util.h"
#include "qstringext.h"
#include <QString>
#include <QThread>
#include <QProcessEnvironment>
#include <QFile>
#include <QTime>
#include <windows.h>
#include <Psapi.h>
#include <tlhelp32.h>
#include <shlwapi.h>
#include <stdio.h>

namespace Utils{

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof((arr)[0]))
QString getDefaultBrowser()
{
    HRESULT hr;
    TCHAR szExe[MAX_PATH + 100];
    DWORD cchExe = ARRAY_SIZE(szExe);

    if (SUCCEEDED(hr = AssocQueryString((ASSOCF)0, ASSOCSTR_EXECUTABLE,
                                        TEXT("http"), TEXT("open"), szExe, &cchExe)))
    {
        return QStringExt::fromStdWString(szExe).afterLast("\\");
    }else
        return "";
}


void waitForExternalProcess(QString PrcName)
{
    bool process_found = true;
    while(process_found)
    {
        PROCESSENTRY32 processInfo;
        HANDLE hSnapShot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
        processInfo.dwSize=sizeof(PROCESSENTRY32);

        if( Process32First(hSnapShot,&processInfo) )
        {
            process_found = false;
            do
            {
                QString ProcessName;
                ProcessName = QString::fromStdWString( processInfo.szExeFile ); //Name
                if( ProcessName == PrcName)
                {
                    process_found = true;
                    break;
                }
            }while(Process32Next(hSnapShot,&processInfo)!=FALSE);
        }
        QThread::msleep(250);
    }
}


bool existsInEnvironment(QString file)
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment ();
    QString PATH = env.value("PATH");
    QStringList list = PATH.split(';');

    for(int i=0;i < list.count();i++)
    {
        QString path = list.at(i);
        if(!path.endsWith('/'))
            path.append("/");

        QFile qfi(path+file);
        if( qfi.exists() )
        {
            return true;
        }
    }
    return false;
}


bool setRunAsAdmin(QString path)
{
    HKEY key;
    LONG result = RegCreateKey(HKEY_CURRENT_USER,
                               L"Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers\\",
                               &key);

    if( result != ERROR_SUCCESS)
        return false;

    path.replace("/","\\");
    result = RegSetValueExA(key,path.toUtf8(),0,REG_SZ,
                            (const BYTE*)"RUNASADMIN",
                            11);

    RegCloseKey(key);
    if( result != ERROR_SUCCESS)
        return false;

    return true;
}


void unSetRunAsAdmin(QString path)
{
    HKEY key;
    LONG result = RegOpenKeyExA(HKEY_CURRENT_USER,"Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers\\",
                                0,KEY_WRITE,
                                &key);

    if( result != ERROR_SUCCESS)
    {
        qDebug("failed to delete Key: %d",(int)result);
        return;
    }

    path.replace("/","\\");
    result = RegDeleteValueA(key,path.toUtf8());
    RegCloseKey(key);

    if( result != ERROR_SUCCESS)
        qDebug("failed to delete Key: %d",(int)result);
}

}

