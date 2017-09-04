#ifndef ACC13_H
#define ACC13_H

#include "type13.h"
#include "hash13.h"
#include "obj13.h"

#define UID13_INVAL ((uint32_t)-1)
#define GID13_INVAL ((uint32_t)-1)
#define UID13_NONE  (0x00000000)
#define GID13_NONE  (0x00000000)
#define UID13_ANY   (0xffffffff)
#define GID13_ANY   (0xffffffff)
#define REGID_INVAL ((uint32_t)-1)
#define REGID_NONE  (0x00000000)
#define REGID_ANY   (0xffffffff)

#define ACC_USER_PASS_HASH_ALG_DEF  h13_sha256
#define ACC_USER_PASS_HASH_LEN_DEF  512

#define ACC_USER_SYSTEM     "system"
#define ACC_USER_MANAGER    "manager"
#define ACC_USER_DEBUG      "debug"
#define ACC_USER_THIS       "this"
#define ACC_USER_GUEST      "guest"

#define ACC_GROUP_ALL	"allgroups"
#define ACC_USER_ALL	"allusers"

#define ACC_GRP_STT_ACTIVE  10
#define ACC_GRP_STT_INACTIVE 20
#define ACC_GRP_STT_REMOVED 30

#define ACC_USR_STT_LOGIN   10   //user logging in
#define ACC_USR_STT_IN      20   //user logged in
#define ACC_USR_STT_LOGOUT  30   //user logging out
#define ACC_USR_STT_OUT     40   //user logged out
#define ACC_USR_STT_INACTIVE 50
#define ACC_USR_STT_REMOVED 60

#define ACC_MEMS_STT_ACTIVE		10
#define ACC_MEMS_STT_INACTIVE	20


struct group13{

    char* name;
    gid13_t gid;
    int stt;

    struct group13* next;

};

struct user13{

    char* name;
    uid13_t uid;
    int stt;

    int32_t lastdate[3];
    time_t lasttime;

    uchar* passhash;

    struct user13* next;

};

struct access13{

    magic13_t magic;

    regid_t regid;

    struct user13* login;

    struct db13* db;

    size_t hashlen;

    void (*hash)(uchar * input, size_t len, uchar* digest);

};

    /***        ***ACL***       ***/

//ac PERMISSIONS
typedef uint8_t acc_perm_t;
#define ACC_PERM_RD  (0x01<<0)
#define ACC_PERM_WR  (0x01<<1)
#define ACC_PERM_EX  (0x01<<2)
#define ACC_PERM_REC_USER	0x01
#define ACC_PERM_REC_GROUP	0x02

typedef uint8_t acc_acl_t;

struct acc_acl_entry{
    acc_acl_t type;
    gid13_t gid;
    uid13_t uid;
    struct acc_acl_entry* next;
};

#ifdef __cplusplus
    extern "C" {
#endif

    error13_t acc_init(struct access13* ac, struct db13* db, regid_t regid);
    error13_t acc_destroy(struct access13* ac);
    error13_t acc_set_hash(struct access13* ac, size_t hashlen, void *hashfn);

    error13_t acc_group_add(struct access13* ac, char* name);
    error13_t acc_group_rm(struct access13* ac, char* name, gid13_t id);
    error13_t acc_group_set_stat(struct access13* ac, char* name, gid13_t id, int stt);
    error13_t acc_group_chk(struct access13* ac, char* name, gid13_t id, struct group13 *group);
    error13_t acc_group_list(struct access13* ac, gid13_t* n, struct group13 **group);
    error13_t acc_group_list_free(struct group13* group);
//    error13_t acc_gid_chk(struct access13* ac, gid13_t gid, struct group13* group);

    error13_t acc_user_add(struct access13* ac, char* name, char* password);
    error13_t acc_user_rm(struct access13* ac, char* name, uid13_t id);
    error13_t acc_user_set_stat(struct access13* ac, char* name, uid13_t id, int stt);
    error13_t acc_user_chk(struct access13* ac, char* name, uid13_t id, struct user13 *user);
    error13_t acc_user_list(struct access13* ac, uid13_t* n, struct user13 **user);
    error13_t acc_user_list_free(struct user13* user);
//    error13_t acc_uid_chk(struct access13* ac, uid13_t uid, struct user13 *user);//TODO

    error13_t acc_user_join_group(struct access13* ac, char* username, uid13_t uid, char* group, gid13_t gid);
    error13_t acc_user_leave_group(struct access13* ac, char* username, uid13_t uid, char* group, gid13_t gid);
    error13_t acc_user_group_check(struct access13* ac, char* username, uid13_t uid, char* group, gid13_t gid);
    error13_t acc_user_group_list(struct access13 *ac, char *username, uid13_t uid, struct group13** grouplist, int resolve_gid);
    error13_t acc_group_user_list(struct access13 *ac, char *groupname, gid13_t gid, struct user13** userlist, int resolve_uid);//TODO

    error13_t acc_user_login(struct access13* ac, char* username, uid13_t uid, char* password, uid13_t* uid);
    error13_t acc_user_logout(struct access13* ac, char* username, uid13_t uid);

    error13_t acc_perm_user_chk(struct access13* ac, objid13_t objid, char* name, uid13_t uid, acc_perm_t perm);
    error13_t acc_perm_user_add(struct access13* ac, objid13_t objid, char* name, uid13_t uid, acc_perm_t perm);
    error13_t acc_perm_user_rm(struct access13* ac, objid13_t objid, char* name, uid13_t uid, acc_perm_t perm);
    error13_t acc_perm_group_chk(struct access13* ac, objid13_t objid, char* name, gid13_t gid, acc_perm_t perm);
    error13_t acc_perm_group_add(struct access13* ac, objid13_t objid, char* name, gid13_t gid, acc_perm_t perm);
    error13_t acc_perm_group_rm(struct access13* ac, objid13_t objid, char* name, gid13_t gid, acc_perm_t perm);
//	error13_t acc_perm_uid_chk(struct access13* ac, objid13_t objid, uid13_t uid, acc_perm_t perm);
//    error13_t acc_perm_uid_add(struct access13* ac, objid13_t objid, uid13_t uid, acc_perm_t perm);
//    error13_t acc_perm_uid_rm(struct access13* ac, objid13_t objid, uid13_t uid, acc_perm_t perm);
//	error13_t acc_perm_gid_chk(struct access13* ac, objid13_t objid, gid13_t gid, acc_perm_t perm);
//    error13_t acc_perm_gid_add(struct access13* ac, objid13_t objid, gid13_t gid, acc_perm_t perm);
//    error13_t acc_perm_gid_rm(struct access13* ac, objid13_t objid, gid13_t gid, acc_perm_t perm);

	error13_t acc_user_access(struct access13* ac, objid13_t objid, char* name, uid13_t uid, acc_perm_t perm);

#ifdef __cplusplus
    }
#endif

#endif // ACC13_H
