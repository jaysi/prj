#include <stdio.h>
#include <errno.h>
#include "../lib13/include/lib13.h"

int accinit;

struct t13{
    struct db13 db;
    struct access13 ac;
};

static inline void t_perr(struct e13* e, error13_t er13){
    printf("]![ User: %s, Dev: %s, Code: %s(%i), Sys: %s\n", e?e13_uerrmsg(e):"", e?e13_ierrmsg(e):"", e13_codemsg(er13), er13, strerror(errno));
}

int t_acc13(struct t13* t){
    error13_t ret;
    int ch;
    char name[1024], pass[1024];
    int stt;
    struct group13 *list, *ptr;
    struct user13* ulist, *uptr;
    gid13_t id;
    struct group13 group;
    struct user13 user;

    if(accinit){
       goto choice;
    }

    ret = db_init(&t->db, DB_DRV_SQLITE);
    if(ret != E13_OK){
        printf("could not init db.\n");
        t_perr(&t->db.e, ret);
        return false_;
    }


    ret = db_open(&t->db, "", "", "", "", "test13.sqlite");
    if(ret != E13_OK){
        printf("could not open db.\n");
        t_perr(&t->db.e, ret);
        return false_;
    }

    ret = acc_init(&t->ac, &t->db, 1);
    if(ret != E13_OK){
        printf("could not init access.\n");
        t_perr(&t->db.e, ret);
        return false_;
    }
    accinit = true_;
choice:
    printf("Please select:\n\n");
    printf("1. group add\n");
    printf("2. group rm\n");
    printf("3. group chk\n");
    printf("4. group stat\n");
    printf("5. group list\n\n");

    printf("6. user add\n");
    printf("7. user rm\n");
    printf("8. user chk\n");
    printf("9. user stat\n");
    printf("10.user list\n\n");

    printf("0. exit\n");
    scanf("%i", &ch);

    switch(ch){
    case 1:
        printf("name: ");
        scanf("%s", name);
        if(acc_group_add(&t->ac, name) != E13_OK){
            printf("could not add group.\n");
            t_perr(&t->db.e, ret);
        } else {
            printf("ok\n");
        }
        break;

    case 2:
        printf("name: ");
        scanf("%s", name);
        if(acc_group_rm(&t->ac, name) != E13_OK){
            printf("could not remove group.\n");
            t_perr(&t->db.e, ret);
        } else {
            printf("ok\n");
        }
        break;

    case 3:
        printf("name: ");
        scanf("%s", name);
        if(acc_group_chk(&t->ac, name, &group) != E13_OK){
            printf("could not check group.\n");
            t_perr(&t->db.e, ret);
        } else {
            printf("ok\n");
            printf("id: %u\n", group.gid);
            printf("stat: %i\n", group.stat);
        }
        break;

    case 4:
        printf("name: ");
        scanf("%s", name);
        printf("stat: ");
        scanf("%i", &stt);
        if(acc_group_set_stat(&t->ac, name, stt) != E13_OK){
            printf("could not set group stat.\n");
            t_perr(&t->db.e, ret);
        } else {
            printf("ok\n");
        }
        break;

    case 5:
        if(acc_group_list(&t->ac, &id, &list) != E13_OK){
            printf("could not get group list.\n");
            t_perr(&t->db.e, ret);
        } else {
            printf("ok, %u groups\n", id);
            if(id){
                for(ptr = list; ptr; ptr = ptr->next){
                    printf("name: %s, id: %u, stat: %i\n", ptr->name, ptr->gid, ptr->stat);
                }
                printf("ends\n");
                acc_group_list_free(list);
            }
        }
        break;

    case 6:
        printf("name: ");
        scanf("%s", name);
        printf("pass: ");
        scanf("%s", pass);
        if(acc_user_add(&t->ac, name, pass) != E13_OK){
            printf("could not add user.\n");
            t_perr(&t->db.e, ret);
        } else {
            printf("ok\n");
        }
        break;

    case 7:
        printf("name: ");
        scanf("%s", name);
        if(acc_user_rm(&t->ac, name) != E13_OK){
            printf("could not remove user.\n");
            t_perr(&t->db.e, ret);
        } else {
            printf("ok\n");
        }
        break;

    case 8:
        printf("name: ");
        scanf("%s", name);
        if(acc_user_chk(&t->ac, name, &user) != E13_OK){
            printf("could not check user.\n");
            t_perr(&t->db.e, ret);
        } else {
            printf("ok\n");
            printf("id: %u\n", user.uid);
            printf("stat: %i\n", user.stat);
            printf("hash: %s\n", user.passhash);
            printf("lastdate: %i/%i/%i\n", user.lastdate[0], user.lastdate[1], user.lastdate[2]);
            printf("lasttime: %i\n", user.lasttime);
        }
        break;

    case 9:
        printf("name: ");
        scanf("%s", name);
        printf("stat: ");
        scanf("%i", &stt);
        if(acc_user_set_stat(&t->ac, name, stt) != E13_OK){
            printf("could not set user stat.\n");
            t_perr(&t->db.e, ret);
        } else {
            printf("ok\n");
        }
        break;

    case 10:
        if(acc_user_list(&t->ac, &id, &ulist) != E13_OK){
            printf("could not get user list.\n");
            t_perr(&t->db.e, ret);
        } else {
            printf("ok, %u user\n", id);
            if(id){
                for(uptr = ulist; uptr; uptr = uptr->next){
                    printf("name: %s, id: %u, stat: %i, lastdate: %i/%i/%i, lasttime: %i\n", uptr->name, uptr->uid, uptr->stat, user.lastdate[0], user.lastdate[1], user.lastdate[2], user.lasttime);
                }
                printf("ends\n");
                acc_user_list_free(ulist);
            }
        }
        break;

    case 0:
        return true_;
    default:
        break;

    }

    goto choice;

    return true_;
}

int main(void)
{
    struct t13 t;
    int ch;
    accinit = false_;

    printf("This is lib13 test.\n");    
choice:
    printf("Please select:\n");
    printf("1. acc13\n");
    printf("0. exit\n");
    scanf("%i", &ch);

    switch(ch){

    case 1:
        t_acc13(&t);
        break;
    case 0:
        exit(0);
    default:
        break;

    }

    goto choice;

    return 0;
}

