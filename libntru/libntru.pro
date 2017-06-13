#-------------------------------------------------
#
# Project created by QtCreator 2014-12-27T14:29:29
#
#-------------------------------------------------

QT       -= core gui

TARGET = ntru
TEMPLATE = lib

DEFINES += LIBNTRU_LIBRARY

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
