#-------------------------------------------------
#
# Project created by QtCreator 2012-11-11T12:42:19
#
#-------------------------------------------------

QT       += core

QT       -= gui
QT       += network
QT       += sql

TARGET = ftsip
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    console.cpp \
    event.cpp \
    csengine.cpp \
    sdp.cpp \
    rtp.cpp \
    focus.cpp \
    procevent.cpp \
    udp_old.cpp \
    udp.cpp \
    proxy.cpp

OTHER_FILES += \
    README.txt \
    runrtp.sh \
    global.db \
    work.txt

HEADERS += \
    console.h \
    csengine.h \
    intervoip.h \
    version.h \
    udp.h

LIBS += -losipparser2
LIBS += -losip2
LIBS += -leXosip2
# for STUN support only
LIBS += -lortp
