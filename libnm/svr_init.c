#include "include/nm_internal.h"

error13_t nm_init(struct netmetre* nm, struct nm_conf* conf) {

    struct ilink_conf linkconf;

    if(!e13_init(&nm->last_err, NM_CONF_WARN_MAX, NM_CONF_ERR_STR_MAX, LIB13_NETMETRE)) {
        return e13_error(E13_INTERNAL);
    }

    nm->conf = (struct nm_conf*)malloc(sizeof(struct nm_conf));
    if(!nm->conf) return e13_ierror(&nm->last_err, E13_NOMEM, NM_MSG_NOMEM);

    if(!conf){
        memcpy(nm->conf, conf, sizeof(struct nm_conf));
    } else {
        memcpy(nm->conf, &nm_conf_def, sizeof(struct nm_conf));
    }

    switch(nm->conf.role){
    case NM_ROLE_SERVER:

        //link config
        linkconf.type = ILINK_TYPE_LISTEN;
        linkconf.port = nm->conf.port;
        linkconf.asym_alg = CRYPT13_ALG_NONE;
        linkconf.sym_alg = CRYPT13_ALG_NONE;
        linkconf.handshake = ILINK_HANDSHAKE_NONE;
        linkconf.datalink = ILINK_DATALINK_IP;
        linkconf.transport = ILINK_TRANSPORT_TCP;
        linkconf.time_login = NM_CONF_TIMEOUT_LOGIN;
        linkconf.time_trans = NM_CONF_TIMEOUT_TRANS;

        ilink_init(&nm->mainlink, &linkconf);
        break;

    case NM_ROLE_CLIENT:

        //link config
        linkconf.type = ILINK_TYPE_CLIENT;
        linkconf.port = nm->conf.port;
        linkconf.asym_alg = CRYPT13_ALG_NONE;
        linkconf.sym_alg = CRYPT13_ALG_NONE;
        linkconf.handshake = ILINK_HANDSHAKE_NONE;
        linkconf.datalink = ILINK_DATALINK_IP;
        linkconf.transport = ILINK_TRANSPORT_TCP;
        linkconf.time_login = NM_CONF_TIMEOUT_LOGIN;
        linkconf.time_trans = NM_CONF_TIMEOUT_TRANS;        


        break;

    default:
        return e13_uerror(&nm->last_err, E13_MISUSE, NM_MSG_BADROLE);
        break;
    }

    return E13_OK;

}

error13_t nm_reinit(struct netmetre* nm, struct nm_conf* conf){

}

error13_t nm_destroy(struct netmetre* nm){

}
