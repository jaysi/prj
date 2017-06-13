#include "step13.h"

int do_add(struct step13*h, char* arg, int recursive){

    int ret;
    /*
    char** array;

    int n = s13_exparts(arg, STEP13_LIST_DELIM);
    array = s13_exmem(n);
    s13_explode(arg, STEP13_LIST_DELIM, array);

    while(n--){
    */
        ret = insert_record_by_path(h, STEP13_MAIN_PATH_LIST_TABLE,
                                     //array[n], //the function merges the path
                                     arg,
                                     recursive);
/*
        if(ret) break;
    }

    s13_free_exmem(array);
*/
    return ret;
}

int do_add_tag(struct step13*h, char* arg, int recursive){
        return insert_tag(h, STEP13_MAIN_TAG_LIST_TABLE,
                          //array[n], //the function merges the path
                          arg);
}


int do_rm(struct step13* h, char* arg, int recursive){
    int ret;
    char** array;

    int n = s13_exparts(arg, STEP13_LIST_DELIM, 0, 0);
    array = s13_exmem(n);
    s13_explode(arg, STEP13_LIST_DELIM, 0, 0, array);

    begin_trans(h);

    while(n--){

        ret = rm_record_by_path(h, STEP13_MAIN_PATH_LIST_TABLE,
                                     array[n], //the function merges the path
                                     recursive);
        if(ret) break;
    }

    finish_trans(h);

    s13_free_exmem(array);
    return ret;
}

int add_tag(struct step13* h, char* tag, int recursive){
    return 0;
}

int rm_tag(struct step13* h, char* tag, int recursive){
    return 0;
}

int add_comment(struct step13* h, char* comment, int recursive){
    return 0;
}

int rm_comment(struct step13* h, char* comment, int recursive){
    return 0;
}

int add_remove(struct step13* h){

    char **list;
    int nlist;
    int ret=0;
    char* arg;
    char* argcp;
    int recursive;

    int (*addrm)(struct step13* h, char* arg, int recursive);

    if(STEP13_SW(STEP13_SW_PATH)&h->switches) arg = h->sw_arg[STEP13_SW_PATH];
    else arg = h->sw_arg[STEP13_SW_TAG];

    switch(h->cmd){
        case STEP13_CMD_ADD:
            if(arg == h->sw_arg[STEP13_SW_PATH]) addrm = &do_add;
            else addrm = &do_add_tag;
        break;
        case STEP13_CMD_RM:
            addrm = &do_rm;
        break;
        default:
        break;
    }

    if(h->switches&STEP13_SW(STEP13_SW_RECURSIVE)) recursive = 1;
    else recursive = 0;

    if(!(argcp = (char*)malloc(strlen(arg)+1))){
            lineout(h, STEP13_ERROR, "memmory allocation error.");
            ret = -1;
            goto end;
    }

    strcpy(argcp, arg);

    begin_trans(h);

    if((nlist = s13_exparts(argcp, STEP13_LIST_DELIM, 0, 0)) == 1){
        ret = addrm(h, argcp, recursive);        
    } else {
        list = s13_exmem(nlist);
        if(s13_explode(argcp, STEP13_LIST_DELIM, 0, 0, list) != nlist){
            s13_free_exmem(list);
            free(argcp);
            lineout(h, STEP13_ERROR, "failed to explode list \'%s\'.", arg);
            ret = -1;
            goto end;
        }
        while(nlist){
            ret = addrm(h, list[--nlist], recursive);
        }
        s13_free_exmem(list);
    }

    free(argcp);

end:

    finish_trans(h);

    if(ret){
        lineout(h, STEP13_ERROR, "add/remove operation failed.");
    } else {
        lineout(h, STEP13_INFO, "add/remove operation done.");
    }

    //sqlite3_close(h->db);
    return ret;
}
