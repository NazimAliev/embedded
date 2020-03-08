#-------------------------------------------------
#
# Project created by QtCreator 2013-06-08T13:39:39
#
#-------------------------------------------------

QT       += core gui
QT      += network
QT      += sql

TARGET = ftgui
TEMPLATE = app


SOURCES += main.cpp\
        dialog.cpp \
    setup.cpp \
    model.cpp \
    udp.cpp \
    group.cpp \
    event.cpp \
    callmodel.cpp \
    bookmodel.cpp \
    freqmodel.cpp \
    radiomodel.cpp \
    widget.cpp

HEADERS  += dialog.h \
    intersip.h \
    model.h \
    callmodel.h \
    bookmodel.h \
    freqmodel.h \
    radiomodel.h \
    widget.h \
    version.h

FORMS    += dialog.ui

OTHER_FILES += \
    global.db \
    command.txt \
    call.txt

RESOURCES += \
    ftgui.qrc
