/*
//WARNING: DO NOT INCLUDE THIS FILE INSIDE YOUR PROJECT
#ifndef ILINK_POLL_USE_SELECT
#include "include/ilink_intern.h"
#endif
*/

#define _deb_sel _NullMsg
#define _deb_sel2 _DebugMsg
#define _deb_rm _NullMsg
#define _getkey _NullMsg//_GetKey
#define _deb_add _NullMsg

#define ilink_poll_lock(list)    MACRO( if((list)->mx) th13_mutex_lock( (list)->mx ); )
#define _assert_fdcount _AssertMacro

error13_t ilink_poll_init(struct ilink_poll_list* list,
                          ilink_timeout_t to,
                          ilink_poll_flag_t flags,
                          th13_mutex_t* mx){

    FD_ZERO(&list->writefds);
    FD_ZERO(&list->readfds);
    FD_ZERO(&list->exfds);
    FD_ZERO(&list->writefds_bak);
    FD_ZERO(&list->readfds_bak);
    FD_ZERO(&list->exfds_bak);

    _ilink_milli2tv(to, &list->tv);

    list->fdmax = 0;
    list->nreadfds = 0;
    list->nwritefds = 0;
    list->nexfds = 0;
    list->first = 0;
    list->n = 0;
    list->flags = flags;
    list->mx = mx;

    return E13_OK;

}

error13_t ilink_poll_set(struct ilink_poll_list* list,
                         ilink_timeout_t to,
                         ilink_poll_flag_t flags){

    _ilink_milli2tv(to, &list->tv);
    list->flags = flags;

    return E13_OK;

}

/*
 * do not forget to realease lock after doing your op on this
*/
error13_t ilink_poll(struct ilink_poll_list* list,
                     uint32_t* n,
                     struct infolink*** ready){

    int ret;
    struct infolink* entry;
    struct infolink** triggered;
    ilink_timeout_t to1, to2;

#ifndef linux
    struct timeval tv1, tv2;
#endif

#ifndef linux
            if ((list->tv.tv_sec || list->tv.tv_usec) &&
                gettimeofday(&tv1, NULL) == -1) {
                return e13_error(E13_SYSE);
            }
#endif

            _assert_fdcount(list->readfds.fd_count || list->writefds.fd_count || list->exfds.fd_count);

    _deb_sel("max: %i, rd: %i, wr: %i, ex: %i, sec: %i, usec: %i, rd_fd_count: %u", list->fdmax, list->nreadfds, list->nwritefds, list->nexfds, list->tv.tv_sec,list->tv.tv_usec, list->readfds.fd_count);

    ret = select(list->fdmax + 1,
                 list->nreadfds>0?&list->readfds:NULL,
                 list->nwritefds>0?&list->writefds:NULL,
                 list->nexfds>0?&list->exfds:NULL,
                 (list->tv.tv_sec || list->tv.tv_usec)?&list->tv:NULL);

    _deb_sel("max: %i, rd: %i, wr: %i, ex: %i, sec: %i, usec: %i, rd_fd_count: %u", list->fdmax, list->nreadfds, list->nwritefds, list->nexfds, list->tv.tv_sec,list->tv.tv_usec, list->readfds.fd_count);

    if(!ret) return E13_TIMEOUT;
    if(ret < 0){
        if(errno == EINTR) return E13_CONTINUE;
#ifndef WIN32
        perror("select()");
#else
        wchar_t *s = NULL;
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL, WSAGetLastError(),
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       (LPWSTR)&s, 0, NULL);

        _deb_sel2("%S, %i", s, list->readfds.fd_array[0]);
        LocalFree(s);

#endif
        _deb_sel("ret = %i, error: %s:%i", ret, errno == EBADF?"BADF":errno==EINVAL?"INVAL":"UNKN", errno);
        return e13_error(E13_SYSE);
    }

#ifndef linux
        if((list->tv.tv_sec || list->tv.tv_usec)){

            if (gettimeofday(&tv2, NULL) == -1) {
                return e13_error(E13_SYSE);
            }

            to1 = _ilink_tv2milli(tv1);
            to2 = _ilink_tv2milli(tv2);

            _ilink_milli2tv(to2-to1, &list->tv);
        }

