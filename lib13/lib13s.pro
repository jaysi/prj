#-------------------------------------------------
#
# Project created by QtCreator 2015-02-08T17:40:32
#
#-------------------------------------------------

QT       -= core gui

TARGET = lib13s
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    xtea.c \
    str13.c \
    sha256.c \
    rr13.c \
    rc4.c \
    rand13.c \
    path13.c \
    pack13.c \
    obj13.c \
    num2text.c \
    mem13.c \
    lock13.c \
    lib13.c \
    io13i.c \
    io13.c \
    hash13.c \
    error13.c \
    des.c \
    db13.c \
    day13.c \
    crypt13.c \
    const13.c \
    base64.c \
    arg13.c \
    aes.c \
    acc13.c \
    include/pack13i.c

HEADERS += \
    include/xtea.h \
    include/type13.h \
    include/thread13.h \
    include/str13.h \
    include/sha256.h \
    include/rr13.h \
    include/rc4.h \
    include/path13.h \
    include/pack13.h \
    include/obj13.h \
    include/msg13.h \
    include/mem13.h \
    include/lock13.h \
    include/lib13.h \
    include/io13i.h \
    include/io13.h \
    include/hash13i.h \
    include/hash13.h \
    include/error13.h \
    include/end13.h \
    include/des.h \
    include/debug13.h \
    include/db13i.h \
    include/db13.h \
    include/day13.h \
    include/crypt13i.h \
    include/crypt13.h \
    include/crc.h \
    include/const13.h \
    include/bit13.h \
    include/base64.h \
    include/arg13.h \
    include/aes.h \
    include/acc13.h
unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

LIBS +=  -L$$PWD/link/

win32: LIBS += -llibpthreadGC2 -lws2_32 -ladvapi32
linux: LIBS += -lpthread

unix:!macx|win32: LIBS += -llibsqlite -llibntru
