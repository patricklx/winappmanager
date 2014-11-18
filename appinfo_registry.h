#ifndef APPINFO_REGISTRY_T_H
#define APPINFO_REGISTRY_T_H

#include <QObject>
#include <QList>
#include <QString>
#include <QMetaType>

class MyRegistryKey
{
public:
    QString key_name;
    QString key_value;

    MyRegistryKey(QString name,QString value){
        key_name = name;
        key_value = value;
    }
    bool contains(QString text){
        return key_value.contains(text,Qt::CaseInsensitive);
    }
};



class RegistryGroup
{
public:
    QString group_name;
    QString group_base;
    QString displayed_name;
    QList<MyRegistryKey> keys;

    RegistryGroup(){}
    RegistryGroup(QString base,QString name);

    void addKey(QString name,QString value);
    QString getValue(QString name);
};


Q_DECLARE_METATYPE(RegistryGroup)


class AppinfoRegistry : public QObject
{
    Q_OBJECT
public:
    int getAppsFromRegistry();
    static int getCount();
    void addGroup(QString base,QString name);
    RegistryGroup* find(QString text);

signals:
    void registry_app_info(RegistryGroup group);
};



#endif // APPINFO_REGISTRY_T_H
