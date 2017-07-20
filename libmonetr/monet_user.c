#include "monet_internal.h"

error13_t _monet_user_login(struct monet* mn, struct infolink* link, char username, char password){
    struct obj13* node;
    error13_t ret;
    uid13_t uid;

    //locking poll_list is not needed, the link is malloc'ed, not still in the list

    _deb_connect("logging in %s", username);
    if((ret = acc_user_login(&mn->ac, username, password, &uid)) != E13_OK){
		_deb_connect("login_user failed code: %i, msg: %s", ret,
					e13_codemsg(ret));
		mn_clog(mn, MN_CLOGID_ALL, mn_msg(mn, MN_MSGID_LOGIN_FAILED), username, ret);
		return ret;
    }

    //alloc + init user struct
    mn_user = (struct monet_user*)m13_malloc(sizeof(struct monet_user));

    mn_user->next = NULL;

    mn_user->link = link;
    mn_user->name = s13_malloc_strcpy(username, MN_MAX_USERNAME);
    mn_user->uid = uid;
//    mn_user->gid_array = NULL;
//    mn_user->name = mn->conf.guest_user;//TODO: TEMP
//    mn_user->id == ++uid;//TODO: TEMP


    th13_mutex_lock(&mn->user_array.mx);

    if(!mn->user_array.array){
        mn->user_array.array = (struct monet_user_array_entry*)m13_malloc(
                    sizeof(struct monet_user_array_entry)*
                    mn->conf.user_bulk_size);
        mn->user_array.nalloc = mn->conf.user_bulk_size;
        mn->user_array.nactive = 0UL;
        mn->user_array.npause = 0UL;

        for( i = 0; i < mn->conf.user_bulk_size; i++ ) {
            mn->user_array.array[i].user = NULL;
        }

        i = 0;
    } else {
        if(mn->user_array.nalloc == mn->user_array.nactive + mn->user_array.npause){
            mn->user_array.array = (struct monet_user_array_entry*)m13_realloc(
                        mn->user_array.array,
                        sizeof(struct monet_user_array_entry)*
                        (mn->conf.user_bulk_size + mn->user_array.nalloc));
            for(i = 0; i < mn->conf.user_bulk_size; i++){
                mn->user_array.array[i + mn->user_array.nalloc].user = NULL;
            }
            i = mn->user_array.nalloc;
            mn->user_array.nalloc += mn->conf.user_bulk_size;
        } else {
            for(i = 0; i < mn->user_array.nalloc; i++){
                if(!mn->user_array.array[i].user) break;
            }
        }
    }

    //the link is not in the poll list, no need to lock, don't worry!
    link->ext_ctx = mn_user;

	mn_user->i = i;
    mn->user_array.array[i].user = mn_user;
    mn->user_array.nactive++;

    //the user list is locked?
    if((ret = obj13_bst_create_node(&node, (objid13_t)uid, user, OBJ13_FLAG_DEF)) != E13_OK){
        goto end;
    }

    if(mn->user_array.obj_root == NULL) mn->user_array.obj_root = node;
    else ret = obj13_bst_insert_node((mn->user_array.obj_root, node);

end:
    if(ret != E13_OK){
        if(node) obj13_bst_free_node(node);
    }
    th13_mutex_unlock(&mn->user_array.mx);
    return ret;
}

error13_t _monet_user_logout(struct monet* mn, uid13_t uid){

    struct obj13* node;
    error13_t ret;
    struct mn_user* user;

    //BE AGGRESSIVE! CLOSE THE CONNECTION!
    th13_mutex_lock(&mn->user_array.mx);

    if((ret = obj13_bst_find_node(mn->user_array.obj_root, uid, &node)) != E13_OK){
        goto end;
    }

    user = mn->user_array.array[((struct mn_user*)node->objptr)->i].user;
    mn->user_array.array[((struct mn_user*)node->objptr)->i].user = NULL;
    mn->user_array.nactive--;

    //no need to lock anything, since the link and user mutexes are the same

    th13_mutex_unlock(&mn->user_array.mx);

end:
    return ret;
}

error13_t _monet_user_find(struct monet* mn, char* name, uid13_t* uid){

}
