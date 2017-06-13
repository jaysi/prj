#include "include/ilink_intern.h"

#define _deb_io1 _NullMsg
#define _deb_io2 _NullMsg

//TODO: THIS IS SO UGLY
#define ilink_milli2tv(to, tvptr) MACRO( ((tvptr)->tv_sec = (to)/1000);\
										((tvptr)->tv_usec = ((to)%1000)*1000);)

error13_t ilink_ctl(struct infolink* link,
                    struct ilink_ctl_s* ctl,
                    uint8_t flags){

    unsigned long imode;

    //blocking mode

    if(flags & ILINK_CTL_BLOCK_MODE){

        if(ctl->blocking == ILINK_MODE_BLOCK) {

            imode = ILINK_MODE_BLOCK;
            _ilink_set_block_mode(link->sock, &imode);
            link->flags &= ~ILINK_FLAG_NONBLOCK;

        } else if(ctl->blocking == ILINK_MODE_NONBLOCK){

            imode = ILINK_MODE_NONBLOCK;
            _ilink_set_block_mode(link->sock, &imode);
            link->flags |= ILINK_FLAG_NONBLOCK;

        }
    }

    //time out ops

    if(flags & ILINK_CTL_TIMEOUT){

        if(ctl->send_time){
            _ilink_second2tv(ctl->send_time, &link->send_tv);
            _ilink_zerotv(link->send_remain_tv);
        } else {
            _ilink_zerotv(link->send_tv);
            _ilink_zerotv(link->send_remain_tv);
        }

        if(ctl->recv_time){
            _ilink_second2tv(ctl->recv_time, &link->recv_tv);
            _ilink_zerotv(link->recv_remain_tv);
        } else {
            _ilink_zerotv(link->recv_tv);
            _ilink_zerotv(link->recv_remain_tv);
        }

        if(ctl->send_time || ctl->recv_time){
            link->flags |= ILINK_FLAG_TIMEOUT;
        } else {
            link->flags &= ~ILINK_FLAG_TIMEOUT;
        }

    }

    return E13_OK;

}

error13_t ilink_get_ctl(struct infolink* link,
                        struct ilink_ctl_s* ctl){

    //blocking
#ifndef WIN32
    if(_ilink_get_block_mode(link->sock)) ctl->blocking = 0;
    else ctl->blocking = 1;
#else
    if(link->flags & ILINK_FLAG_NONBLOCK) ctl->blocking = 0;
    else ctl->blocking = 1;
#endif

    //time out
    if(link->flags & ILINK_FLAG_TIMEOUT){

        if(link->recv_tv.tv_sec || link->recv_tv.tv_usec){
            ctl->recv_time = _ilink_tv2milli(link->recv_tv);
        }

        if(link->send_tv.tv_sec || link->send_tv.tv_usec){
            ctl->send_time = _ilink_tv2milli(link->send_tv);
        }

    } else {
        ctl->recv_time = 0UL;
        ctl->send_time = 0UL;
    }

    return E13_OK;
}

error13_t _ilink_recv_stream(   struct infolink* link,
                                ilink_data_t* data,
                                ilink_datalen_t* len,
                                struct ilink_io* ios){

    //_wdeb(L"Code is out-of-order, see recv_pkt_all()");

    int n;
    int has_timeout = 0;

    fd_set fds;
#ifdef WIN32
    struct timeval tv, tv1, tv2;
    uint32_t remain1, remain2;
#endif

    _deb_io2("recv raw mode");

    if(ios && ios->ctl.recv_time){
        has_timeout = 1;
        _ilink_second2tv(ios->ctl.recv_time, &tv);
    }

    if(
            !has_timeout &&
            (
                link->flags & ILINK_FLAG_TIMEOUT &&
                !_ilink_is_zerotv(link->recv_tv)
                )

            ){

        has_timeout = 2;
        if(!_ilink_is_zerotv(link->recv_remain_tv))
            memcpy(&tv, &link->recv_remain_tv, sizeof(struct timeval));
        else
            memcpy(&tv, &link->recv_tv, sizeof(struct timeval));

    }

    if(has_timeout){

        FD_ZERO(&fds);
        FD_SET(link->sock, &fds);

#ifdef WIN32
        if (gettimeofday(&tv1, NULL) == -1) {
            return e13_error(E13_SYSE);
        }
#endif

        switch (select(link->sock + 1, &fds, NULL, NULL, &tv)) {
        	case -1:
            if (errno == EINTR) {
                return e13_error(E13_CONTINUE);
            } else {
                return e13_error(E13_SYSE);
            }
            break;//-1

            case 0:
            	return e13_error(E13_TIMEOUT);
				break;

            default:
				break;
        }

    }//link->flags & ILINK_FLAG_TIMEOUT


    if(has_timeout){

        if (FD_ISSET(link->sock, &fds)) {
            n = recv(link->sock, data, *len, 0);
#ifdef WIN32
            if (gettimeofday(&tv2, NULL) == -1) {
                return e13_error(E13_SYSE);
            }
            remain1 = _ilink_tv2milli(tv1);
            remain2 = _ilink_tv2milli(tv2);
            _deb_io2("remain 1 = %i, remain 2 = %i", remain1, remain2);
            assert(remain2 >= remain1);
            _ilink_milli2tv(remain2 - remain1, &tv);
#endif
        } else {
            return e13_error(E13_TIMEOUT);
        }

        if(has_timeout == 1){

            ios->ctl.recv_time = _ilink_tv2milli(tv)/1000;

        } else if(has_timeout == 2){

            memcpy(&link->recv_remain_tv, &tv, sizeof(struct timeval));

        }

    } else {

        n = recv(link->sock, data, *len, 0);

    }

    if ((n == -1) || (n == 0)) {

        if(errno == EINTR) return E13_CONTINUE;

        return e13_error(E13_DC);

    }

    *len = n;

    if(link->rcvd_callback){
        link->rcvd_callback(link->rcvd_callback_ctx, data, *len);
    }

    return E13_OK;

}

