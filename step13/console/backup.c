#include "step13.h"

int backup_file(void* vh){

    struct step13* h = (struct step13*)vh;
    char target[MAXPATHNAME], source[MAXPATHNAME];

    while(sqlite3_step(h->stmt) == SQLITE_ROW){

        p13_join_path(h->sw_arg[STEP13_SW_BACKUPDIR],
                       (char*)sqlite3_column_text(h->stmt, STEP13_COLID_PATH),
                       target);

        s13_strcpy(source, target, MAXPATHNAME);

        p13_create_dir_struct(target);

        s13_strcpy(target, source, MAXPATHNAME);

        p13_merge_path(h->sw_arg[STEP13_SW_ROOT],
                       (char*)sqlite3_column_text(h->stmt, STEP13_COLID_PATH),
                       source);

        lineout(h, STEP13_INFO, "backing up \'%s\'.", source);

        if(io13_copy_file(target, source, 0) < 0){
            lineout(h, STEP13_ERROR, "failed to copy \'%s\' to \'%s\'.",
                    source, target);
            return -1;
        }
    }

    return 1;
}

int backup(struct step13* h){

    /*
     *  needs compare table
    */

    char *backupdir = h->sw_arg[STEP13_SW_BACKUPDIR];
    char bakdir[MAXTIME];
    char path[MAXPATHNAME];
    FILE* commentfile;

    d13_now(bakdir);

    lineout(h, STEP13_INFO, "creating backup \'%s\'...", bakdir);

    /*
    if(compare(h)){
        lineout(h, STEP13_ERROR, "backup failed.");
        return -1;
    }
    */

    p13_join_path(backupdir, bakdir, backupdir);

#ifdef __WIN32
    if(mkdir(backupdir)){
#else
    if(mkdir(backupdir, MKDIRPERM)){
#endif        
        lineout(h, STEP13_ERROR, "unable to create directory \'%s\', backup failed.", backupdir);
        return -1;
    }

    if(h->switches & STEP13_SW(STEP13_SW_COMMENT)){

        p13_join_path(backupdir, STEP13_BACKUP_COMMENT, path);
        commentfile = fopen(path, "w+");
        if(!commentfile){
            lineout(h, STEP13_ERROR, "unable to create comment file \'%s\', backup failed.", path);
            return -1;
        }
        fprintf(commentfile, "%s", h->sw_arg[STEP13_SW_COMMENT]);
        fclose(commentfile);

    }

    h->run_query_fn = &backup_file;

    if(run_query2(  h, 1,
                    STEP13_COMPARE_PATH_LIST_TABLE,
                    STEP13_SELECT_BY_TYPE_STAT,
                    "ss",
                    P13_TYPE_FILE,
                    STEP13_STAT_UPDATE)){

         lineout(h, STEP13_INFO, "failed to create backup.");
         return -1;
    }

    lineout(h, STEP13_INFO, "finished creating backup.");
    return 0;
}

