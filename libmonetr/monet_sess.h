#ifndef MONET_SESS_H
#define MONET_SESS_H

#include "../lib13/include/lib13.h"
#include "monet_user.h"
#include "monet_mod.h"
#include "monet_fifo.h"
#include "monet_cont.h"

#define MN_SESSFLAG_NOT_INIT    (0x0001<<0)
#define MN_SESSFLAG_PAUSE       (0x0001<<1)//store requests, do not process
#define MN_SESSFLAG_STOP        (0x0001<<2)//do not store or process requests
#define MN_SESSFLAG_DEF			(0x0000)

typedef uint32_t mn_sessid_t;
typedef uint16_t mn_sessflag_t;

struct monet_iuid {
    uint32_t i;
    uid13_t uid;
};

struct monet_session {

    //session id is the index number of the session
    mn_sessflag_t flags;
    uint32_t index;
    char* name;
    uid13_t owner_uid;
    th13_t thid;
    th13_sem_t sem;
    uint32_t niuid;
    struct io13 rdpipe, wrpipe;
    struct monet_iuid* iuid;
    struct monet_mod_list modlist;//open modlist
    struct monet_cont_list contlist;//kinda journal!
    struct monet_fifo fifo;

};

struct monet_session_array_entry {
    struct monet_session* sess;
};

//session 0 is system session

struct monet_session_array {

    th13_mutex_t mx;
    mn_sessid_t nalloc;
    mn_sessid_t npause;
    mn_sessid_t nactive;
    mn_sessid_t ndisalloc;
    struct monet_session_array_entry* array;

};

#endif // MONET_SESS_H
