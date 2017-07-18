/*
 * This provides an encrypted stateful dynamic channel over the net
 * Dynamic means reset / change protocol, crypto during the connection
 *
 * Server:
 *
*/

#ifndef INFOLINK_H
#define INFOLINK_H

#include "ilink_types.h"
#include "ilink_conf.h"

#define ILINK_FLAG_CONNECT  (0x0001<<0)
#define ILINK_FLAG_SEND     (0x0001<<1)
#define ILINK_FLAG_RECV     (0x0001<<2)
#define ILINK_FLAG_LISTEN   (0x0001<<3)
#define ILINK_FLAG_ASYM     (0x0001<<4)//asym crypt active, in handshakes mostly
#define ILINK_FLAG_NOPACK   (0x0001<<5)
#define ILINK_FLAG_NOCRC    (0x0001<<6)
#define ILINK_FLAG_PIPE     (0x0001<<7) //act as a pipe, to overcome select
                                        //limmitations in window$ and making
                                        //remote administrations easier
#define ILINK_FLAG_DC       (0x0001<<8) //a dc request has been performed,
                                        //d not accept any other requests
#define ILINK_FLAG_NONBLOCK (0x0001<<9)
#define ILINK_FLAG_TIMEOUT  (0x0001<<10) //timeout has been set, care it!
#define ILINK_FLAG_INTR		(0x0001<<11) //interrupted

struct ilink_pkt_hdr;

struct ilink_buf {

    ilink_datalen_t pos;
    ilink_datalen_t bufsize;
    ilink_data_t* buf;
    struct ilink_buf* next;
};

struct ilink_buf_list {

    uint32_t n;
    struct ilink_buf* first, *last, *cur;//the current is very important, no one should touch it!

};

struct infolink {

    magic13_t magic;

    ilink_flag_t flags;

    int refcount;//refrence counter

    ilink_sock_t sock;

    struct ilink_conf conf;

    struct sockaddr saddr;
    size_t saddr_len;

    struct timeval send_tv, send_remain_tv;
    struct timeval recv_tv, recv_remain_tv;

/*
    ilink_datalen_t lastsent;
    ilink_datalen_t lastrcvd;

    ilink_datalen_t datasent;
    ilink_datalen_t datarcvd;

    ilink_datalen_t totalsentpkt;
    ilink_datalen_t totalrcvdpkt;
*/

    //packet mode i/o

    struct ilink_buf_list send_list;
    struct ilink_buf_list rcvd_list;

//    ilink_datalen_t sendpktsize, pktsentsize;
//    ilink_datalen_t recvpktsize, pktrcvdsize;
//    ilink_data_t* sendbuf;
//    ilink_data_t* recvbuf;

    void* ext_ctx;//external context
    void* tmp_ctx;//temp context, e.g. poll->local pipe->struct

    void* sent_callback_ctx;
    error13_t(*sent_callback)(void* ctx, ilink_data_t* data,
                              ilink_datalen_t datalen);
    void* rcvd_callback_ctx;
    error13_t(*rcvd_callback)(void* ctx, ilink_data_t* data,
                              ilink_datalen_t datalen);

    time_t start_time;
    time_t end_time;

    msegid13_t mseg;

    ilink_poll_flag_t pollflags;

    struct e13* e;

    struct m13_mempool *linkpool;

    struct crypt13 asym_crypt, sym_crypt;

    //plistnext and preadynext for poll
    struct infolink* next, *acceptlink, *plistnext, *preadynext, *popnext;

};

struct infolink_inf {
    char* myaddr, *myport, *peeraddr, *peerport, *status;
};

#define ILINK_CTL_BLOCK_MODE    (0x01<<0)
#define ILINK_CTL_TIMEOUT       (0x01<<1)

struct ilink_ctl_s {

    int blocking;

    uint32_t send_time; //seconds
    uint32_t recv_time; //seconds

};

struct ilink_io {
    struct ilink_ctl_s ctl;
    ilink_datalen_t maxpayload;
};

#define ILINK_POLL_WR   (0x01<<0)
#define ILINK_POLL_RD   (0x01<<1)
#define ILINK_POLL_EX   (0x01<<2)
#define ILINK_POLL_ALL   (ILINK_POLL_WR|ILINK_POLL_RD|ILINK_POLL_EX)
#define ILINK_POLL_NOLOCK (0x01<<3)
#define ILINK_POLL_WR_READY (0x01<<4)
#define ILINK_POLL_RD_READY (0x01<<5)
#define ILINK_POLL_EX_READY (0x01<<6)
#define ILINK_POLL_ALL_READY (ILINK_POLL_WR_READY|ILINK_POLL_RD_READY|ILINK_POLL_EX_READY)

