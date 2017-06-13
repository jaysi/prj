#ifndef MONET_FIFO_H
#define MONET_FIFO_H

#include "../lib13/include/lib13.h"
#include "../infolink/include/infolink.h"

enum monet_fifo_entry_t {

    MN_FIFO_ENT_EMPTY,
    MN_FIFO_ENT_REQUEST_PKT,
    MN_FIFO_ENT_REPLY_PKT,
    MN_FIFO_ENT_RM_LINK,//remove ilink connection
    MN_FIFO_ENT_INVAL

};

struct monet_fifo_entry {

    uint8_t flags;
    enum monet_fifo_entry_t type;
    uid13_t owner;
    uint32_t sess_index;
    ilink_datalen_t len;
    ilink_data_t* ptr;
    struct monet_fifo_entry* next;

};

#define MN_FIFO_FLAG_BLOCK (0x01<<0)    //don't accept any new requests

struct monet_fifo {
    uint8_t flags;    
    uint32_t n;
    struct monet_fifo_entry* first, *last;
};

#endif // MONET_FIFO_H
