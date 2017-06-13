#ifndef MONET_PKT_H
#define MONET_PKT_H

#include "../lib13/include/lib13.h"
#include "monet_user.h"
#include "monet_sess.h"
#include "../infolink/include/ilink_intern.h"

#define MN_PKT_START			(ILINK_PKT_TYPE_RESERVED + 1)
#define MN_PKT_LOGIN			(MN_PKT_START + 1)
#define MN_PKT_DC   			(MN_PKT_START + 2)//dc request
//?#define MN_PKT_MOD  			(MN_PKT_START + 3)//module request
#define MN_PKT_UNPOLL 			(MN_PKT_START + 4)//unpoll, pipe required
#define MN_PKT_POLL 			(MN_PKT_START + 5)//poll, pipe required
#define MN_PKT_SESS_START   	(MN_PKT_START + 6)//start
#define MN_PKT_SESS_PAUSE   	(MN_PKT_START + 7)//pause, use start to resume
#define MN_PKT_SESS_END   		(MN_PKT_START + 8)//end
#define MN_PKT_SESS_JOIN   		(MN_PKT_START + 9)//end
#define MN_PKT_SESS_DEBUG   	(MN_PKT_START + 10)//end
#define MN_PKT_SESS_LIST   		(MN_PKT_START + 11)//end
#define MN_PKT_SESS_TRANS_START (MN_PKT_START + 12)//trans start
#define MN_PKT_SESS_TRANS_ACK   (MN_PKT_START + 13)//trans
#define MN_PKT_SESS_TRANS_END   (MN_PKT_START + 14)//trans

#define MN_MAX_LOGIN_PAYLOAD    (MN_MAX_USERNAME + MN_MAX_PASSWORD + (2*sizeof(uint16_t)))

//payload structs
struct mn_pl_sess_start {
    mn_sessflag_t flags;
    uint32_t nuser;
    char* name;
    char** userlist;
};

#define _mn_pack_pl_sess_start(s, fl, nu, name, uli)

#endif // MONET_PKT_H