struct ilink_poll_list{

    uint32_t n;
    struct infolink* first, *last;
    ilink_poll_flag_t flags;

    ilink_sock_t fdmax, nreadfds, nwritefds, nexfds;

#ifdef ILINK_POLL_USE_SELECT
    fd_set readfds, writefds, exfds, readfds_bak, writefds_bak, exfds_bak;
#elif defined(ILINK_POLL_USE_POLL)

#endif //ILINK_POLL_USE_POLL

    struct timeval tv;
    th13_mutex_t* mx;
    th13_cond_t cond;

    struct ilink_poll_list* next;
};

typedef uint16_t ilink_dc_t;
#define ILINK_DC_SEND   (0x0001<<0)
#define ILINK_DC_RECV   (0x0001<<1)
#define ILINK_DC_CLOSE  (0x0001<<2)
#define ILINK_DC_FREE   (0x0001<<3)//free ilink struct
#define ILINK_DC_ALL    (0xffff)

/*      exported-functions      */


#ifdef __cplusplus
    extern "C" {
#endif

    //server side: init, accept
    //client side: connect
    //sending: prepare_send, send
    //recieving: recv

    error13_t ilink_set_conf(struct infolink* link,
                             struct ilink_conf* conf);

    error13_t ilink_def_conf(struct ilink_conf* conf);

    error13_t ilink_init(struct infolink* link, struct ilink_conf *conf);

    error13_t ilink_destroy(struct infolink* link);

    error13_t ilink_accept(struct infolink* acceptlink,
                           struct infolink **link);

    error13_t ilink_connect(struct infolink* link, struct ilink_conf* conf);

    error13_t ilink_disconnect(struct infolink* link,
                               ilink_dc_t type);

    error13_t ilink_send(   struct infolink* link,
                            ilink_data_t* data,
                            ilink_datalen_t* len,
                            struct ilink_io* ios);

    error13_t ilink_recv(   struct infolink* link,
                            ilink_data_t* data,
                            ilink_datalen_t* len,
                            struct ilink_io* ios);

    error13_t ilink_reset(struct infolink* link, ilink_flag_t flags);

    error13_t ilink_ctl(    struct infolink* link,
                            struct ilink_ctl_s* ctl,
                            uint8_t flags);

    error13_t ilink_get_ctl(struct infolink* link,
                            struct ilink_ctl_s* ctl);

    //for now there is no need for this to be thread safe
    error13_t ilink_poll(struct ilink_poll_list* list,
                         uint32_t* n,
                         struct infolink*** ready);

    error13_t ilink_poll_init(struct ilink_poll_list* list,
                              ilink_timeout_t to,
                              ilink_poll_flag_t flags,
                              th13_mutex_t* mx);

    error13_t ilink_poll_set(struct ilink_poll_list* list,
                             ilink_timeout_t to,
                             ilink_poll_flag_t flags);

    error13_t ilink_poll_add(struct ilink_poll_list* list,
                             struct infolink* link,
                             uint8_t dir);

    error13_t ilink_poll_rm(struct ilink_poll_list* list,
                            ilink_sock_t sockfd,
                            uint8_t dir);

    //packet
    error13_t ilink_pkt_hdr(struct infolink* link,
                            ilink_data_t* data,
                            ilink_datalen_t datalen,
                            struct ilink_pkt_hdr* hdr,
                            ilink_ppkt_flag_t flags);

    error13_t ilink_explode_pkt(struct infolink* link,
                                ilink_data_t** data,
                                ilink_datalen_t* datalen,
                                ilink_pkt_type_t* type,
                                ilink_ppkt_flag_t flags);

    error13_t ilink_prepare_pkt(struct infolink* link,
                                ilink_data_t* data,
                                ilink_datalen_t datalen,
                                ilink_pkt_type_t type,
                                ilink_ppkt_flag_t flags);

    //info
    error13_t ilink_get_info(struct infolink* link,
                             struct infolink_inf* info);

#define ilink_poll_unlock(list)    MACRO(\
    if((list)->mx) th13_mutex_unlock( (list)->mx );\
    )

#ifdef __cplusplus
    }
#endif

#endif // INFOLINK_H