#endif

    //max size, may tune later
    triggered = (struct infolink**)m13_malloc(ret*sizeof(struct infolink*));
    if(!triggered){
        return e13_error(E13_NOMEM);
    }

    *n = 0;

    ilink_poll_lock(list);

    for(entry = list->first; entry; entry = entry->plistnext){

        //entry->pollflags &= ~(ILINK_POLL_ALL_READY);

        if( (entry->pollflags & ILINK_POLL_RD) &&
            !(entry->pollflags & ILINK_POLL_RD_READY) &&
            FD_ISSET(entry->sock, &list->readfds)
        ){

            entry->pollflags |= ILINK_POLL_RD_READY;
            FD_SET(entry->sock, &list->readfds);

        }

        if( (entry->pollflags & ILINK_POLL_WR) &&
            !(entry->pollflags & ILINK_POLL_WR_READY) &&
            FD_ISSET(entry->sock, &list->writefds)
        ){

            entry->pollflags |= ILINK_POLL_WR_READY;
            FD_SET(entry->sock, &list->writefds);

        }

        if( (entry->pollflags & ILINK_POLL_EX) &&
            !(entry->pollflags & ILINK_POLL_EX_READY) &&
            FD_ISSET(entry->sock, &list->exfds)
        ){

            entry->pollflags |= ILINK_POLL_EX_READY;
            FD_SET(entry->sock, &list->exfds);

        }

        if(entry->pollflags & ILINK_POLL_ALL_READY){

            _deb_sel("added sock %i as %i", entry->sock, *n);
            triggered[*n] = entry;
            (*n)++;
        }

        if((*n) == ret) break;

    }

    if(list->nreadfds) memcpy(&list->readfds, &list->readfds_bak, sizeof(struct fd_set));
    if(list->nwritefds) memcpy(&list->writefds, &list->writefds_bak, sizeof(struct fd_set));
    if(list->nexfds) memcpy(&list->exfds, &list->exfds_bak, sizeof(struct fd_set));

    //ilink_poll_unlock(list);

    *ready = triggered;

    _deb_sel("max: %i, rd: %i, wr: %i, ex: %i, sec: %i, usec: %i, rd_fd_count: %u", list->fdmax, list->nreadfds, list->nwritefds, list->nexfds, list->tv.tv_sec,list->tv.tv_usec, list->readfds.fd_count);

    return E13_OK;

}

error13_t ilink_poll_add(struct ilink_poll_list* list, struct infolink* link, uint8_t dir){

    struct infolink* ent;
    int exists = 0;

    if(!(dir & ILINK_POLL_NOLOCK)) ilink_poll_lock(list);

//    _deb_sel("max: %i, rd: %i, wr: %i, ex: %i, sec: %i, usec: %i, rd_fd_count: %u", list->fdmax, list->nreadfds, list->nwritefds, list->nexfds, list->tv.tv_sec,list->tv.tv_usec, list->readfds.fd_count);
//    _deb_sel2("added %i", link->sock);

    _ilink_inc_link_refcount(0, link);

    for(ent = list->first; ent; ent = ent->plistnext){
        if(ent->sock == link->sock){
            _deb_add("%i already exists in poll list, updating...", link->sock);
            _getkey();
            exists = 1;
            break;
        }
    }

    _deb_add("add %i", link->sock);

    if(dir & ILINK_POLL_WR) {
        if(exists){
            if(!FD_ISSET(link->sock, &list->writefds)) goto addwr;
        } else {
addwr:
            FD_SET(link->sock, &list->writefds);
            FD_SET(link->sock, &list->writefds_bak);
            list->nwritefds++;
        }
    }
    if(dir & ILINK_POLL_RD) {
        if(exists){
            if(!FD_ISSET(link->sock, &list->readfds)) goto addrd;
        } else {
addrd:
            FD_SET(link->sock, &list->readfds);
            FD_SET(link->sock, &list->readfds_bak);
            list->nreadfds++;
        }
    }
    if(dir & ILINK_POLL_EX) {
        if(exists){
            if(!FD_ISSET(link->sock, &list->exfds)) goto addrd;
        } else {
addex:
            FD_SET(link->sock, &list->exfds);
            FD_SET(link->sock, &list->exfds_bak);
            list->nexfds++;
        }
    }

    _deb_sel("max: %i, rd: %i, wr: %i, ex: %i, sec: %i, usec: %i, rd_fd_count: %u", list->fdmax, list->nreadfds, list->nwritefds, list->nexfds, list->tv.tv_sec,list->tv.tv_usec, list->readfds.fd_count);

    link->pollflags |= dir;

    if(!exists){

        if(link->sock > list->fdmax) list->fdmax = link->sock;

        link->plistnext = NULL;
        link->preadynext = NULL;

        if(!list->first){
            list->first = link;
            list->last = link;
        } else {
            list->last->plistnext = link;
            list->last = list->last->plistnext;
        }

        list->n++;

    }

    if(!(dir & ILINK_POLL_NOLOCK)) ilink_poll_unlock(list);

    _deb_sel("max: %i, rd: %i, wr: %i, ex: %i, sec: %i, usec: %i, rd_fd_count: %u", list->fdmax, list->nreadfds, list->nwritefds, list->nexfds, list->tv.tv_sec,list->tv.tv_usec, list->readfds.fd_count);

    return E13_OK;

}

