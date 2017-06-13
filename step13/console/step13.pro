TEMPLATE = app
CONFIG += console
CONFIG -= qt

SOURCES += \
    ../lib13/include/pack13i.c \
    ../lib13/str13.c \
    ../lib13/sha256.c \
    ../lib13/rr13.c \
    ../lib13/rc4.c \
    ../lib13/rand13.c \
    ../lib13/path13.c \
    ../lib13/pack13.c \
    ../lib13/obj13.c \
    ../lib13/mem13.c \
    ../lib13/lock13.c \
    ../lib13/io13i.c \
    ../lib13/io13.c \
    ../lib13/hash13.c \
    ../lib13/error13.c \
    ../lib13/ecc.c \
    ../lib13/des.c \
    ../lib13/day13.c \
    ../lib13/crypt13.c \
    ../lib13/const13.c \
    ../lib13/arg13.c \
    ../lib13/aes.c \
    ../lib13/xtea.c \
    ../sqlite/sqlite3.c \
    ui.c \
    step13.c \    
    db.c \
    args.c \
    init.c \
    list.c \
    compare.c \
    modif.c \
    update.c \
    backup.c \
    commit.c

HEADERS += \
    ../lib13/include/xtea.h \
    ../lib13/include/type13.h \
    ../lib13/include/thread13.h \
    ../lib13/include/str13.h \
    ../lib13/include/sha256.h \
    ../lib13/include/rr13.h \
    ../lib13/include/rc4.h \
    ../lib13/include/path13.h \
    ../lib13/include/pack13.h \
    ../lib13/include/obj13.h \
    ../lib13/include/mem13.h \
    ../lib13/include/lock13.h \
    ../lib13/include/lib13.h \
    ../lib13/include/io13i.h \
    ../lib13/include/io13.h \
    ../lib13/include/hash13i.h \
    ../lib13/include/hash13.h \
    ../lib13/include/error13.h \
    ../lib13/include/end13.h \
    ../lib13/include/ecci.h \
    ../lib13/include/ecc.h \
    ../lib13/include/des.h \
    ../lib13/include/debug13.h \
    ../lib13/include/day13.h \
    ../lib13/include/crypt13i.h \
    ../lib13/include/crypt13.h \
    ../lib13/include/crc.h \
    ../lib13/include/const13.h \
    ../lib13/include/bit13.h \
    ../lib13/include/arg13.h \
    ../lib13/include/aes.h \
    ../sqlite/sqlite3.h \
    ui.h \
    str13.h \
    step13.h \
    path13.h \
    io13.h \
    debug13.h \
    db.h \
    day13.h \
    args.h \
    hash13.h \
    lib13.h \
    mem13.h

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../pthread-win32/ -llibpthreadGC2
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../pthread-win32/ -llibpthreadGC2

INCLUDEPATH += $$PWD/../pthread-win32
DEPENDPATH += $$PWD/../pthread-win32

