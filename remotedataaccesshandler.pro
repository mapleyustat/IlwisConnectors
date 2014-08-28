##-------------------------------------------------
#
# Project created by QtCreator 2012-09-19T13:18:13
#
#-------------------------------------------------

CONFIG += plugin
TARGET = remotedataaccesshandler

include(global.pri)

QT = network sql
QT       -= gui

TEMPLATE = lib

DEFINES += REMOTEDATAACCESSHANDLER_LIBRARY

LIBS += -L$$PWD/../libraries/$$PLATFORM$$CONF/core/ -lilwiscore \
        -L$$PWD/../libraries/$$PLATFORM$$CONF/ -llibgeos \
        -L$$PWD/../libraries/$$PLATFORM$$CONF/ -lhttpserver
		
win32:CONFIG(release, debug|release): {
    QMAKE_CXXFLAGS_RELEASE += -O2
}

INCLUDEPATH +=  $$PWD/core \
                $$PWD/../external/geos \
                $$PWD/extensions/httpserver \
                $$PWD/extensions/httpserver/httpserver

DEPENDPATH +=   $$PWD/core \
                $$PWD/../external/geos 

DESTDIR = $$PWD/../libraries/$$PLATFORM$$CONF/extensions/$$TARGET
DLLDESTDIR = $$PWD/../output/$$PLATFORM$$CONF/bin/extensions/$$TARGET

OTHER_FILES += \
    remotedataaccesshandler/remotedataaccesshandler.json

HEADERS += \
    remotedataaccesshandler/remotedataaccesshandlermodule.h \
    remotedataaccesshandler/remotedataaccessrequesthandler.h

SOURCES += \
    remotedataaccesshandler/remotedataaccesshandlermodule.cpp \
    remotedataaccesshandler/remotedataaccessrequesthandler.cpp




