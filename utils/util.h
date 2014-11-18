#ifndef UTIL_H
#define UTIL_H

#include <QString>

namespace Utils
{

bool existsInEnvironment(QString file);
bool setRunAsAdmin(QString path);
void unSetRunAsAdmin(QString path);
void waitForExternalProcess(QString PrcName);
QString getDefaultBrowser();

}

#endif // UTIL_H
