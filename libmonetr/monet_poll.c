#include "monet_internal.h"

#define _deb1           _NullMsg
#define _deb2           _NullMsg
#define _deb3           _NullMsg
#define _deb_trace_pkt0 _NullMsg
#define _deb_poll	    _NullMsg
#define _deb_prepare    _NullMsg
//TODO: REPLACE UID WITH I IN THIS PIECE OF CODE FOR PERFORMANCE

enum _monet_proc_pkt_code {
    MN_PROC_PKT_EMPTY,
    MN_PROC_PKT_DC,//disconnect request
    MN_PROC_PKT_UNPOLL,
    MN_PROC_PKT_NOPERM,//the user is not permitted to request this op
    MN_PROC_PKT_POLL,//add poll link
    MN_PROC_PKT_TEST,
    MN_PROC_PKT_USER_LIST,
    MN_PROC_PKT_USER_ADD,
    MN_PROC_PKT_USER_RM,
    MN_PROC_PKT_USER_BAN,
    MN_PROC_PKT_USER_KICK,
    MN_PROC_PKT_GROUP_LIST,
    MN_PROC_PKT_GROUP_ADD,
    MN_PROC_PKT_GROUP_RM,
    MN_PROC_PKT_GROUP_JOIN,
    MN_PROC_PKT_GROUP_LEAVE,
    MN_PROC_PKT_MOD_LIST,
    MN_PROC_PKT_INVAL
};

struct _mn_poll_add_s{
    struct infolink* link;
    ilink_poll_flag_t flags;
};

error13_t _monet_proc_pkt(struct monet* mn,
                          struct infolink* ilink,
                          enum _monet_proc_pkt_code* proc_code){

    ilink_data_t* data = NULL;
    ilink_datalen_t datalen;
    ilink_pkt_type_t type;
    error13_t ret;
    //TODO: TEMPORARY, REMOVE LATER
    //user_array is already locked??
    uid13_t uid[mn->user_array.nalloc];
    uid13_t i = 0, j;
    struct monet_fifo_entry* fentry;

    _deb3("called for sock %i.", ilink->sock);

    if((ret = ilink_explode_pkt(ilink, &data, &datalen, &type, 0)) != E13_OK){
        _deb3("%s", e13_codemsg(ret));
        return ret;
    }

    _deb3("hdr.type = %u, data = %s, datalen = %u", type, data, datalen);

    switch(type){

    case MN_PKT_DC:
        _deb3("dc");
        *proc_code = MN_PROC_PKT_DC;
        break;

    case MN_PKT_UNPOLL:

        //allowed from pipe only
        if(ilink->sock != mn->pipelink_recv->sock){ret = e13_error(E13_PERM); *proc_code = MN_PROC_PKT_INVAL; break;}

        _deb3("unpoll");
        *proc_code = MN_PROC_PKT_UNPOLL;
        ret = E13_CONTINUE;//continue processing the packet
        break;

    case MN_PKT_POLL:

        //allowed from pipe only
        if(ilink->sock != mn->pipelink_recv->sock){ret = e13_error(E13_PERM); *proc_code = MN_PROC_PKT_INVAL; break;}

        _deb3("poll");
        *proc_code = MN_PROC_PKT_POLL;
        ret = E13_CONTINUE;//add poll link
        break;

    case ILINK_PKT_TYPE_TEST:
    	_deb_trace_pkt0("TEST PKT");
        *proc_code = MN_PROC_PKT_TEST;
        //TODO: TEMPORARY, REMOVE LATER
        //already locked?
        //th13_mutex_lock(mn->user_array.mx);
        for(j = 0; j < mn->user_array.nalloc; j++){
            if(mn->user_array.array[j].user){
                uid[i++] = mn->user_array.array[j].user->uid;
            }
        }
        //th13_mutex_unlock(mn->user_array.mx);
        _monet_request_send(mn, i, uid, data, datalen, type, 0, 1);
        _deb_trace_pkt0("DATA: %s", data);
        _deb3("test data: %s", data);
        break;

        //session packets
    default:
        ret = e13_error(E13_IMPLEMENT);
        _deb3("not implemented type %u", type);

        fentry = (struct monet_fifo_entry*)m13_malloc(sizeof(struct monet_fifo_entry));
        if(!fentry) {ret = e13_error(E13_NOMEM); goto end;}
        fentry->flags = MONET_FIFO_ENTRY_FLAG_INIT;
        fentry->reqid = 0UL;//TODO: TEMPORARY?? ever need a reqid?
        fentry->type = MN_FIFO_ENT_REQUEST_PKT;
        fentry->owner = ((struct monet_user*)ilink->ext_ctx)->uid;
        fentry->sess_index = 0UL;
        fentry->pkt_type = type;
        fentry->len = datalen;
        fentry->ptr = data;
        fentry->next = NULL;

        th13_mutex_lock(&mn->req_fifo.mx);

        if(!mn->req_fifo.n){
            mn->req_fifo.first = fentry;
            mn->req_fifo.last = fentry;
            mn->req_fifo.n = 1UL;
        } else {
            mn->req_fifo.last->next = fentry;
            mn->req_fifo.last = fentry;
            mn->req_fifo.n++;
        }

        th13_sem_post(&mn->wait_sem);
        th13_mutex_unlock(&mn->req_fifo.mx);

        break;

    }

    //ilink_reset(link, ILINK_FLAG_RECV);

    if(e13_iserror(ret) && data) m13_free(data);

    return ret;

}

