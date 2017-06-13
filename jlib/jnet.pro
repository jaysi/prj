#-------------------------------------------------
#
# Project created by QtCreator 2014-12-10T10:59:47
#
#-------------------------------------------------

QT       -= core gui

TARGET = jnet
TEMPLATE = lib

DEFINES += JNET_LIBRARY

SOURCES += \
    src/jn_svrif.c \
    src/jn_svr_send.c \
    src/jn_svr_poll.c \
    src/jn_svr_mod.c \
    src/jn_svr_login.c \
    src/jn_svr_com.c \
    src/jn_svr.c \
    src/jn_pkt_gen.c \
    src/jn_fifo.c \
    src/jn_conn.c \
    src/jn_cltif.c \
    src/jn_clt_s.c \
    src/jn_clt_com.c \
    src/jn_clt.c \
    src/jn_block.c \
    src/jn_base.c \
    src/jn_assm.c \
    src/jn.c \
    src/jpack.c

HEADERS += \
    include/jnet_internals.h \
    include/jnet.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
