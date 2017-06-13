TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c


unix:!macx|win32: LIBS += -L$$PWD -lliblib13