void* _monet_poll_thread(void* arg){

    struct _monet_poll_thread_arg* args = arg;
    struct infolink** ready, *entry, *tmplink;
    struct monet* mn = args->mn;
    struct monet_poll* pol = &(mn->poll);
    struct ilink_poll_list* list = &(pol->polist);
    uint32_t npolled, i;
    struct ilink_ctl_s ctl_nonblock, ctl_block;
    enum _monet_proc_pkt_code proc_code;
    error13_t ret;
    struct _mn_poll_add_s* s;

    ctl_nonblock.blocking = ILINK_MODE_NONBLOCK;
    ctl_block.blocking = ILINK_MODE_BLOCK;

    _deb1("index = %u, list->n = %u", args->index, list->n);

    th13_sem_post(mn->wait_sem);

    while(1) {

        list->tv.tv_sec = 0;
        list->tv.tv_usec = 0;

        _deb_poll("polling... rd_count = %i", list->readfds.fd_count);

        switch( (ret = ilink_poll(list, &npolled, &ready)) ) {

        case E13_OK://do something

            _deb_poll("OK, npolled = %u", npolled);

            for(i = 0; i < npolled; i++) {

                entry = *ready + i;

                _deb_poll("entry->sock = %i", entry->sock);

                if(entry->pollflags & ILINK_POLL_RD_READY){

                    ilink_ctl(entry, &ctl_nonblock, ILINK_CTL_BLOCK_MODE);

                    _deb_poll("poll RD_READY, rd_count = %i", list->readfds.fd_count);

                    _deb_trace_pkt0("RECIEVING");

                    switch( (ret = ilink_recv(entry, NULL, 0, NULL)) ){
                        case E13_OK:
                        _deb_poll("recv OK, polling... rd_count = %i", list->readfds.fd_count);
//                        break;
                        case E13_DONE://packet fully recieved
						_deb_trace_pkt0("RECV DONE, PROCESSING");
                        _deb_poll("recv DONE, process packet, rd_count = %i", list->readfds.fd_count);
                        switch((ret=_monet_proc_pkt(args->mn, entry, &proc_code)
                                )){
                            case E13_CONTINUE:
                                _deb_poll("process CONTINUE, rd_count = %i", list->readfds.fd_count);
                                //poll already locked, just add to list
                                if(proc_code == MN_PROC_PKT_POLL){
                                    s = mn->pipelink_send.tmp_ctx;
                                    _deb_poll("process PKT_POLL %i", s->link->sock);
                                    for(tmplink = s->link;
                                        tmplink;
                                        tmplink = tmplink->popnext) {

                                        ilink_poll_add(list,
                                                       tmplink,
                                                       s->flags);

                                    }

                                    m13_free(s);
                                }
                                if(proc_code == MN_PROC_PKT_UNPOLL){
                                    s = mn->pipelink_send.tmp_ctx;
                                    _deb_poll("process PKT_UNPOLL %i", s->link->sock);

                                    for(tmplink = s->link;
                                        tmplink;
                                        tmplink = tmplink->popnext) {

                                        ilink_poll_rm(list,
                                                      tmplink,
                                                      s->flags);

                                    }

                                    m13_free(s);
                                }
                                _deb_poll("process CONTINUE ends, rd_count = %i", list->readfds.fd_count);
                                break;

                            case E13_OK:
                                _deb_poll("process OK, rd_count = %i", list->readfds.fd_count);
                                break;

                            case e13_error(E13_PERM):
                                _deb_poll("permission denied");
                                break;

                            default:
                                _deb_poll("process default! rd_count = %i, _monet_proc_pkt = %i", list->readfds.fd_count, ret);
                                break;
                            }
                        break;
                        case E13_CONTINUE:
                            _deb_poll("recv CONTINUE, interrupted");
                        break;
                        case e13_error(E13_DC):
                            _deb_poll("recv DC, disconnected %i", entry->sock);
                            ilink_poll_rm(list, entry, ILINK_POLL_ALL|ILINK_POLL_NOLOCK);
                        break;
                        default:
                            _deb_poll("recv other case! ilink_recv = %i", ret);
                        break;
                    }

                    _deb_poll("recv done, set link to BLOCK MODE");
                    ilink_ctl(entry, &ctl_block, ILINK_CTL_BLOCK_MODE);

                    entry->pollflags &= ~ILINK_POLL_RD_READY;

                    _deb_poll("poll RD_READY ends.");

                }//ILINK_POLL_RD_READY

                if(entry->pollflags & ILINK_POLL_WR_READY){

                    _deb_poll("poll WR_READY!");

                    ilink_ctl(entry, &ctl_nonblock, ILINK_CTL_BLOCK_MODE);

                    //send
                    switch(ret = ilink_send(entry, NULL, NULL, NULL)){
                        case E13_CONTINUE:
                            _deb_poll("send CONTINUE");
                        case E13_NEXT://there is packets inside
                            _deb_poll("send NEXT, may happen after CONTINUE, continue polling");
                        //continue polling
                        break;

                        case E13_OK:
                            _deb_poll("send OK");
                        case E13_DONE:
                            _deb_poll("send DONE, may happen after OK, removing poll request");
                            ilink_poll_rm(list, entry, ILINK_POLL_WR|ILINK_POLL_NOLOCK);
                        break;

                        case e13_error(E13_DC):
                            _deb_poll("send DC, disconnected");
                            ilink_poll_rm(list, entry, ILINK_POLL_ALL|ILINK_POLL_NOLOCK);
                        break;

                        default:
                            _deb_poll("send default, ret = %i", ret);
                        break;


                    }

                    _deb_poll("send done, set link to BLOCK MODE");

                    ilink_ctl(entry, &ctl_block, ILINK_CTL_BLOCK_MODE);

                    entry->pollflags &= ~ILINK_POLL_WR_READY;

                    _deb_poll("poll WR_READY ends.");

                }//ILINK_POLL_WR_READY

                if(entry->pollflags & ILINK_POLL_EX_READY){

                    _deb_poll("poll EX_READY exception, donno what to do!");

                    entry->pollflags &= ~ILINK_POLL_EX_READY;

                }//ILINK_POLL_EX_READY

            }

            if(ready) m13_free(ready);
            //do not forget to release poll list lock at the end.
            _deb_poll("poll OK ends, unlocking");
            //TODO: where did i lock it???
            ilink_poll_unlock(list);

        break;//E13_OK

        case E13_CONTINUE:
            _deb_poll("poll CONTINUE, interrupted");
        break;//E13_CONTINUE

        case E13_TIMEOUT:
            _deb_poll("poll TIMEOUT should not happen!");
        break;//E13_TIMEOUT

        default:
            _deb_poll("poll default, ilink_poll ret = %i", ret);
            return NULL;
        break;

        }
    }

}

