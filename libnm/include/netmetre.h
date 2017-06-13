#ifndef NETMETRE_H
#define NETMETRE_H

#include "../lib13/include/lib13.h"

enum nm_role_t {
    NM_ROLE_EMPTY,
    NM_ROLE_SERVER,
    NM_ROLE_CLIENT,
    NM_ROLE_INVAL
};
#define NM_ROLE_DEF NM_ROLE_CLIENT

enum nm_stop_t{
    NM_STOP_EMPTY,
    NM_STOP_SOFT,//WAIT FOR ME!
    NM_STOP_HARD,//KILLALL
    NM_STOP_INVAL
};
#define NM_STOP_DEF NM_STOP_SOFT

#define NM_CONF_WARN_MAX 10
#define NM_CONF_ERR_STR_MAX 1024
#define NM_CONF_PORT_DEF "2580" //BAK_SWE + C= + ASCII => TODO
#define NM_CONF_TIMEOUT_LOGIN   (10*1000) //milliseconds
#define NM_CONF_TIMEOUT_TRANS   (10*1000) // like above

struct nm_conf {
    enum nm_role role;
    char* port;
} nm_conf_def = {
    NM_ROLE_DEF,
    NM_CONF_PORT_DEF
};

struct nm_prj {
    char* name;
    objid13_t objid;
    struct db13* db;
    void* ext_ptr;
    struct nm_prj* next;
};

struct nm_prj_list {
    objid13_t n;
    struct nm_prj* first, *last;
};

struct nm_sess {
    char* name;
    objid13_t objid;
    struct nm_prj_list* prj_list;
    struct nm_sess* next;
};

struct nm_sess_list {
    objid13_t n;
    struct nm_sess* first, *last;
};

struct netmetre {

    struct nm_conf conf;

    struct infolink mainlink;

    struct nm_sess_list* sess_list;

    struct e13 last_err;

};

#ifdef __cplusplus
extern "C" {
#endif

error13_t nm_init(struct netmetre* nm, struct nm_conf* conf);
error13_t nm_reinit(struct netmetre* nm, struct nm_conf* conf);
error13_t nm_destroy(struct netmetre* nm);

error13_t nm_start(struct netmetre* nm);
error13_t nm_pause(struct netmetre* nm);
error13_t nm_stop(struct netmetre* nm, enum nm_stop_t how);

#ifdef __cplusplus
}
#endif

#endif // NETMETRE_H

