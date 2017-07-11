#include "monet_internal.h"

error13_t _monet_user_login(struct monet* mn, struct monet_user* user, uid13_t uid){
    struct obj13* node;
    error13_t ret;

    //the user list is locked?
    ret = obj13_bst_create_node(&node, (objid13_t)uid, user, OBJ13_FLAG_DEF);
    if(ret != E13_OK) return ret;

    if(mn->user_array.obj_root == NULL) mn->user_array.obj_root = node;
    else return obj13_bst_insert_node((mn->user_array.obj_root, node);

    return E13_OK;
}

error13_t _monet_user_logout(struct monet* mn, uid13_t uid){

}

error13_t _monet_user_find(struct monet* mn, char* name, uid13_t* uid){

}
