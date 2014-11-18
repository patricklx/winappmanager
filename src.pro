#-------------------------------------------------
#
# Project created by QtCreator 2010-09-30T22:29:19
#
#-------------------------------------------------

QT += core
QT += gui
QT += widgets
QT += xml
QT += network
TARGET = WinAppManager
TEMPLATE = app
CONFIG += exceptions
CONFIG += openssl-linked

YAML = E:\programming\tools\yaml

INCLUDEPATH += $$YAML/include
INCLUDEPATH += E:\programming\tools\boost

debug: DESTDIR = build/debug
release: DESTDIR = build/release

OPENSSL = E:\programming\tools\openssl\dist\lib
contains(CONFIG,release64){
    message (64 bit build)
    message ($$PWD/src)
    OPENSSL = E:\programming\tools\openssl\dist64\lib
}

LIBS += -lShlwapi

QMAKE_CXXFLAGS+=-g -fexceptions
QMAKE_STRIP=
QMAKE_LFLAGS_RELEASE=

contains(CONFIG,release)   {
    message (release build)
   CONFIG += static
   LIBS += -L$$[QT_INSTALL_PLUGINS]/imageformats -lqico
   LIBS += -L$$[QT_INSTALL_PLUGINS]/accessible -lqtaccessiblewidgets
   LIBS += -L$$[QT_INSTALL_PLUGINS]/platforms -lqwindows
   LIBS += -L$$OPENSSL -lcrypto -lssl

   QMAKE_LFLAGS += -static-libgcc -static-libstdc -static
    contains(CONFIG,release64){
        LIBS += -L$$YAML/build/release64 -lyaml-cpp
        DESTDIR = build/release64
    }else{
        LIBS += -L$$YAML/build/release/ -lyaml-cpp
    }
}else{
    LIBS += -L$$YAML/debug/ -lyaml-cpp
}

MOC_DIR = $${DESTDIR}/moc
OBJECTS_DIR = $${DESTDIR}/obj
UI_DIR = gui
INCLUDEPATH += $$PWD

SOURCES += main.cpp\
        mainwindow.cpp \
    settingsdialog.cpp \
    choosedialog.cpp \
    updaterdialog.cpp \
    winapp_manager_updater.cpp \
    utils/util.cpp \
    utils/qstringext.cpp \
    appinfo.cpp \
    appinfo_registry.cpp \
    applist.cpp \
    task.cpp \
    tasklist.cpp \
    mytreewidget.cpp

HEADERS  += mainwindow.h \
    settingsdialog.h \
    choosedialog.h \
    updaterdialog.h \
    winapp_manager_updater.h \
    utils/util.h \
    utils/qstringext.h \
    appinfo_registry.h \
    appinfo.h \
    applist.h \
    task.h \
    tasklist.h \
    mytreewidget.h

FORMS    += gui/mainwindow.ui \
    gui/settingsdialog.ui \
    gui/choosedialog.ui \
    gui/updaterdialog.ui \
    gui/applist.ui \
    gui/tasklist.ui \
    gui/listitem.ui

RESOURCES += \
    resource.qrc


RC_FILE = appico.rc




OTHER_FILES += \
    appico.rc \
    cvs/New Text Document.txt \
    cvs/New Text Document.txt \
    yaml/yaml-cpp.pro \
    test.yaml \
    uml/App.qml \
    uml/Apps.qml \
    uml/Button.qml

QMAKE_CXXFLAGS += -Wuninitialized -fdata-sections -ffunction-sections
QMAKE_LFLAGS_RELEASE += -Wl,--gc-sections




INCLUDEPATH += $$PWD/../yaml
DEPENDPATH += $$PWD/../yaml

