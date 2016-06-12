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


release: DESTDIR = build/release
debug: DESTDIR = build/debug




CONFIG += static
LIBS += -L$$[QT_INSTALL_PLUGINS]/imageformats -lqico
LIBS += -L$$[QT_INSTALL_PLUGINS]/platforms -lqwindows

QMAKE_LFLAGS += -static-libgcc -static-libstdc++ -static


contains(CONFIG,release64){
    message (64 bit build)
    message ($$PWD/src)
    OPENSSL = E:/programming/tools/openssl/dist64/bin
    LIBS += -L$$YAML/build/release64 -lyaml-cpp
    DESTDIR = build/release64
}else{
    OPENSSL = E:/programming/tools/openssl/dist/bin
    LIBS += -L$$YAML/build/release/ -lyaml-cpp
}

#LIBS += -L$$OPENSSL -llibeay32 -lssleay32
LIBS += -lShlwapi

#QMAKE_CXXFLAGS+=-g -fexceptions
#QMAKE_STRIP=
#QMAKE_LFLAGS_RELEASE=


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

QMAKE_CXXFLAGS += -Wuninitialized

contains(CONFIG, mydebug){
    QMAKE_CXXFLAGS_RELEASE += -g
    QMAKE_CFLAGS_RELEASE += -g
    QMAKE_LFLAGS_RELEASE =
}else{
    QMAKE_LFLAGS_RELEASE += -Wl,--gc-sections
}




INCLUDEPATH += $$PWD/../yaml
DEPENDPATH += $$PWD/../yaml

DISTFILES +=