error13_t _ilink_recv_pkt(  struct infolink* link,
                            struct ilink_io* ios){

    //_wdeb(L"Code is out-of-order, see recv_pkt_all()");

    int n;
    error13_t ret;
    char tmpbuf[ILINK_RECV_PKT_TMP_BUF_SIZE];
    char* buf;
    size_t len;
    struct ilink_pkt_hdr hdr;
    int newbuf = 0;
    struct ilink_buf* ibuf;

    int has_timeout = 0;

    fd_set fds;
#ifdef WIN32
    struct timeval tv, tv1, tv2;
    uint32_t remain1, remain2;
#endif


    _deb_io2("recv pkt mode, link->recvbuf = %s",
			!link->rcvd_list.first?"NULL":link->rcvd_list.first->buf);

    if(!link->rcvd_list.first ||
       ((link->rcvd_list.last) &&
        (link->rcvd_list.last->bufsize == link->rcvd_list.last->pos))
        ){
        _deb_io2("first empty");
new_buf:
        buf = tmpbuf;
        len = ILINK_RECV_PKT_TMP_BUF_SIZE;
        newbuf = 1;
    } else {
        _deb_io2("first not empty");
        if(link->rcvd_list.last->bufsize > link->rcvd_list.last->pos){
            _deb_io2("continue previous");
            buf = link->rcvd_list.last->buf + link->rcvd_list.last->pos;
            len = link->rcvd_list.last->bufsize - link->rcvd_list.last->pos;
        } else {
            _deb_io2("new buf");
            goto new_buf;
        }
    }

    if(ios && ios->ctl.recv_time){
        has_timeout = 1;
        _ilink_second2tv(ios->ctl.recv_time, &tv);
    }

    if(
            !has_timeout &&
            (
                link->flags & ILINK_FLAG_TIMEOUT &&
                !_ilink_is_zerotv(link->recv_tv)
                )

            ){

        has_timeout = 2;
        if(!_ilink_is_zerotv(link->recv_remain_tv))
            memcpy(&tv, &link->recv_remain_tv, sizeof(struct timeval));
        else
            memcpy(&tv, &link->recv_tv, sizeof(struct timeval));

    }

    if(has_timeout){

        FD_ZERO(&fds);
        FD_SET(link->sock, &fds);

#ifdef WIN32
        if (gettimeofday(&tv1, NULL) == -1) {
            return e13_error(E13_SYSE);
        }
#endif

        switch (select(link->sock + 1, &fds, NULL, NULL, &tv)) {

        	case -1:

            if (errno == EINTR) {
				link->flags |= ILINK_FLAG_INTR;
                return e13_error(E13_CONTINUE);
            } else {
                return e13_error(E13_SYSE);
            }

            break;//-1

            case 0:
            	_deb_io2("timed out");
            	return e13_error(E13_TIMEOUT);
				break;

            default:
				break;
        }

    }//link->flags & ILINK_FLAG_TIMEOUT


    if(has_timeout){

        if (FD_ISSET(link->sock, &fds)) {
				_deb_io2("timeout mode: recieving %i bytes", len);
            n = recv(link->sock, buf, len, 0);
#ifdef WIN32
            if (gettimeofday(&tv2, NULL) == -1) {
                return e13_error(E13_SYSE);
            }
            remain1 = _ilink_tv2milli(tv1);
            remain2 = _ilink_tv2milli(tv2);
            _deb_io2("remain 1 = %i, remain 2 = %i", remain1, remain2);
            _deb_io2("remain 1 = %i, remain 2 = %i", remain1, remain2);
            assert(remain2 >= remain1);
            _ilink_milli2tv(remain2 - remain1, &tv);
#endif
        } else {
            return e13_error(E13_TIMEOUT);
        }

        if(has_timeout == 1){

            ios->ctl.recv_time = _ilink_tv2milli(tv)/1000;

        } else if(has_timeout == 2){

            memcpy(&link->recv_remain_tv, &tv, sizeof(struct timeval));

        }

    } else {

		_deb_io2("no timeout mode: recieving %i bytes", len);
        n = recv(link->sock, buf, len, 0);

    }

    //n = recv(link->sock, buf, len, 0);

    if ( n <= 0 ) {

        if(errno == EINTR) {_deb_io2("INTERRUPTED"); return E13_CONTINUE;}

        _deb_io2("DC, n = %i", n);
        perror("recv()");
        return e13_error(E13_DC);

    }

    _deb_io2("got %i bytes", n);

    if(newbuf){

        if((ret = ilink_pkt_hdr(link, buf, n, &hdr,
                                _ilink_flags_to_ppkt(link->flags))) != E13_OK){
            _deb_io2("ilink_pkt_hdr() returns %i", ret);
            return ret;
        }

        if(	(hdr.datalen + ILINK_PKT_HDR_SIZE > ILINK_MAX_PKT_SIZE) ||
			(ios && ios->maxpayload && hdr.datalen > ios->maxpayload)) {

            _deb_io2("E13_TOOBIG, %u + %u > %u", hdr.datalen,
					ILINK_PKT_HDR_SIZE, ILINK_PKT_HDR_SIZE);

            return e13_error(E13_TOOBIG);
        }

//        if(hdr.datalen < ILINK_PKT_HDR_SIZE + 1){
//            _deb_io2("E13_EMPTY");
//            return e13_error(E13_EMPTY);
//        }

        ibuf = (struct ilink_buf*)m13_malloc(sizeof(struct ilink_buf));
        if(!ibuf){
            return e13_error(E13_NOMEM);
        }
        ibuf->next = NULL;

        ibuf->bufsize =
                crypt13_enc_size(link->flags &
								ILINK_FLAG_ASYM?&link->asym_crypt:
									&link->sym_crypt,
                                 hdr.datalen + ILINK_PKT_HDR_SIZE);

        ibuf->buf =
                (ilink_data_t*)m13_malloc(ibuf->bufsize);

        memcpy(ibuf->buf, buf, n<ibuf->bufsize?n:ibuf->bufsize);
        ibuf->pos = n<ibuf->bufsize?n:ibuf->bufsize;

        if(!link->rcvd_list.first){
            _deb_io2("added as first");
            link->rcvd_list.first = ibuf;
            link->rcvd_list.last = ibuf;
            link->rcvd_list.n = 1UL;
        } else {
            _deb_io2("added as last");
            link->rcvd_list.last->next = ibuf;
            link->rcvd_list.last = link->rcvd_list.last->next;
            link->rcvd_list.n++;
        }

        _deb_io2("got %u bytes %s", ibuf->pos, ibuf->buf + ILINK_PKT_HDR_SIZE);

    } else {

        /****   TODO: WARNING!!! USING ibuf->bufsize before it sets!!!   ****/
        link->rcvd_list.last->pos += n<ibuf->bufsize?n:ibuf->bufsize;

    }

    if(link->rcvd_list.last->pos == link->rcvd_list.last->bufsize) {
        _deb_io2("DONE");
        if(link->rcvd_callback){
            link->rcvd_callback(link->rcvd_callback_ctx, link->rcvd_list.last->buf, link->rcvd_list.last->bufsize);
        }

//        if(link->rcvd_list.last == link->rcvd_list.first){
//            link->rcvd_list.n = 0UL;
//            m13_free(link->rcvd_list.first->buf);
//            m13_free(link->rcvd_list.first);
//            link->rcvd_list.first = NULL;
//        } else {

//            prev = link->rcvd_list.first;
//            for(ibuf = link->rcvd_list.first; ibuf->next; ibuf = ibuf->next){

//                prev = ibuf;

//            }

//            link->rcvd_list.last = prev;
//            link->rcvd_list.n--;

//            m13_free(ibuf->buf);
//            m13_free(ibuf);

//        }

//        _deb_io2("DONE");
        return E13_DONE;
    }

    _deb_io2("OK");

    return E13_OK;

}

