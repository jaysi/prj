#ifndef MONET_POLL_H
#define MONET_POLL_H

#include "../lib13/include/lib13.h"
#include "../infolink/include/infolink.h"

struct _monet_poll_thread_arg {

    uint32_t index;
    struct monet* mn;

};

struct monet_poll {

    th13_t th;
    //th13_mutex_t mx;//use user_array mutex?
    struct ilink_poll_list polist;

};

#ifdef __cplusplus
    extern "C" {
#endif

void* _monet_poll_thread(void* arg);
error13_t _monet_add_poll_link(struct monet* mn,
                               struct infolink* link,
                               ilink_poll_flag_t flags,
                               int lock);
error13_t _monet_request_send(struct monet* mn,
                              uid13_t nuid,
                              uid13_t* uidlist,
                              ilink_data_t* data,
                              ilink_datalen_t datalen,
                              ilink_pkt_type_t type,
                              ilink_ppkt_flag_t flags,
                              int nolock);

#ifdef __cplusplus
    }
#endif

#endif // MONET_POLL_H
