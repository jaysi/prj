#include "step13.h"

int init(struct step13* h){

    /*
     *acceptable arguments are:
     *--root
    */

    char path[MAXPATHNAME];    

    lineout(h, STEP13_INFO, "initializing \'%s\'...", h->sw_arg[STEP13_SW_ROOT]);

    p13_join_path(h->sw_arg[STEP13_SW_ROOT], STEP13_DIR, path);

#ifdef __WIN32
    if(mkdir(path)){
#else
    if(mkdir(path, MKDIRPERM)){
#endif        
        lineout(h, STEP13_ERROR, "could not create step13 directory \'%s\'; init failed", path);
        return -1;
    }

#ifdef __WIN32
    if(mkdir(h->sw_arg[STEP13_SW_BACKUPDIR])){
#else
    if(mkdir(h->sw_arg[STEP13_SW_BACKUPDIR], MKDIRPERM)){
#endif    
        lineout(h, STEP13_ERROR, "could not create default backup directory \'%s\'; init failed", h->sw_arg[STEP13_SW_BACKUPDIR]);
        return -1;
    }

    if(open_db(h)){
        lineout(h, STEP13_ERROR, "could not open db.");
        return -1;
    }

    if(create_main_list(h, 1)){
        lineout(h, STEP13_ERROR, "init failed.");
        return -1;
    }

    if(create_main_tag_list(h)){
        lineout(h, STEP13_ERROR, "init failed.");
        return -1;
    }

    if(create_table(h, STEP13_COMPARE_PATH_LIST_TABLE, STEP13_CREATE_PATH_LIST)){
        lineout(h, STEP13_ERROR, "init failed.");
        return -1;
    }

    lineout(h, STEP13_INFO, "init done.");

    return 0;

}