error13_t ilink_recv(   struct infolink* link,
                        ilink_data_t* data,
                        ilink_datalen_t* len,
                        struct ilink_io* ios){

	error13_t ret;

    switch(link->conf.transport){

    case ILINK_TRANSPORT_TCP:
        _deb_io2("TCP");
        if(data) {
			ret = _ilink_recv_stream(link, data, len, ios);
		}
        else {
			ret = _ilink_recv_pkt(link, ios);
		}
        break;

    default:
        _deb_io2("OTHER");
        ret = e13_error(E13_IMPLEMENT);

        break;

    }

//    _deb_io2("MISSED?");

    return ret == E13_DONE?E13_OK:ret;

}

error13_t _ilink_send_stream(   struct infolink* link,
                                ilink_data_t* data,
                                ilink_datalen_t* len,
                                struct ilink_io* ios){

    //_wdeb(L"Code is out-of-order, see recv_pkt_all()");

    int n;

    int has_timeout = 0;

    fd_set fds;
#ifdef WIN32
    struct timeval tv, tv1, tv2;
    uint32_t remain1, remain2;
#endif

    _deb_io2("send raw mode");

    if(ios && ios->ctl.send_time){
        has_timeout = 1;
        _ilink_second2tv(ios->ctl.send_time, &tv);
    }

    if(
            !has_timeout &&
            (
                link->flags & ILINK_FLAG_TIMEOUT &&
                !_ilink_is_zerotv(link->send_tv)
                )

            ){

        has_timeout = 2;
        if(!_ilink_is_zerotv(link->send_remain_tv))
            memcpy(&tv, &link->send_remain_tv, sizeof(struct timeval));
        else
            memcpy(&tv, &link->send_tv, sizeof(struct timeval));

    }

    if(has_timeout){

        FD_ZERO(&fds);
        FD_SET(link->sock, &fds);

#ifdef WIN32
        if (gettimeofday(&tv1, NULL) == -1) {
            return e13_error(E13_SYSE);
        }
#endif

        switch (select(link->sock + 1, NULL, &fds, NULL, &tv)) {
        	case -1:
            if (errno == EINTR) {
                return e13_error(E13_CONTINUE);
            } else {
                return e13_error(E13_SYSE);
            }
            break;

            case 0:
				return e13_error(E13_TIMEOUT);
				break;

			default:
				break;
        }

    }//link->flags & ILINK_FLAG_TIMEOUT


    if(has_timeout){

        if (FD_ISSET(link->sock, &fds)) {
            n = send(link->sock, data, *len, 0);
#ifdef WIN32
            if (gettimeofday(&tv2, NULL) == -1) {
                return e13_error(E13_SYSE);
            }
            remain1 = _ilink_tv2milli(tv1);
            remain2 = _ilink_tv2milli(tv2);
            _deb_io2("remain 1 = %i, remain 2 = %i", remain1, remain2); assert(remain2 >= remain1);
            _ilink_milli2tv(remain2 - remain1, &tv);
#endif
        } else {
            return e13_error(E13_TIMEOUT);
        }

        if(has_timeout == 1){

            ios->ctl.send_time = _ilink_tv2milli(tv)/1000;

        } else if(has_timeout == 2){

            memcpy(&link->send_remain_tv, &tv, sizeof(struct timeval));

        }

    } else {

        n = send(link->sock, data, *len, 0);

    }

    //n = send(link->sock, data, *len, 0);

    if ((n == -1) || (n == 0)) {

        if(errno == EINTR) return E13_CONTINUE;

        return e13_error(E13_DC);

    }

    *len = n;

    if(link->sent_callback){
        link->sent_callback(link->sent_callback_ctx, data, *len);
    }

    return E13_OK;

}