error13_t ilink_poll_rm(struct ilink_poll_list* list, struct infolink* link, uint8_t dir){

    struct infolink* prev, *entry;
    int replace_fdmax;
    int dont_remove_it = 0;

    if(!(dir & ILINK_POLL_NOLOCK)) ilink_poll_lock(list);

    _assert_fdcount(list->readfds.fd_count || list->writefds.fd_count || list->exfds.fd_count);

    _deb_rm("removing %i, flags = %u, dir = %u, rd_fdcount = %i", link->sock, link->pollflags, dir, list->readfds.fd_count);

    if(link->pollflags & ILINK_POLL_WR) dont_remove_it++;
    if(link->pollflags & ILINK_POLL_RD) dont_remove_it++;
    if(link->pollflags & ILINK_POLL_EX) dont_remove_it++;

    if(dir & ILINK_POLL_WR && link->pollflags & ILINK_POLL_WR){
        _deb_rm("remove fd %i wr", link->sock);
        FD_CLR(link->sock, &list->writefds);
        FD_CLR(link->sock, &list->writefds_bak);
        list->nwritefds--;
        dont_remove_it--;
    }
    if(dir & ILINK_POLL_RD && link->pollflags & ILINK_POLL_RD){
        _deb_rm("remove fd %i rd", link->sock);
        FD_CLR(link->sock, &list->readfds);
        FD_CLR(link->sock, &list->readfds_bak);
        list->nreadfds--;
        dont_remove_it--;
    }
    if(dir & ILINK_POLL_EX && link->pollflags & ILINK_POLL_EX){
        _deb_rm("remove fd %i ex", link->sock);
        FD_CLR(link->sock, &list->exfds);
        FD_CLR(link->sock, &list->exfds_bak);
        list->nexfds--;
        dont_remove_it--;
    }

    link->pollflags &= ~dir;

    if(!dont_remove_it){

        if(link->sock == list->fdmax){
            replace_fdmax = 1;
            list->fdmax = -1;
        } else {
            replace_fdmax = 0;
        }

        prev = list->first;
        for(entry = list->first; entry; entry = entry->next){

            if(entry->sock == link->sock){
                if(list->first == entry){
                    if(list->last == entry){
                        list->last = NULL;
                    }
                    list->first = list->first->next;
                } else {
                    if(list->last == entry){
                        list->last = prev;
                    }
                    prev->next = entry->next;
                }
            } else {
                if(replace_fdmax){
                    if(list->fdmax < entry->sock) list->fdmax = entry->sock;
                }
            }
        }

    }//!dont_remove_it

    _assert_fdcount(list->readfds.fd_count || list->writefds.fd_count || list->exfds.fd_count);

    _ilink_dec_link_refcount(0, link);

    if(!(dir & ILINK_POLL_NOLOCK)) ilink_poll_unlock(list);

    return E13_OK;

}
