#-------------------------------------------------
#
# Project created by QtCreator 2015-02-08T17:44:21
#
#-------------------------------------------------

QT       -= core gui

TARGET = libntrus
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    src/sha2big.c \
    src/sha2.c \
    src/poly.c \
    src/ntru.c \
    src/mgf.c \
    src/key.c \
    src/idxgen.c \
    src/hash.c \
    src/encparams.c \
    src/bitstring.c

HEADERS += \
    src/sph_types.h \
    src/sph_sha2.h \
    src/poly.h \
    src/ntru.h \
    src/mgf.h \
    src/md_helper.h \
    src/key.h \
    src/idxgen.h \
    src/hash.h \
    src/err.h \
    src/encparams.h \
    src/bitstring.h
unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

win32: LIBS += -lws2_32 -ladvapi32
