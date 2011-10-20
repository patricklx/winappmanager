#ifndef APPINFO_REGISTRY_T_H
#define APPINFO_REGISTRY_T_H

#include <QObject>
#include <QList>
#include <QString>
#include <QMetaType>

class registry_key_t
{
public:
    QString key_name;
    QString key_value;

    registry_key_t(QString name,QString value){
        key_name = name;
        key_value = value;
    }
    bool contains(QString text){
        return key_value.toUpper().contains(text.toUpper());
    }
};



class registry_group_t
{
public:
    QString group_name;
    QString group_base;
    QString displayed_name;
    QList<registry_key_t> keys;

    registry_group_t(){}
    registry_group_t(QString base,QString name);

    void addKey(QString name,QString value);
    QString getValue(QString name);
};


Q_DECLARE_METATYPE(registry_group_t)


class appinfo_registry_t : public QObject
{
    Q_OBJECT
public:
    int getAppsFromRegistry();
    int getCount();
    void addGroup(QString base,QString name);
    registry_group_t* find(QString text);

signals:
    void registry_app_info(registry_group_t group);
};



#endif // APPINFO_REGISTRY_T_H