error13_t _ilink_send_pkt(  struct infolink* link,
                            struct ilink_io* ios){

    //_wdeb(L"Code is out-of-order, see recv_pkt_all()");

    int n;
    struct ilink_buf* del;

    int has_timeout = 0;

    fd_set fds;
#ifdef WIN32
    struct timeval tv, tv1, tv2;
    uint32_t remain1, remain2;
#endif

    if(!link->send_list.first){
        return e13_error(E13_EMPTY);
    }

    _deb_io2("sending (sendpktsize = %u - pktsentsize = %u)=%u bytes (%s)",
             link->send_list.first->bufsize,
             link->send_list.first->pos,
             link->send_list.first->bufsize - link->send_list.first->pos,
             link->send_list.first->buf + link->send_list.first->pos);

    if(ios && ios->ctl.send_time){
        has_timeout = 1;
        _ilink_second2tv(ios->ctl.send_time, &tv);
    }

    if(
            !has_timeout &&
            (
                link->flags & ILINK_FLAG_TIMEOUT &&
                !_ilink_is_zerotv(link->send_tv)
                )

            ){

        has_timeout = 2;
        if(!_ilink_is_zerotv(link->send_remain_tv))
            memcpy(&tv, &link->send_remain_tv, sizeof(struct timeval));
        else
            memcpy(&tv, &link->send_tv, sizeof(struct timeval));

    }

    if(has_timeout){

        FD_ZERO(&fds);
        FD_SET(link->sock, &fds);

#ifdef WIN32
        if (gettimeofday(&tv1, NULL) == -1) {
            return e13_error(E13_SYSE);
        }
#endif

        switch (select(link->sock + 1, NULL, &fds, NULL, &tv)) {

        	case -1:
            if (errno == EINTR) {
                return e13_error(E13_CONTINUE);
            } else {
                return e13_error(E13_SYSE);
            }
            break;//-1

            case 0:
            	_deb_io2("timed out");
            	return e13_error(E13_TIMEOUT);
				break;

			default:
				break;
        }

    }//link->flags & ILINK_FLAG_TIMEOUT


    if(has_timeout){

        if (FD_ISSET(link->sock, &fds)) {

			_deb_io2("timeout mode: sending %i bytes pkt [bufsize = %i, pos = %i]",
						link->send_list.first->bufsize - link->send_list.first->pos,
						link->send_list.first->bufsize,
						link->send_list.first->pos);
            n = send(link->sock, link->send_list.first->buf+link->send_list.first->pos,
                     link->send_list.first->bufsize - link->send_list.first->pos, 0);
			_deb_io2("n = %i", n);
#ifdef WIN32
            if (gettimeofday(&tv2, NULL) == -1) {
                return e13_error(E13_SYSE);
            }
            remain1 = _ilink_tv2milli(tv1);
            remain2 = _ilink_tv2milli(tv2);
            _deb_io2("remain 1 = %i, remain 2 = %i", remain1, remain2); assert(remain2 >= remain1);
            _ilink_milli2tv(remain2 - remain1, &tv);
#endif
        } else {
            return e13_error(E13_TIMEOUT);
        }

        if(has_timeout == 1){

            ios->ctl.send_time = _ilink_tv2milli(tv)/1000;

        } else if(has_timeout == 2){

            memcpy(&link->send_remain_tv, &tv, sizeof(struct timeval));

        }

    } else {

		_deb_io2("no timeout mode: sending %i bytes pkt [bufsize = %i, pos = %i]",
				link->send_list.first->bufsize - link->send_list.first->pos,
				link->send_list.first->bufsize,
				link->send_list.first->pos);

        n = send(link->sock, link->send_list.first->buf+link->send_list.first->pos,
                 link->send_list.first->bufsize - link->send_list.first->pos, 0);
                 _deb_io2("n = %i, errno = %i", n, errno);

    }

//    n = send(link->sock, link->send_list.first->buf+link->send_list.first->pos,
//             link->send_list.first->bufsize - link->send_list.first->pos, 0);

    if ((n == -1) || (n == 0)) {

        if(errno == EINTR) {_deb_io2("INTERRUPTED"); return E13_CONTINUE;}

        _deb_io2("DC n = %i", n);
        perror("send()");
        return e13_error(E13_DC);

    }

    link->send_list.first->pos += n;

    if(link->send_list.first->pos == link->send_list.first->bufsize){
        if(link->sent_callback){

            link->sent_callback(link->sent_callback_ctx,
                                link->send_list.first->buf,
                                link->send_list.first->bufsize);

        }

        del = link->send_list.first;

        link->send_list.first = link->send_list.first->next;
        link->send_list.n--;

        m13_free(del->buf);
        m13_free(del);

        if(link->send_list.first) return E13_NEXT;

        return E13_DONE;
    }

    return E13_OK;

}

