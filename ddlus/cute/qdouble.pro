QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qdouble
TEMPLATE = app

RESOURCES = qnmui.qrc
RC_FILE = appicon.rc

LIBS = libpthreadGC2

SOURCES += main.cpp\
    ../console/str.c \
    ../console/stack.c \
    ../console/parse.c \
    ../console/num2text_en.c \
    ../console/num2text.c \
    ../console/if.c \
    ../console/history.c \
    ../console/func.c \
    ../console/format.c \
    ../console/flow.c \
    ../console/file.c \
    ../console/double.c \
    ../console/date.c \
    ../console/cmd.c \
    ../console/calc.c \
    ../console/array.c \
    ../console/analyze.c \
    ../console/var.c \
    mainwindow.cpp \
    keypad.cpp \
    update.cpp \
    configdialog.cpp \
    aboutdialog.cpp \    
    finddialog.cpp \
    plotdialog.cpp \
    arraydialog.cpp \
    ../console/combine.c


HEADERS  += mainwindow.h \    
    ../console/double.h \
    update.h \
    plotdialog.h \
    keypad.h \
    finddialog.h \
    cutemsg.h \
    configdialog.h \
    arraydialog.h \
    aboutdialog.h


FORMS    += mainwindow.ui \
    keypad.ui \
    configdialog.ui \
    aboutdialog.ui \
    finddialog.ui \
    plotdialog.ui \
    arraydialog.ui

OTHER_FILES += \
    appicon.rc \
    TODO \
    android/AndroidManifest.xml \
    android/res/layout/splash.xml \
    android/res/values/libs.xml \
    android/res/values/strings.xml \
    android/res/values-de/strings.xml \
    android/res/values-el/strings.xml \
    android/res/values-es/strings.xml \
    android/res/values-et/strings.xml \
    android/res/values-fa/strings.xml \
    android/res/values-fr/strings.xml \
    android/res/values-id/strings.xml \
    android/res/values-it/strings.xml \
    android/res/values-ja/strings.xml \
    android/res/values-ms/strings.xml \
    android/res/values-nb/strings.xml \
    android/res/values-nl/strings.xml \
    android/res/values-pl/strings.xml \
    android/res/values-pt-rBR/strings.xml \
    android/res/values-ro/strings.xml \
    android/res/values-rs/strings.xml \
    android/res/values-ru/strings.xml \
    android/res/values-zh-rCN/strings.xml \
    android/res/values-zh-rTW/strings.xml \
    android/src/org/kde/necessitas/ministro/IMinistro.aidl \
    android/src/org/kde/necessitas/ministro/IMinistroCallback.aidl \
    android/src/org/qtproject/qt5/android/bindings/QtActivity.java \
    android/src/org/qtproject/qt5/android/bindings/QtApplication.java \
    android/version.xml
