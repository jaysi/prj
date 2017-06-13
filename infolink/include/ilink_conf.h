#ifndef ILINK_CONF_H
#define ILINK_CONF_H

#include "ilink_intern.h"

struct infolink;

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN    46
#endif

#define INET_PORTLEN        6

//datalink
#define ILINK_DATALINK_IP4      (0x00000001<<0)
#define ILINK_DATALINK_IP6      (0x00000001<<1)
#define ILINK_DATALINK_IP       (ILINK_DATALINK_IP6|ILINK_DATALINK_IP4)
#define ILINK_DATALINK_SCRAM    (0xffffffff)
#define ILINK_DATALINK_MASK     (0xffffffff)

//transport
#define ILINK_TRANSPORT_TCP     (0x00000001<<0)
#define ILINK_TRANSPORT_UDP     (0x00000001<<1)
#define ILINK_TRANSPORT_SCRAM   (0x00000001<<31)
#define ILINK_TRANSPORT_MASK    (0xffffffff)

/*          hand-shaking        */

//handshakes
#define ILINK_HANDSHAKE_NONE   0x00
#define ILINK_HANDSHAKE_PROTO1 0x01
#define ILINK_HANDSHAKE_ANY    0xff

//default
#define ILINK_DEF_DATALINK      ILINK_DATALINK_IP
#define ILINK_DEF_TRANSPORT     ILINK_TRANSPORT_TCP
#define ILINK_DEF_HANDSHAKE     ILINK_HANDSHAKE_NONE

#define ILINK_DEF_SYM_KEY_LEN   2048
#define ILINK_DEF_ASYM_KEY_LEN  2048
#define ILINK_DEF_SYM_ALG       CRYPT13_ALG_NONE
#define ILINK_DEF_ASYM_ALG      CRYPT13_ALG_NONE

#define ILINK_DEF_FLAG_LISTEN   0x00
#define ILINK_DEF_FLAG_SERVER   ILINK_FLAG_CONNECT
#define ILINK_DEF_FLAG_CLIENT   ILINK_FLAG_CONNECT

#define ILINK_DEF_ADDR  "localhost"
#define ILINK_DEF_PORT  "13375"//leets!

/*      limmits & defaults     */
#define ILINK_MAX_PKT_SIZE  65000
#define ILINK_RECV_PKT_TMP_BUF_SIZE 2048
#define ILINK_MAX_HANDSHAKE_DATALEN (4*1024)
#define ILINK_DEF_BACKLOG   10
#define ILINK_LINKPOOL_NSEG 10
#define ILINK_ACCEPTLINKPOOL_BUCK 10

#define ILINK_DEF_LOGIN_TIME    (60*1000)//1 mins
#define ILINK_DEF_TRANS_TIME    (5*60*1000)//5 mins

enum ilink_type {
    ILINK_TYPE_EMPTY,
    ILINK_TYPE_LISTEN,
    ILINK_TYPE_SERVER,
    ILINK_TYPE_CLIENT,
    ILINK_TYPE_INVAL
};

struct ilink_conf {

    enum ilink_type type;

    char *addr;//peer address
    char *port;
    char inaddr[INET6_ADDRSTRLEN];
    size_t saddr_len;

    void *accept_callback_ctx;
    error13_t (*accept_callback)(void *accept_callback_ctx, struct infolink* link);

    crypt13_alg_t asym_alg;
    crypt13_alg_t sym_alg;

    uint32_t time_login;//in milliseconds
    uint32_t time_trans;//transaction time, in milliseconds

    ilink_datalink_t datalink;
    ilink_transport_t transport;
    ilink_handshake_t handshake;
    struct sockaddr_storage saddr;

};

#endif // ILINK_CONF_H