error13_t ilink_send(   struct infolink* link,
                        ilink_data_t* data,
                        ilink_datalen_t* len,
                        struct ilink_io* ios){

	error13_t ret;

    switch(link->conf.transport){

    case ILINK_TRANSPORT_TCP:
        if(data) {
			_deb_io2("send raw mode");
			ret = _ilink_send_stream(link, data, len, ios);
		}
        else {
			_deb_io2("send pkt mode");
			ret = _ilink_send_pkt(link, ios);
		}
        break;

    default:
    	ret = e13_error(E13_IMPLEMENT);
        break;

    }

    return ret == E13_DONE?E13_OK:ret;

}

error13_t ilink_reset(struct infolink* link, ilink_flag_t flags){

    struct ilink_buf* del;

    if(flags & ILINK_FLAG_SEND){
        while(link->send_list.first){
            del = link->send_list.first;
            link->send_list.first = link->send_list.first->next;
            if(del->buf) m13_free(del->buf);
            m13_free(del);
        }
        link->send_list.n = 0;

        if(flags & ILINK_FLAG_TIMEOUT){
            _ilink_zerotv(link->send_remain_tv);
        }

    }

    if(flags & ILINK_FLAG_RECV){
        while(link->rcvd_list.first){
            del = link->rcvd_list.first;
            link->rcvd_list.first = link->rcvd_list.first->next;
            if(del->buf) m13_free(del->buf);
            m13_free(del);
        }
        link->rcvd_list.n = 0;

        if(flags & ILINK_FLAG_TIMEOUT){
            _ilink_zerotv(link->recv_remain_tv);
        }

    }

    return E13_OK;

}

