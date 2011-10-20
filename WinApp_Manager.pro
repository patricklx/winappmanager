#-------------------------------------------------
#
# Project created by QtCreator 2010-09-30T22:29:19
#
#-------------------------------------------------

QT       += core gui
QT       += xml
QT       += network

TARGET = WinApp_Manager
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    appinfo_t.cpp \
    settingsdialog.cpp \
    choosedialog.cpp \
    updaterdialog.cpp \
    task_t.cpp \
    tasklist_t.cpp \
    applist_t.cpp \
    appinfo_registry_t.cpp \
    eventsink.cpp

HEADERS  += mainwindow.h \
    appinfo_t.h \
    settingsdialog.h \
    choosedialog.h \
    updaterdialog.h \
    task_t.h \
    tasklist_t.h \
    applist_t.h \
    appinfo_registry_t.h \
    eventsink.h

FORMS    += mainwindow.ui \
    settingsdialog.ui \
    choosedialog.ui \
    updaterdialog.ui \
    applist_t.ui \
    tasklist_t.ui

RESOURCES += \
    resource.qrc

    RC_FILE = appico.rc

    CONFIG(release, debug|release)    {
        message (release build)
       CONFIG += static
	LIBS += -L$$[QT_INSTALL_PLUGINS]/imageformats -lqico
       QMAKE_LFLAGS += -static-libgcc
    }

OTHER_FILES += \
    appico.rc \
    cvs/New Text Document.txt \
    cvs/New Text Document.txt

QMAKE_CFLAGS_RELEASE= -O3
QMAKE_CXXFLAGS_RELEASE = -O3
QMAKE_CXXFLAGS += -Wuninitialized

LIBS += -lshlwapi



