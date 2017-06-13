#-------------------------------------------------
#
# Project created by QtCreator 2016-05-11T17:55:54
#
#-------------------------------------------------

QT       -= core gui

TARGET = libnm
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    svr_init.c

HEADERS += \
    include/netmetre.h \
    include/nm_msg.h \
    include/nm_internal.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}
