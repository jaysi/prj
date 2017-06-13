#include "include/ilink_intern.h"

#define _deb1 _NullMsg
#define _deb_accept _NullMsg
#define _deb_connect _NullMsg

#ifdef _WIN32
#define inet_ntop(family, src, dst, size) WSAAddressToStringA(src, family, NULL, dst, ((LPDWORD)(&(size))))
#endif

error13_t ilink_def_conf(struct ilink_conf* conf){

    conf->accept_callback = NULL;
    conf->accept_callback_ctx = NULL;
    conf->addr = ILINK_DEF_ADDR;
    conf->asym_alg = ILINK_DEF_ASYM_ALG;
    conf->datalink = ILINK_DEF_DATALINK;
    conf->handshake = ILINK_DEF_HANDSHAKE;
    conf->inaddr[0] = '\0';
    conf->port = ILINK_DEF_PORT;
    conf->sym_alg = ILINK_DEF_SYM_ALG;
    conf->time_login = ILINK_DEF_LOGIN_TIME;
    conf->time_trans = ILINK_DEF_TRANS_TIME;
    conf->transport = ILINK_DEF_TRANSPORT;
    conf->type = ILINK_TYPE_EMPTY;

    return E13_OK;

}

error13_t _ilink_init_link(struct infolink* link){
    memset(link, 0, sizeof(struct infolink));
    link->e = NULL;
    link->linkpool = NULL;
    link->acceptlink = NULL;
    link->mseg = -1;
    link->refcount = 0;
    link->send_list.first = NULL;
    link->rcvd_list.first = NULL;
    link->send_list.n = 0UL;
    link->rcvd_list.n = 0UL;
    link->next = NULL;
    link->ext_ctx = NULL;
    return E13_OK;
}

error13_t _ilink_realloc_link(struct infolink* link){
    return e13_cleanup(link->e);
}

error13_t _ilink_free_link(struct infolink* link){
    if(link->acceptlink){
        free(link->acceptlink);
        link->acceptlink = NULL;
    }
    return E13_OK;
}

error13_t _ilink_accept0(struct infolink* acceptlink, struct infolink** link){

    //accept a simple incoming tcp request

    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    char ch;

    if (listen(acceptlink->sock, ILINK_DEF_BACKLOG) == -1) {
        _deb_accept("listen failed");
        return e13_ierror(acceptlink->e, E13_SYSE, "s", "listen()");
    }

#ifndef _WIN32
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        return e13_ierror(acceptlink->e, E13_SYSE, "s", "sigaction()");
    }
#endif

    *link = m13_malloc(sizeof(struct infolink));

    if(!(*link)){

        _deb_accept("malloc failed");
        return e13_ierror(acceptlink->e, E13_NOMEM, "s", "accept()");

    }

    _ilink_init_link(*link);
    memcpy(&((*link)->conf), &acceptlink->conf, sizeof(struct ilink_conf));

    sin_size = sizeof their_addr;

    _deb_accept("accepting connections on port %s", acceptlink->conf.port);

    (*link)->sock = accept(acceptlink->sock, (struct sockaddr *)&their_addr, &sin_size);

    _deb_accept("accept() returns sock: %i", (*link)->sock);

    if ((*link)->sock == -1) {

        //try to check if acceptlink->sock is closed, if true return INTERRUPT;
        //TODO: fix this in windows!

        _deb_accept("listen failed");

        if( recv(acceptlink->sock, &ch, 1,
    #ifndef _WIN32
                MSG_DONTWAIT
    #else
                0
    #endif
                ) == -1 ) {
            _deb_accept("interrupted");
            return e13_ierror(acceptlink->e, E13_INTERRUPT, "s", "accept()");
        } else {
            _deb_accept("continue");
            e13_warn(acceptlink->e, E13_SYSE, "s", "accept()");
            return E13_CONTINUE;
        }
    }

    sin_size = sizeof (*link)->conf.inaddr;

    inet_ntop(their_addr.ss_family,
        get_in_addr((struct sockaddr *)&their_addr),
        (*link)->conf.inaddr, sin_size);

    (*link)->conf.saddr_len = sizeof their_addr;
    memcpy(&((*link)->conf.saddr), (struct sockaddr *)&their_addr, sizeof(struct sockaddr));


    (*link)->acceptlink = acceptlink;
    (*link)->conf.type = ILINK_TYPE_SERVER;
    (*link)->flags = ILINK_DEF_FLAG_SERVER;

    _deb_accept("accepted connection from %s", (*link)->conf.inaddr);

    _ilink_init_asym_crypto(*link, NULL, 0);
    _ilink_init_sym_crypto(*link, NULL, 0);
    _deb_accept("sym_crypt_MAGIC: %x", (*link)->sym_crypt.magic);

    if(acceptlink->conf.accept_callback){
        _deb_accept("callback");
        return acceptlink->conf.accept_callback(acceptlink->conf.accept_callback_ctx, *link);
    }

    return E13_OK;
}

