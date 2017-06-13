TEMPLATE = app
CONFIG += console
CONFIG -= qt

SOURCES += \
    sqlite3.c \
    shell.c

HEADERS += \
    sqlite3ext.h \
    sqlite3.h

