#-------------------------------------------------
#
# Project created by QtCreator 2015-05-23T17:06:25
#
#-------------------------------------------------

QT       -= core gui

TARGET = infolinks
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    infolink.c \
    ilink_sock.c \
    ilink_poll.c \
    ilink_pkt.c \
    ilink_io.c \
    ilink_hs.c \
    ilink_crypt.c

HEADERS += \
    include/infolink.h \
    include/ilink_types.h \
    include/ilink_intern.h \
    include/ilink_conf.h
unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

INCLUDEPATH += $$PWD/../lib13/include
DEPENDPATH += $$PWD/../lib13/include

OTHER_FILES += \
    TODO

win32: LIBS += -llibws2_32

unix:!macx|win32: LIBS += -L$$PWD/../lib/ -llib13 -llibntru -lpthread
