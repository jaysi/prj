TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c \
    ../libntru/src/sha2big.c \
    ../libntru/src/sha2.c \
    ../libntru/src/poly.c \
    ../libntru/src/ntru.c \
    ../libntru/src/mgf.c \
    ../libntru/src/key.c \
    ../libntru/src/idxgen.c \
    ../libntru/src/hash.c \
    ../libntru/src/encparams.c \
    ../libntru/src/bitstring.c \
    ../lib13/str13.c \
    ../lib13/sha256.c \
    ../lib13/rr13.c \
    ../lib13/rc4.c \
    ../lib13/rand13.c \
    ../lib13/path13.c \
    ../lib13/pack13.c \
    ../lib13/obj13.c \
    ../lib13/num2text.c \
    ../lib13/mem13.c \
    ../lib13/lock13.c \
    ../lib13/lib13.c \
    ../lib13/io13i.c \
    ../lib13/io13.c \
    ../lib13/hash13.c \
    ../lib13/error13.c \
    ../lib13/des.c \
    ../lib13/db13.c \
    ../lib13/day13.c \
    ../lib13/crypt13.c \
    ../lib13/const13.c \
    ../lib13/base64.c \
    ../lib13/arg13.c \
    ../lib13/aes.c \
    ../lib13/acc13.c \
    ../lib13/xtea.c \
    ../lib13/include/pack13i.c \
    ../infolink/infolink.c \
    ../infolink/ilink_sock.c \
    ../infolink/ilink_poll.c \
    ../infolink/ilink_pkt.c \
    ../infolink/ilink_io.c \
    ../infolink/ilink_hs.c \
    ../infolink/ilink_crypt.c \
    ../monetR/monet_sess.c \
    ../monetR/monet_poll.c \
    ../monetR/monet_mod.c \
    ../monetR/monet.c

HEADERS += \
    ../libntru/src/sph_types.h \
    ../libntru/src/sph_sha2.h \
    ../libntru/src/poly.h \
    ../libntru/src/ntru.h \
    ../libntru/src/mgf.h \
    ../libntru/src/md_helper.h \
    ../libntru/src/key.h \
    ../libntru/src/idxgen.h \
    ../libntru/src/hash.h \
    ../libntru/src/err.h \
    ../libntru/src/encparams.h \
    ../libntru/src/bitstring.h \
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
    ../lib13/include/msg13.h \
    ../lib13/include/mem13.h \
    ../lib13/include/lock13.h \
    ../lib13/include/lib13.h \
    ../lib13/include/io13i.h \
    ../lib13/include/io13.h \
    ../lib13/include/hash13i.h \
    ../lib13/include/hash13.h \
    ../lib13/include/error13.h \
    ../lib13/include/end13.h \
    ../lib13/include/des.h \
    ../lib13/include/debug13.h \
    ../lib13/include/db13i.h \
    ../lib13/include/db13.h \
    ../lib13/include/day13.h \
    ../lib13/include/crypt13i.h \
    ../lib13/include/crypt13.h \
    ../lib13/include/crc.h \
    ../lib13/include/const13.h \
    ../lib13/include/bit13.h \
    ../lib13/include/base64.h \
    ../lib13/include/arg13.h \
    ../lib13/include/aes.h \
    ../lib13/include/acc13.h \
    ../infolink/include/infolink.h \
    ../infolink/include/ilink_types.h \
    ../infolink/include/ilink_intern.h \
    ../infolink/include/ilink_conf.h \
    ../monetR/monetR.h \
    ../monetR/monet_user.h \
    ../monetR/monet_sess.h \
    ../monetR/monet_poll.h \
    ../monetR/monet_pkt.h \
    ../monetR/monet_mod.h \
    ../monetR/monet_internal.h \
    ../monetR/monet_fifo.h


win32: LIBS += -L$$PWD/../link.lib/win32/debug/dynamic -L$$PWD/../link.lib/win32/release/static -L$$PWD/../link.lib/win32/debug/static
win32: LIBS += -llibws2_32 -ladvapi32 -llibpthreadGC2 -llibsqlite

#unix:!macx|win32: LIBS += -llibntru -llib13 -linfolink