/*

error13_t _ilink_send_all_stream(ilink_sock_t fd, uchar * buf, uint32_t * len, struct timeval *tv)
{
    uint32_t total;
    uint32_t bytesleft = *len;
    int n;
    total = 0;
    fd_set writefd;
#ifndef NDEBUG
    int parts = 0;
#endif
#ifdef WIN32
    struct timeval tv1, tv2;
#endif

    _deb_io1(L"called, conn->sockfd = %i", fd);
    _deb_io1(L"len is %i", (int)*len);

    if (!tv) {
        while (total < *len) {
            n = send(fd, buf + total, bytesleft, 0);
            if (n == -1) {
                break;
            }
            total += n;
            bytesleft -= n;
        }
    } else {
        _deb_io1(L"for timed-out write");

        FD_ZERO(&writefd);
        FD_SET(fd, &writefd);
 retryselect:
        while (total < *len) {
            _deb_io1(L"timeout sec = %u, usec = %u\n", (uint16_t) tv->tv_sec, (uint16_t) tv->tv_usec);

            //READ TIME HERE
#ifdef WIN32
            if (gettimeofday(&tv1, NULL) == -1) {
                return -JE_GETTIME;
            }
#endif

            if (select(fd + 1, NULL, &writefd, NULL, tv) == -1) {
                if (errno == EINTR)
                    goto retryselect;
                else {
                    return e13_error(E13_SYSE);
                }
            }
            if (FD_ISSET(fd, &writefd)) {
                n = send(fd, buf + total, bytesleft, 0);
                if ((n == -1) || (n == 0))
                    break;

                _wdeb4(L"send() returns %i", n);
#ifndef NDEBUG
                parts++;
#endif
                total += n;
                bytesleft -= n;
                if (bytesleft == 0)
                    break;

                //READ TIME HERE, SEE IF THERE'S ANY TIME REMAINED
                //CONTINUE WITH REMAINING TIME
#ifdef WIN32
                if (gettimeofday(&tv2, NULL) == -1) {
                    return e13_error(E13_SYSE);
                }
                tv->tv_sec = tv2.tv_sec - tv1.tv_sec;
                tv->tv_usec = tv2.tv_usec - tv1.tv_usec;	//TODO:bad imp.
#endif
                if ((tv->tv_sec == 0) && (tv->tv_usec == 0))
                    break;
                else
                    goto retryselect;
            } else {	//timed-out
                _deb_io1(L"timed out");
                return e13_error(E13_TIMEOUT);
            }
        }
    }

    *len = total;

    _wdeb2(L"sent %i bytes in %i parts", (int)total, parts);

    return bytesleft == 0 ? E13_OK : e13_error(E13_DC);
}

error13_t _ilink_send_all_dgram(ilink_sock_t fd, uchar * buf, uint32_t * len, struct timeval *tv){
    return e13_error(E13_IMPLEMENT);
}

error13_t ilink_send_all_buf(struct infolink* link, uchar * buf, uint32_t * len, struct timeval *tv){

    switch(link->link_def.tl){

    case ILINK_TL_TCP:
        return _ilink_send_all_stream(link->sock, buf, len, tv);
        break;

        //TODO: TEMP IMP

    default:
        break;

    }

    return _ilink_send_all_dgram(link->sock, buf, len, tv);

}

error13_t _ilink_recv_all_stream(ilink_sock_t fd, uchar * buf, uint32_t * len, struct timeval *tv)
{

    //_wdeb(L"Code is out-of-order, see recv_pkt_all()");

    uint32_t total;
    uint32_t bytesleft = *len;
    int n;
    total = 0;
    fd_set readfd;
    _ilink_blockmode_t imode;
#ifdef WIN32
    struct timeval tv1, tv2;
#endif

    _deb_io1(L"called, conn->sockfd = %i", fd);
    _deb_io1(L"buf size = %i", (int)*len);
    if (!tv) {

        _deb_io1(L"for blocking-once read");

        n = recv(fd, buf, *len, 0);
        if ((n == -1) || (n == 0)) {
            goto end;
        }
        total = n;
        bytesleft -= n;
    } else {		//timed-out

        _deb_io1(L"for timed-out read");

        FD_ZERO(&readfd);
        FD_SET(fd, &readfd);
 retryselect:
        while (total < *len) {
            _deb_io1(L"timeout sec = %u, usec = %u", (uint16_t) tv->tv_sec, (uint16_t) tv->tv_usec);

            //READ TIME HERE
#ifdef WIN32
            if (gettimeofday(&tv1, NULL) == -1) {
                return -JE_GETTIME;
            }
#endif
            if (select(fd + 1, &readfd, NULL, NULL, tv) == -1) {
                if (errno == EINTR)
                    goto retryselect;
                else {
                    return -JE_SYSE;
                }
            }
            if (FD_ISSET(fd, &readfd)) {

                imode = ILINK_MODE_NONBLOCK;
                _ilink_set_block_mode(fd, &imode);

                n = recv(fd, buf + total, bytesleft, 0);

                imode = ILINK_MODE_BLOCK;
                _ilink_set_block_mode(fd, &imode);

                if ((n == -1) || (n == 0))
                    break;
                total += n;
                bytesleft -= n;
                if (bytesleft == 0)
                    break;

                //READ TIME HERE, SEE IF THERE'S ANY TIME REMAINED
                //CONTINUE WITH REMAINED TIME
#ifdef WIN32
                if (gettimeofday(&tv2, NULL) == -1) {
                    return e13_error(E13_SYSE);
                }
                //to avoid integer overflow
                if (tv2.tv_sec > tv1.tv_sec) {
                    tv->tv_sec = tv2.tv_sec - tv1.tv_sec;
                } else
                    tv->tv_sec = 0L;
                if (tv2.tv_usec > tv1.tv_usec) {
                    tv->tv_usec =
                        tv2.tv_usec - tv1.tv_usec;
                } else
                    tv->tv_usec = 0L;
#endif
                if ((tv->tv_sec == 0) && (tv->tv_usec == 0))
                    break;
                else
                    goto retryselect;
            } else {	//timed-out
                _deb_io1(L"timed-out");
                return e13_error(E13_TIMEOUT);
            }
        }
    }
 end:
    *len = total;

    _deb_io1(L"got %i bytes, bytesleft = %i", (int)total, bytesleft);

    return bytesleft == 0 ? E13_OK : e13_error(E13_DC);
}

error13_t _ilink_recv_all_dgram(ilink_sock_t fd, uchar * buf, uint32_t * len, struct timeval *tv){
    return e13_error(E13_IMPLEMENT);
}

error13_t _ilink_recv_pkt_all_stream(ilink_sock_t fd, uchar * buf, uint32_t * len, struct timeval *tv,
                                crypt13 *crypto, struct ilink_packet* pkt)
{

    //_wdeb(L"Code is out-of-order, see recv_pkt_all()");

    uint32_t total;
    uint32_t bytesleft = *len;
    int n;
    total = 0;
    fd_set readfd;
    _ilink_blockmode_t imode;
    int got_hdr;
    error13_t ret;
#ifdef WIN32
    struct timeval tv1, tv2;
#endif

    _deb_io1(L"for timed-out read");

    FD_ZERO(&readfd);
    FD_SET(fd, &readfd);
    got_hdr = 0;
 retryselect:
    while (bytesleft) {
        _deb_io1(L"timeout sec = %u, usec = %u, bytesleft = %u", (uint16_t) tv->tv_sec, (uint16_t) tv->tv_usec, bytesleft);

        //READ TIME HERE
#ifdef WIN32
        if (gettimeofday(&tv1, NULL) == -1) {
            return -JE_GETTIME;
        }
#endif
        if (select(fd + 1, &readfd, NULL, NULL, tv) == -1) {
            if (errno == EINTR)
                goto retryselect;
            else {
                return -JE_SYSE;
            }
        }
        if (FD_ISSET(fd, &readfd)) {

            imode = ILINK_MODE_NONBLOCK;
            _ilink_set_block_mode(fd, &imode);

            n = recv(fd, buf + total, bytesleft, 0);

            imode = ILINK_MODE_BLOCK;
            _ilink_set_block_mode(fd, &imode);

            if ((n == -1) || (n == 0))
                break;

            total += n;

            _deb_io1(L"got %i bytes, total = %u", n, total);

            if (!got_hdr) {

                if((ret = crypt13_decrypt(crypto, buf, total, buf, ILINK_PKT_HDR_SIZE)) != E13_OK) return ret;

                if ((ret = _ilink_disassm_pkt_hdr(&pkt->hdr, buf)) < 0)
                    return ret;

                if ((ret = _ilink_assert_pkt(pkt, 0)) < 0)
                    return ret;

                if (pkt->hdr.fraglen > ((*len) - total))
                    return e13_error(E13_TOOBIG);

                //bytesleft = enc_data_size(pkt->hdr.fraglen);
                bytesleft = crypt13_bufsize(crypto, pkt->hdr.fraglen);

                got_hdr = 1;

                _deb_io1(L"got hdr, need %u bytes",
                       pkt->hdr.fraglen);

            } else {

                bytesleft -= n;

            }

            if (total >= bytesleft) {
                return 0;
            }
            //READ TIME HERE, SEE IF THERE'S ANY TIME REMAINED
            //CONTINUE WITH REMAINED TIME
#ifdef WIN32
            if (gettimeofday(&tv2, NULL) == -1) {
                return e13_error(E13_SYSE);
            }
            //to avoid integer overflow
            if (tv2.tv_sec > tv1.tv_sec) {
                tv->tv_sec = tv2.tv_sec - tv1.tv_sec;
            } else
                tv->tv_sec = 0L;
            if (tv2.tv_usec > tv1.tv_usec) {
                tv->tv_usec = tv2.tv_usec - tv1.tv_usec;
            } else
                tv->tv_usec = 0L;
#endif
            if ((tv->tv_sec == 0) && (tv->tv_usec == 0))
                break;
            else
                goto retryselect;
        } else {	//timed-out
            _deb_io1(L"timed-out");
            return e13_error(E13_TIMEOUT);
        }
    }

    *len = total;

    _deb_io1(L"got %i bytes, bytesleft = %i", (int)total, bytesleft);

    return bytesleft == 0 ? E13_OK : e13_error(E13_DC);
}

error13_t ilink_send(struct infolink* link,
                     struct ilink_trans* trans){

    int sysret;
    error13_t ret;

    if(!trans->flags & ILINK_TRANSF_SEND) return e13_error(E13_READONLY);

    if(!(trans->flags & ILINK_TRANSF_SENDING)){
        trans->flags |= ILINK_TRANSF_SENDING;
        trans->sent = ILINK_DATALEN_ZERO;
    }

    switch(link->link_def.tl){
    case ILINK_TL_TCP:

        sysret = send(link->sock, trans->sendbuf + trans->sent, trans->sendbufsize - trans->sent, 0);

        break;
    case ILINK_TL_UDP:

        sysret = sendto(link->sock, trans->sendbuf + trans->sent, trans->sendbufsize - trans->sent, &link->saddr, link->saddr_len);

        break;
    default:
        return e13_error(E13_IMPLEMENT);
        break;
    }

    if(sysret <= 0){

        if(errno == EAGAIN){
            ret = E13_CONTINUE;
        } else {
            if(sysret == 0) ret = e13_error(E13_DC);
            else ret = e13_error(E13_SYSE);
        }

    } else {

        trans->sent += sysret;

        if(trans->sent == trans->sendbufsize){
            trans->flags &= ~ILINK_TRANSF_SENDING;
            ret = E13_OK;
        } else {
            ret = E13_CONTINUE;
        }

    }

    return ret;

}

error13_t ilink_recv(struct infolink* link,
                     struct ilink_trans* trans){

    int sysret;
    error13_t ret;

    if(!trans->flags & ILINK_TRANSF_RECV) return e13_error(E13_WRITEONLY);

    if(!(trans->flags & ILINK_TRANSF_RECVING)){
        trans->flags |= ILINK_TRANSF_RECVING;
        trans->sent = ILINK_DATALEN_ZERO;
    }

    switch(link->link_def.tl){
    case ILINK_TL_TCP:

        sysret = recv(link->sock, trans->recvbuf + trans->rcvd, trans->recvbufsize - trans->rcvd, 0);

        break;
    case ILINK_TL_UDP:

        sysret = recvfrom(link->sock, trans->recvbuf + trans->rcvd, trans->recvbufsize - trans->rcvd, &link->saddr, link->saddr_len);

        break;
    default:
        return e13_error(E13_IMPLEMENT);
        break;
    }

    if(sysret <= 0){

        if(errno == EAGAIN){
            ret = E13_CONTINUE;
        } else {
            if(sysret == 0) ret = e13_error(E13_DC);
            else ret = e13_error(E13_SYSE);
        }

    } else {

        trans->rcvd += sysret;

        if(trans->rcvd == trans->rcvdbufsize){
            trans->flags &= ~ILINK_TRANSF_RECVING;
            ret = E13_OK;
        } else {
            ret = E13_CONTINUE;
        }

    }

    return ret;


}

*/