error13_t _monet_add_poll_link(struct monet* mn,
                               struct infolink* link,
                               ilink_poll_flag_t flags,
                               int lock){

    //TODO: write code
    error13_t ret;
    struct _mn_poll_add_s* s = (struct _mn_poll_add_s*)m13_malloc(sizeof(struct _mn_poll_add_s));
    s->flags = flags | ILINK_POLL_NOLOCK;

    if(lock) th13_mutex_lock(&mn->user_array.mx);

    s->link = link;
    s->link->popnext = NULL;

    if((ret = ilink_prepare_pkt(&mn->pipelink_send, NULL, 0, MN_PKT_POLL, 0)) != E13_OK){
		goto end;
    }

    mn->pipelink_send.tmp_ctx = s;
    ret = ilink_send(&mn->pipelink_send, NULL, NULL, NULL);

    _deb_poll("added poll link %i", link->sock);

end:

    if(lock) th13_mutex_unlock(&mn->user_array.mx);

    return ret;

}

error13_t _monet_request_send(struct monet* mn,
                              uid13_t nuid,
                              uid13_t* uidlist,
                              ilink_data_t* data,
                              ilink_datalen_t datalen,
                              ilink_pkt_type_t type,
                              ilink_ppkt_flag_t flags,
                              int nolock) {

    error13_t ret;
    uid13_t i, j;
    struct _mn_poll_add_s* s = (struct _mn_poll_add_s*)m13_malloc(sizeof(struct _mn_poll_add_s));
    s->flags = flags|ILINK_POLL_NOLOCK|ILINK_POLL_WR;
    s->link = NULL;
    struct infolink* last;

    if(!nolock) th13_mutex_lock(&mn->user_array.mx);

    _deb_prepare("nalloc: %u, nuid: %u", mn->user_array.nalloc, nuid);

    i = 0;
        for(j = 0; j < mn->user_array.nalloc; j++){
            if(     mn->user_array.array[j].user &&
                    mn->user_array.array[j].user->uid == uidlist[i] &&
                    !(mn->user_array.array[j].user->link->flags & ILINK_FLAG_PIPE)//do not send to pipe
            ){
                if(!s->link){
                    s->link = mn->user_array.array[j].user->link;
                    s->link->popnext = NULL;
                    last = s->link;
                } else {
                    last->popnext = mn->user_array.array[j].user->link;
                    last = last->popnext;
                    last->popnext = NULL;
                }

                i++;
                _deb_prepare("prepare %s to send to %i", data, last->sock);
                ret = ilink_prepare_pkt(last, data, datalen, type, flags);
                if(ret != E13_OK) goto end;
                if(i == nuid) break;

            }
        }

    ilink_prepare_pkt(&mn->pipelink_send, NULL, 0, MN_PKT_POLL, 0);
    mn->pipelink_send.tmp_ctx = s;
    ilink_send(&mn->pipelink_send, NULL, NULL, NULL);

end:
    if(!nolock) th13_mutex_unlock(&mn->user_array.mx);

    return ret;

}
