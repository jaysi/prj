#include <unistd.h>
#include "monet_internal.h"
#include "../lib13/include/lib13.h"
/* one thread per session */

#define _deb1 _DebugMsg

struct _mn_sess_arg_s {
    struct monet* mn;
    uint32_t i;
};

void* _monet_session_thread(void* arg){

    io13_dataptr_t buf;
    io13_datalen_t size;
    struct _mn_sess_arg_s* args = arg;
    struct io13* rdpipe, *wrpipe;
    error13_t ret;

    //def_size = args->mn->conf.sess_def_pipe_read;

    //mx locked from create session call

    rdpipe = &args->mn->sess_array.array[args->i].sess->rdpipe;
    wrpipe = &args->mn->sess_array.array[args->i].sess->wrpipe;

    th13_mutex_unlock(&args->mn->sess_array.mx);

    while( 1 ) {

        if((ret = io13_read_packed(rdpipe, &buf, &size, 0)) != E13_DONE){
            //TODO: handle error
        } else {
            //TODO: process packet
            io13_reset(rdpipe);
        }

    }

    return NULL;
}

error13_t _monet_init_session(struct monet* mn,
                              uid13_t owner,
                              char* name,
                              uint32_t i,
                              int lock) {

    struct monet_session* sess;
    struct _mn_sess_arg_s sess_arg;
    error13_t ret;
    sess = (struct monet_session*)m13_malloc(sizeof(struct monet_session));
    if(!sess) {
        return e13_error(E13_NOMEM);
    }

    io13_init(&sess->rdpipe, 0);
    io13_init(&sess->wrpipe, 0);
    if((ret = io13_open_pipe(&sess->rdpipe, &sess->wrpipe)) != E13_OK){
        m13_free(sess);
        return ret;
    }

    th13_mutex_lock(&mn->sess_array.mx);

    mn->sess_array.array[i].sess = sess;

    mn->sess_array.array[i].sess->name =
            s13_malloc_strcpy(name, mn->conf.max_sessname);
    mn->sess_array.array[i].sess->index = i;
    mn->sess_array.array[i].sess->modlist.first = NULL;
    mn->sess_array.array[i].sess->modlist.n = 0UL;
    mn->sess_array.array[i].sess->contlist.first = NULL;
    mn->sess_array.array[i].sess->contlist.n = 0UL;
    mn->sess_array.array[i].sess->owner_uid = owner;
    mn->sess_array.array[i].sess->niuid = 0UL;
    mn->sess_array.array[i].sess->iuid = NULL;
    mn->sess_array.array[i].sess->fifo.first = NULL;//write fifo init
    mn->sess_array.array[i].sess->fifo.flags = 0;
    mn->sess_array.array[i].sess->flags  = 0;

    th13_sem_init(&mn->sess_array.array[i].sess->sem, 0, 0);
    sess_arg.i = i;
    sess_arg.mn = mn;
    th13_create(&mn->sess_array.array[i].sess->thid, NULL, &_monet_session_thread, &sess_arg);

    return E13_OK;
}

error13_t _monet_allocate_session(struct monet* mn){

    int i;

    mn->sess_array.array = (struct monet_session_array_entry*)m13_realloc(
                mn->sess_array.array,
                sizeof(struct monet_session_array_entry)*(mn->sess_array.nalloc +
                                                    mn->conf.sess_bulk_size)
                );

    if(!mn->sess_array.array){
        return e13_error(E13_NOMEM);
    }

    for(i = mn->sess_array.nalloc;
        i < mn->sess_array.nalloc + mn->conf.sess_bulk_size; i++){
        mn->sess_array.array[i].sess = NULL;
    }

    mn->sess_array.nalloc += mn->conf.sess_bulk_size;

    return E13_OK;

}

error13_t _monet_create_session(struct monet* mn,
                                uid13_t owner,
                                char* name){

    error13_t ret;
    int i;
#ifdef _DEBUG_MUTEX
    th13_mutexattr_t attr;
#endif

    if(!mn->sess_array.array){
        //init

#ifdef _DEBUG_MUTEX
    _deb1("DEBUG_MUTEX ON");
    TH13_MUTEXATTR_INIT_ERRORCHECK(&attr);
    th13_mutex_init(&mn->sess_array.mx, &attr);
#else
    th13_mutex_init(&mn->sess_array.mx, NULL);
#endif

        th13_mutex_lock(&mn->sess_array.mx);

        mn->sess_array.array = (struct monet_session_array_entry*)m13_malloc(
                    sizeof(struct monet_session_array_entry)*mn->conf.sess_bulk_size);
        if(!mn->sess_array.array){
            return e13_error(E13_NOMEM);
        }

        mn->sess_array.ndisalloc = 0UL;
        mn->sess_array.npause = 0UL;
        mn->sess_array.nactive = 0UL;

        mn->sess_array.nalloc = mn->conf.sess_bulk_size;

        i = 0;

        //if(!mn->sess_array.array)
    } else {

        th13_mutex_lock(&mn->sess_array.mx);

        if(mn->sess_array.nactive + mn->sess_array.npause ==
           mn->sess_array.nalloc) {
            //allocate new
            if( ( ret = _monet_allocate_session( mn ) ) != E13_OK ) return ret;
            i = mn->sess_array.nalloc - mn->conf.sess_bulk_size;
        } else {
            //use old
            for(i = 0; i < mn->sess_array.nalloc; i++){
                if(!mn->sess_array.array[i].sess){
                    break;
                }
            }
        }

    }

    _monet_init_session(mn, owner, name, i, 0);

    mn->sess_array.nactive++;

    th13_mutex_unlock(&mn->sess_array.mx);

    return E13_OK;

}

/******************************************************************************
* mostly client side requests and their server side handlers here
******************************************************************************/

error13_t _mn_handle_open_sess(){


}

error13_t mn_open_sess(struct monet* mn, char* name, mn_sessflag_t flags){



}