error13_t ilink_accept(struct infolink* acceptlink, struct infolink **link){

    if( (   acceptlink->conf.datalink & ILINK_DATALINK_IP4 ||
            acceptlink->conf.datalink & ILINK_DATALINK_IP6   ) &&
        (   acceptlink->conf.transport & ILINK_TRANSPORT_TCP ) &&
        (   acceptlink->conf.handshake == ILINK_HANDSHAKE_NONE)
      ) {

        return _ilink_accept0(acceptlink, link);

    }

    _deb_accept("protocol not implemented, datalink: %i, transport: %i, handshake: %i",
                acceptlink->conf.datalink,
                acceptlink->conf.transport,
                acceptlink->conf.handshake
                );

    return e13_error(E13_IMPLEMENT);

}

error13_t ilink_init(struct infolink *link, struct ilink_conf *conf){

    error13_t ret;

    int sockt, family;

    _ilink_init_link(link);

    if(conf) memcpy(&link->conf, conf, sizeof(struct ilink_conf));
    else ilink_def_conf(&link->conf);

    _deb1("ok");

    switch(link->conf.datalink & ILINK_DATALINK_MASK){
    case (ILINK_DATALINK_IP):
        family = AF_UNSPEC;
        break;
    case ILINK_DATALINK_IP4:
        family = AF_INET;
        break;
    case ILINK_DATALINK_IP6:
        family = AF_INET6;
        break;
    default:
        return e13_error(E13_IMPLEMENT);
        break;
    }

    switch(link->conf.transport & ILINK_TRANSPORT_MASK){
    case ILINK_TRANSPORT_TCP:
        sockt = SOCK_STREAM;
        break;
    case ILINK_TRANSPORT_UDP:
        sockt = SOCK_DGRAM;
        break;
    default:
        return e13_error(E13_IMPLEMENT);
        break;
    }

    _deb1("ok");

    if((ret = _ilink_init_sock(link, link->conf.port, family, sockt)) != E13_OK){

        return ret;

    }

    _deb1("ok");

    //if(sockt == SOCK_STREAM) {_deb1("SOCK STREAM"); listen(link->sock, 10);}

    _deb1("ok");

    //both random keys in listen socket! (NULL)

    if((ret = _ilink_init_asym_crypto(link, NULL, 0)) != E13_OK){

        return ret;

    }

    _deb1("ok");

    if((ret = _ilink_init_sym_crypto(link, NULL, 0)) != E13_OK){

        return ret;

    }

    _deb1("ok");

    link->conf.type = ILINK_TYPE_LISTEN;
    link->flags = ILINK_DEF_FLAG_LISTEN;

    return E13_OK;

}

