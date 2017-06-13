#ifndef ACC13_H
#define ACC13_H

#include "type13.h"
#include "hash13.h"

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

#define ACC_USER_SYSTEM  "system"
#define ACC_USER_MANAGER "manager"
#define ACC_USER_DEBUG   "debug"
#define ACC_USER_THIS    "this"

#define ACC_GRP_STT_ACTIVE  10
#define ACC_GRP_STT_INACTIVE 20
#define ACC_GRP_STT_REMOVED 30

#define ACC_USR_STT_LOGIN   10   //user logging in
#define ACC_USR_STT_IN      20   //user logged in
#define ACC_USR_STT_LOGOUT  30   //user logging out
#define ACC_USR_STT_OUT     40   //user logged out
#define ACC_USR_STT_INACTIVE 50
#define ACC_USR_STT_REMOVED 60

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

#ifdef __cplusplus
    extern "C" {
#endif

    error13_t acc_init(struct access13* ac, struct db13* db, regid_t regid);
    error13_t acc_destroy(struct access13* ac);
    error13_t acc_set_hash(struct access13* ac, size_t hashlen, void *hashfn);

    error13_t acc_group_add(struct access13* ac, char* name);
    error13_t acc_group_rm(struct access13* ac, char* name);
    error13_t acc_group_set_stat(struct access13* ac, char* name, int stt);
    error13_t acc_group_chk(struct access13* ac, char* name, struct group13 *group);
    error13_t acc_group_list(struct access13* ac, gid13_t* n, struct group13 **group);
    error13_t acc_group_list_free(struct group13* group);

    error13_t acc_user_add(struct access13* ac, char* name, char* password);
    error13_t acc_user_rm(struct access13* ac, char* name);
    error13_t acc_user_set_stat(struct access13* ac, char* name, int stt);
    error13_t acc_user_chk(struct access13* ac, char* name, struct user13 *user);
    error13_t acc_user_list(struct access13* ac, uid13_t* n, struct user13 **user);
    error13_t acc_user_list_free(struct user13* user);

    error13_t acc_user_join_group(struct access13* ac, char* username, char* group);
    error13_t acc_user_leave_group(struct access13* ac, char* username, char* group);
    error13_t acc_user_group_check(struct access13* ac, char* username, char* group);

    error13_t acc_user_login(struct access13* ac, char* username, char* password, uid13_t* uid);
    error13_t acc_user_logout(struct access13* ac, char* username, uid13_t uid);

#ifdef __cplusplus
    }
#endif

    /***        ***ACL***       ***/

//ac PERMISSIONS
typedef uint8_t acc_perm_t;
#define ACC_PERM_RD  (0x01<<0)
#define ACC_PERM_WR  (0x01<<1)
#define ACC_PERM_EX  (0x01<<2)

typedef uint8_t acc_acl_t;

struct acc_acl_entry{
    acc_acl_t type;
    gid13_t gid;
    uid13_t uid;
    struct acc_acl_entry* next;
};

#endif // ACC13_H
