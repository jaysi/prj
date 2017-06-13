#ifndef ILINK_INTERN_H
#define ILINK_INTERN_H

#ifdef WIN32
#define ILINK_POLL_USE_SELECT
#else
#define ILINK_POLL_USE_SELECT
#endif

#include "../../lib13/include/lib13.h"
#include "infolink.h"

struct infolink;

#define ILINK_HANDSHAKE_REPLY_FAIL  ( 0x00 )
#define ILINK_HANDSHAKE_REPLY_OK    ( 0x01 )
#define ILINK_HANDSHAKE_REPLY_NEXT  ( 0x02 )//continue to next phase

#define ILINK_HS_REQ_PACKSTR    "CH"
#define ILINK_HS_REQ_HDR_SIZE   ( 1+2 )
struct ilink_handshake_request {
    uint8_t hs_proto;
    ilink_datalen_t datalen;
    ilink_data_t* data;
}__attribute__((packed));

#define ILINK_HS_REPCODE_FAIL   0x00
#define ILINK_HS_REPCODE_OK     0x01
#define ILINK_HS_REPCODE_AGAIN  0x02

#define ILINK_HS_REPLY_PACKSTR  "CCH"
#define ILINK_HS_REPLY_HDR_SIZE (1+1+2)
struct ilink_handshake_reply{
    uint8_t hs_reply_code;
    uint8_t hs_proto;
    uint8_t hs_ilink_version;
    ilink_datalen_t datalen;
    ilink_data_t* data;
}__attribute__((packed));

/*      packet-structure        */

#define ILINK_PKT_TYPE_EMPTY    (0x0000)
#define ILINK_PKT_TYPE_TEST     (0x0001)
#define ILINK_PKT_TYPE_INVAL      (0x00ff)
#define ILINK_PKT_TYPE_NOP  ILINK_PKT_TYPE_EMPTY
#define ILINK_PKT_TYPE_RESERVED 256 //reserved packet types

#define ILINK_PPKT_ASYM     (0x01<<0)
#define ILINK_PPKT_NOPACK   (0x01<<1)
#define ILINK_PPKT_NOCRC    (0x01<<2)
#define ILINK_PPKT_COPY     (0x01<<3)//for now used in explode (for rcvd) pkts

#define ILINK_PKT_HDR_PACKSTR   "HLLL"
#define ILINK_PKT_HDR_SIZE      (2+4+4+4)
struct ilink_pkt_hdr{
    ilink_pkt_type_t type;
    ilink_pkt_id_t id;
    ilink_datalen_t datalen;
    uint32_t datacrc32;
}__attribute__((packed));

struct ilink_packet{
    struct ilink_pkt_hdr hdr;
    ilink_data_t payload;
};

#define ILINK_MODE_BLOCK			0
#define ILINK_MODE_NONBLOCK         1
#define ILINK_MODE_KEEP             2

typedef unsigned long _ilink_blockmode_t;

#ifdef __cplusplus
extern "C" {
#endif

error13_t _ilink_init_asym_crypto(struct infolink* link, uint8_t* key, size_t keylen);
error13_t _ilink_init_sym_crypto(struct infolink* link, uint8_t* key, size_t keylen);
error13_t _ilink_svr_handshake(struct infolink* acceptlink,
                               struct infolink* link);
error13_t _ilink_clt_handshake(struct infolink* link);
void *get_in_addr(struct sockaddr *sa);
error13_t _ilink_init_sock(struct infolink *link,
                           char* port,
                           int family,
                           int socktype);

error13_t _ilink_asym_encrypt(struct infolink* link, ilink_data_t* in, ilink_datalen_t insize, ilink_data_t* out, ilink_datalen_t *outsize);
error13_t _ilink_sym_encrypt(struct infolink* link, ilink_data_t* in, ilink_datalen_t insize, ilink_data_t* out, ilink_datalen_t *outsize);
error13_t _ilink_asym_decrypt(struct infolink* link, ilink_data_t* in, ilink_datalen_t insize, ilink_data_t* out, ilink_datalen_t *outsize);
error13_t _ilink_sym_decrypt(struct infolink* link, ilink_data_t* in, ilink_datalen_t insize, ilink_data_t* out, ilink_datalen_t *outsize);

ilink_ppkt_flag_t _ilink_flags_to_ppkt(ilink_flag_t flags);

#ifdef __cplusplus
}
#endif

#ifndef WIN32
#define _ilink_set_block_mode(sockfd, blockmodeptr)	MACRO( fcntl(sockfd, F_SETFL, *(blockmodeptr)==ILINK_MODE_NONBLOCK?O_NONBLOCK:0); )
#define _ilink_get_block_mode(sockfd)	( fcntl(sockfd, F_GETFL, 0)&O_NONBLOCK )
#else
#define _ilink_set_block_mode(sockfd, blockmodeptr)	MACRO( ioctlsocket(sockfd, FIONBIO, blockmodeptr); )
#endif // WIN32

#define _ilink_inc_link_refcount(mx, link)   MACRO((link)->refcount++;)
#define _ilink_dec_link_refcount(mx, link)   MACRO((link)->refcount--;)

#define _ilink_tv2milli(tv) ((tv).tv_sec*1000) + ((tv).tv_usec/1000)
#define _ilink_milli2tv(to, tvptr) MACRO( ((tvptr)->tv_sec = (to)/1000);((tvptr)->tv_usec = ((to)%1000)*1000);)
#define _ilink_second2tv(to, tvptr) MACRO( ilink_milli2tv(to*1000, tvptr); )
#define _ilink_zerotv(tv) MACRO( (tv).tv_sec = 0; (tv).tv_usec = 0; )
#define _ilink_is_zerotv(tv) ( (!((tv).tv_sec) && !((tv).tv_usec)) )

#define _ilink_is_init(ilink)   ((ilink)->magic == MAGIC13_ILINK?1:0)

#endif // ILINK_INTERN_H