error13_t _ilink_connect0(struct infolink* link){

    ilink_sock_t sockfd;
    struct addrinfo hints, *servinfo, *p;
    socklen_t sin_size;
    int rv;
    int family;

    switch(link->conf.datalink){
    case ILINK_DATALINK_IP:
        family = AF_UNSPEC;
        break;
    case ILINK_DATALINK_IP4:
        family = AF_INET;
        break;
    case ILINK_DATALINK_IP6:
        family = AF_INET6;
        break;
    default:
        return e13_error(E13_IMPLEMENT);
        break;
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = family;

    if ((rv = getaddrinfo(link->conf.addr, link->conf.port, &hints, &servinfo)) != 0) {
    		_deb_connect("fails here");
        return e13_ierror(link->e, E13_SYSE, "s", gai_strerror(rv));
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            e13_warn(link->e, E13_SYSE, "s", "socket()");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
        		_deb_connect("connect returns -1 but not that serious!");
#ifdef _WIN32
        closesocket(sockfd);
#else
        close(sockfd);
#endif
            e13_warn(link->e, E13_SYSE, "s", "connect()");
            continue;
        }

        break;
    }

    if (p == NULL) {
        freeaddrinfo(servinfo);
        _deb_connect("fails here");
        return e13_ierror(link->e, E13_SYSE, "s", "connect()");
    }

    link->saddr_len = p->ai_addrlen;

    sin_size = sizeof link->conf.inaddr;

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            link->conf.inaddr, sin_size);

    memcpy(&link->saddr, (struct sockaddr *)p->ai_addr, sizeof(struct sockaddr));

    freeaddrinfo(servinfo); // all done with this structure

    link->conf.type = ILINK_TYPE_CLIENT;
    link->flags = ILINK_DEF_FLAG_CLIENT;
    link->sock = sockfd;

    _ilink_init_asym_crypto(link, NULL, 0);
    _ilink_init_sym_crypto(link, NULL, 0);

    return E13_OK;
}

error13_t ilink_connect(struct infolink* link, struct ilink_conf* conf){

    _ilink_init_link(link);

    if(conf) memcpy(&link->conf, conf, sizeof(struct ilink_conf));
    else ilink_def_conf(&link->conf);

    link->e = (struct e13*)malloc(sizeof(struct e13));
    if(!link->e) return e13_error(E13_NOMEM);

    if(e13_init(link->e, E13_MAX_WARN_DEF, E13_MAX_ESTR_DEF, LIB13_ILINK) != E13_OK){
        free(link->e);
        link->e = NULL;
        return e13_error(E13_ERROR);
    }

#ifdef _WIN32
    WSADATA wsaData;   // if this doesn't work
    //WSAData wsaData; // then try this instead

    // MAKEWORD(1,1) for Winsock 1.1, MAKEWORD(2,0) for Winsock 2.0:

    if (WSAStartup(MAKEWORD(2,0), &wsaData) != 0) {
    		_deb_connect("fails here!");
        return e13_ierror(link->e, E13_SYSE, "s", "wsastartup()");
    }
#endif

    if( (   link->conf.datalink & ILINK_DATALINK_IP4 ||
            link->conf.datalink & ILINK_DATALINK_IP6   ) &&
        (   link->conf.transport & ILINK_TRANSPORT_TCP ) &&
        (   link->conf.handshake == ILINK_HANDSHAKE_NONE)
      ) {

        return _ilink_connect0(link);

    }

    return e13_error(E13_IMPLEMENT);

}

error13_t ilink_disconnect(struct infolink* link, ilink_dc_t type){

    int shut = -1;

    if(type & ILINK_DC_RECV){
        shut++;
    }

    if(type & ILINK_DC_SEND){
        if(!shut) shut = 2;
        else shut = 1;
    }

    if(shut > -1){
        shutdown(link->sock, shut);
    }

    if(type & ILINK_DC_CLOSE){
#ifdef _WIN32
        closesocket(link->sock);
#else
        close(link->sock);
#endif
    }

    if(type & ILINK_DC_FREE){

        _ilink_free_link(link);

        if(link->conf.type == ILINK_TYPE_SERVER && link->acceptlink){
            //m13_pool_free(&link->acceptlink->linkpool, link->mseg);
        }
    }

    return E13_OK;
}

error13_t ilink_destroy(struct infolink *link){
    return e13_error(E13_IMPLEMENT);
}


