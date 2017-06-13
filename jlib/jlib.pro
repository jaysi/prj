#-------------------------------------------------
#
# Project created by QtCreator 2014-05-13T09:16:32
#
#-------------------------------------------------

QT       -= core gui

TARGET = jlib
TEMPLATE = lib

DEFINES += JLIB_LIBRARY

SOURCES += \
    src/xtea.c \
    src/sizet.c \
    src/sha256.c \
    src/rc4.c \
    src/posix_ring_buf.c \
    src/memmem.c \
    src/jtable.c \
    src/jstruct.c \
    src/jssif_table.c \
    src/jssif.c \
    src/jss.c \
    src/jrand.c \
    src/jparse.c \
    src/jpack.c \
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
    src/jmempool.c \
    src/jhash.c \
    src/jer.c \
    src/jdbif_type.c \
    src/jdbif_table.c \
    src/jdbif_debug.c \
    src/jdbif_data.c \
    src/jdbif_cell.c \
    src/jdbif.c \
    src/jdb_wr_thread.c \
    src/jdb_undo.c \
    src/jdb_type.c \
    src/jdb_table.c \
    src/jdb_snap.c \
    src/jdb_pack.c \
    src/jdb_map.c \
    src/jdb_lock.c \
    src/jdb_list.c \
    src/jdb_jrnl.c \
    src/jdb_io.c \
    src/jdb_index.c \
    src/jdb_hash.c \
    src/jdb_h.c \
    src/jdb_fav.c \
    src/jdb_data_ptr.c \
    src/jdb_data.c \
    src/jdb_cell.c \
    src/jdb_block.c \
    src/jdb.c \
    src/jcs.c \
    src/jcalc.c \
    src/j_if.c \
    src/hardio.c \
    src/ecc.c \
    src/des.c \
    src/debug.c \
    src/crc.c \
    src/aes.c \
    include/jpacki.c \
    include/jdb_blocki.c \
    src/jnmod/schat.c \
    src/jnmod/jnmod.c \
    src/jnmod/fileshare.c \
    src/jnmod/echo.c

HEADERS += \
    include/xtea.h \
    include/sha256.h \
    include/ring_buf.h \
    include/rc4.h \
    include/ph.h \
    include/memmem.h \
    include/jtypes.h \
    include/jtime.h \
    include/jtable.h \
    include/jstruct.h \
    include/jss.h \
    include/jrand.h \
    include/jparse.h \
    include/jpack.h \
    include/jnet_internals.h \
    include/jnet.h \
    include/jlog.h \
    include/jlist.h \
    include/jhash.h \
    include/jer.h \
    include/jdb_types.h \
    include/jdb_mem.h \
    include/jdb_lock.h \
    include/jdb_list.h \
    include/jdb_intern.h \
    include/jdb_index.h \
    include/jdb_diff.h \
    include/jdb_debug.h \
    include/jdb_chlog.h \
    include/jdb_block.h \
    include/jdb.h \
    include/jcs.h \
    include/jcrypt.h \
    include/jconst.h \
    include/jcompat.h \
    include/jbits.h \
    include/j_if.h \
    include/j2d.h \
    include/hardio.h \
    include/ecc.h \
    include/dll.h \
    include/des.h \
    include/debug.h \
    include/crc.h \
    include/aes.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

OTHER_FILES += \
    src/Makefile
