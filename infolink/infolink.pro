#-------------------------------------------------
#
# Project created by QtCreator 2013-08-21T08:58:25
#
#-------------------------------------------------

QT       -= core gui

TARGET = infolink
TEMPLATE = lib
#CONFIG += staticlib

DEFINES += INFOLINK_LIBRARY

SOURCES += \
    infolink.c \
    ilink_crypt.c \
    ilink_sock.c \
    ilink_hs.c \
    ilink_io.c \
    ilink_pkt.c \
    ilink_poll_select.c \
    ilink_poll.c

HEADERS += \
    infolink.h \
    ilink_intern.h \
    include/infolink.h \
    include/ilink_types.h \
    include/ilink_intern.h \
    include/ilink_conf.h

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xEED41159
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = infolink.dll
    addFiles.path = !:/sys/bin
    DEPLOYMENT += addFiles
}

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

win32: LIBS += -L$$PWD/../link.lib/win32/debug/dynamic -L$$PWD/../link.lib/win32/release/static

win32: LIBS += -lws2_32 -ladvapi32 -llibpthreadGC2

unix:!macx|win32: LIBS += -llib13 -llibntru
