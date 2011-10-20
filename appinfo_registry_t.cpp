#include "appinfo_registry_t.h"
#include <QSettings>
#include <QStringList>
#include <QApplication>

registry_group_t::registry_group_t(QString base,QString name)
{
    group_name = name;
    group_base = base;
}

void registry_group_t::addKey(QString name,QString value)
{
    keys.append(registry_key_t(name,value));
}

QString registry_group_t::getValue(QString name)
{
    for(int i=0;i<keys.count();i++)
    {
        if(keys[i].key_name == name )
            return keys[i].key_value;
    }
    return QString();
}

int appinfo_registry_t::getCount()
{
    QStringList regBase;
    if( QSysInfo::WordSize == 64 )
    {
        regBase <<  ("HKEY_CURRENT_USER\\Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\");
        regBase <<  ("HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\");
    }
    regBase <<  ("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\");
    regBase <<  ("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\");

    int count=0;
    for(int a=0;a<regBase.count();a++)
    {
        QSettings Registry(regBase[a], QSettings::NativeFormat);
        QStringList childkeys;

        childkeys = Registry.childGroups();
        for(int i=0;i<childkeys.count();i++)
        {
            count++;
        }
    }
    return count;
}

int appinfo_registry_t::getAppsFromRegistry()
{
    QStringList regBase;
    if( QSysInfo::WordSize == 64 )
    {
        qDebug("looking also in Wow6432Node");
        regBase <<  ("HKEY_CURRENT_USER\\Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\");
        regBase <<  ("HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\");
    }
    regBase <<  ("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\");
    regBase <<  ("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\");

    int count=0;
    for(int a=0;a<regBase.count();a++)
    {
        QSettings Registry(regBase[a], QSettings::NativeFormat);
        QStringList childkeys;

        childkeys = Registry.childGroups();
        for(int i=0;i<childkeys.count();i++)
        {
            registry_group_t reg_group(regBase[a],childkeys[i]);

            QSettings valuesettings(regBase[a]+childkeys[i],QSettings::NativeFormat);
            reg_group.addKey("DisplayIcon",valuesettings.value( "DisplayIcon" ).toString());
            reg_group.addKey("QuietUninstallString",valuesettings.value( "QuietUninstallString" ).toString());
            reg_group.addKey("UninstallString",valuesettings.value( "UninstallString" ).toString());
            reg_group.addKey("DisplayVersion",valuesettings.value( "DisplayVersion" ).toString());
            reg_group.displayed_name = valuesettings.value( "DisplayName" ).toString();
            emit registry_app_info(reg_group);
            count++;
        }
    }
    return count;
}
