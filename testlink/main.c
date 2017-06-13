#include <stdio.h>
#include "../infolink/include/infolink.h"

#define _deb1 _DebugMsg

#define PORT "40000"
#define LOCALADDR "127.0.0.1"

#define MSG1    "message raw"
#define MSG2    "message packet"

#ifdef WIN32
#define WSTRERROR() MACRO(\
                            FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,\
                                           NULL, WSAGetLastError(),\
                                           MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),\
                                           &s, 0, NULL);\
                            wcstombs(mbserror, s, 200);\
                            LocalFree(s);\
                        )
#else

#define WSTRERROR() MACRO(strcpy(mbserror, strerror(errno));)

#endif
struct ilink_conf _ilink_conf_def = {
    ILINK_TYPE_EMPTY,
    ILINK_DEF_ADDR,
    ILINK_DEF_PORT,
    "",
    0,
    NULL,
    NULL,
    ILINK_DEF_ASYM_ALG,
    ILINK_DEF_SYM_ALG,
    ILINK_DEF_LOGIN_TIME,
    ILINK_DEF_TRANS_TIME,
    ILINK_DEF_DATALINK,
    ILINK_DEF_TRANSPORT,
    ILINK_DEF_HANDSHAKE
};

void accept_callback(void* ctx, struct infolink* link){

    printf("got connection from %s\n", link->conf.inaddr);

//    ilink_close(link);

}

int run_svr(){

    struct infolink link, *peerlink;
    error13_t ret;
    struct ilink_conf conf;
    ilink_data_t* data;
    ilink_datalen_t datalen;
    ilink_pkt_type_t type;
    char mbserror[200];
    wchar_t *s = NULL;

    printf("init...\n");

    ilink_def_conf(&conf);

    conf.type = ILINK_TYPE_LISTEN;
    conf.port = PORT;

    if((ret = ilink_init(&link, &conf)) != E13_OK){
        wcstombs(mbserror, gai_strerror(errno), 200);
        printf("ierror: %s, uerror: %s, gai: %s\n", e13_ierrmsg(link.e), e13_uerrmsg(link.e), mbserror);
        perror("ilink_init()");
        return -1;
    }

    printf("waiting...\n");

    if((ret = ilink_accept(&link, &peerlink)) != E13_OK){
        WSTRERROR();
        printf("ierror: %s, uerror: %s, gai: %s\n", e13_ierrmsg(link.e), e13_uerrmsg(link.e), mbserror);
        perror("ilink_accept()");
        return -1;
    }

    printf("recieving packet...\n");

    ret = ilink_recv(peerlink, NULL, NULL, NULL);
    if(ret != E13_DONE && ret != E13_OK){
        WSTRERROR();
        printf("ierror: %s, uerror: %s, gai: %s\n", e13_ierrmsg(peerlink->e), e13_uerrmsg(peerlink->e), mbserror);
        perror("ilink_recv()");
        return -1;
    }

    printf("parsing...\n");

    _deb1("data: %s", peerlink->recvbuf + ILINK_PKT_HDR_SIZE);

    if((ret = ilink_explode_pkt(peerlink, &data, &datalen, &type, 0)) != E13_OK){
        WSTRERROR();
        printf("ierror: %s, uerror: %s, gai: %s\n", e13_ierrmsg(peerlink->e), e13_uerrmsg(peerlink->e), mbserror);
        perror("ilink_explode_pkt()");
        return -1;
    }

    printf("MSG (%u bytes): %s\n", datalen, data);

    return 0;

}

int run_clt(){
    char addr[100];

    struct infolink link;
    error13_t ret;
    struct ilink_conf conf;
    ilink_data_t* data;
    ilink_datalen_t datalen;
    char mbserror[200];
    wchar_t *s = NULL;

    printf("address (* = 127.0.0.1): ");
    scanf("%s", addr);
    if(*addr == '*'){
        strcpy(addr, LOCALADDR);
    }

    ilink_def_conf(&conf);

    conf.type = ILINK_TYPE_CLIENT;
    conf.addr = addr;
    conf.port = PORT;

    printf("connecting to %s port %s\n", addr, PORT);

    if((ret = ilink_connect(&link, &conf)) != E13_OK){
        WSTRERROR();
        printf("ierror: %s, uerror: %s, gai: %s\n", e13_ierrmsg(link.e), e13_uerrmsg(link.e), mbserror);
        perror("ilink_connect()");
        return -1;
    }

    printf("connected to %s:%s, sending message packet [ %s ] ...\n", link.conf.inaddr, link.conf.port, MSG1);

    data = MSG1;
    datalen = strlen(MSG1) + 1;

    if((ret = ilink_prepare_pkt(&link, data, datalen, ILINK_PKT_TYPE_RESERVED, 0)) != E13_OK){        
        WSTRERROR();
        printf("ierror: %s, uerror: %s, gai: %s\n", e13_ierrmsg(link.e), e13_uerrmsg(link.e), mbserror);
        perror("ilink_prepare_pkt()");
        return -1;
    }

    ret = ilink_send(&link, NULL, NULL, NULL);
    if(ret != E13_DONE && ret != E13_OK){
        WSTRERROR();
        printf("ierror: %s, uerror: %s, gai: %s\n", e13_ierrmsg(link.e), e13_uerrmsg(link.e), mbserror);
        perror("ilink_send()");
        return -1;
    }

    do{}while(1);

    return 0;

}


int main(void)
{
    char ch;

do_select:

    printf("select client/server/exit ( c / s / e ):");
    scanf("%c", &ch);

    switch(ch){
    case 'c':
    case 'C':
        run_clt();
        break;
    case 's':
    case 'S':
        run_svr();
        break;
    case 'e':
    case 'E':
    case 'x':
    case 'X':
    case 'q':
    case 'Q':
        return 0;
    default:
        printf("select either c or s.\n");
        goto do_select;
    }

    return 0;
}

