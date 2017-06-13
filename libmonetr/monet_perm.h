#ifndef MONET_PERM_H
#define MONET_PERM_H

#include "../lib13/include/error13.h"
#include "../infolink/include/ilink_types.h"

#ifdef __cplusplus
extern "C" {
#endif

error13_t _monet_add_perm(struct monet* mn, uid13_t uid, gid13_t gid, ilink_pkt_type_t pkttype, ilink_data_t* pktdata);
error13_t _monet_rm_perm(struct monet* mn, uid13_t uid, gid13_t gid, ilink_pkt_type_t pkttype, ilink_data_t* pktdata);
error13_t _monet_check_perm(struct monet* mn, uid13_t uid, gid13_t gid, ilink_pkt_type_t pkttype, ilink_data_t* pktdata, ilink_datalen_t datalen);

#ifdef __cplusplus
}
#endif

#endif // MONET_PERM_H

