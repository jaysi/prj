#ifndef MONET_USER_H
#define MONET_USER_H

#include "../lib13/include/lib13.h"

#define MN_MAX_USERNAME    	256
#define MN_MAX_PASSWORD    	1024
#define MN_MAX_PASSHASH		1024
#define MN_MAX_HOSTNAME    	256

#define MN_USER_LOGIN_TO	10//in seconds

struct monet_user {

    char* name;
    size_t i;//array index
    uid13_t uid;
//    gid13_t* gid_array;
//    struct user13* u13;
    struct infolink* link;
    struct monet_user* next;

};

struct monet_user_array_entry {
    struct monet_user* user;
};

struct monet_user_array {
    th13_mutex_t mx;
    uid13_t nalloc;
    uid13_t nactive;
    uid13_t npause;
    struct monet_user_array_entry* array;
};


/*struct monet_user_list {
    th13_mutex_t mx;
    uid13_t n;
    struct monet_user* first, *last, *curr;
};*/

#endif // MONET_USER_H
