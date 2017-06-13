#include "step13.h"

#include <assert.h>
#include <unistd.h>

#define _deb1(f, a...)

#define LITE_INT(stmt, icol)    sqlite3_column_int(stmt, icol)
#define LITE_CHAR(stmt, icol)    (char*)sqlite3_column_text(stmt, icol)

int copy_new_files(void* vh){

    struct step13* h = (struct step13*)vh;
    char target[MAXPATHNAME], source[MAXPATHNAME];
    char sizebuf[MAXSIZEBUF];
    struct sqlite3_stmt* tmp;
    struct stat st;

    begin_trans(h);

    while(sqlite3_step(h->stmt) == SQLITE_ROW){

        _deb1("source: %s", (char*)sqlite3_column_text(h->stmt, STEP13_COLID_PATH));

        p13_merge_path2(                h->sw_arg[STEP13_SW_SOURCE],
                                        (char*)sqlite3_column_text(h->stmt, STEP13_COLID_PATH),
                                        h->sw_arg[STEP13_SW_ROOT],
                                        target
                                        );

        _deb1("target: %s", target);

        /*create a copy of target, next call is gonna change the bytes!*/
        s13_strcpy(source, target, MAXPATHNAME);

        p13_create_dir_struct(target);

        //restore the target!
        s13_strcpy(target, source, MAXPATHNAME);

        /*
        p13_merge_path(h->sw_arg[STEP13_SW_SOURCE],
                       (char*)sqlite3_column_text(h->stmt, STEP13_COLID_PATH),
                       source);
        */

        s13_strcpy(source, (char*)sqlite3_column_text(h->stmt, STEP13_COLID_PATH), MAXPATHNAME);

        _deb1("source: %s", source);

        lineout(h, STEP13_INFO, "\'%s\' -> '%s' [%s]",
                source, target,
                get_size((size_t)sqlite3_column_int(h->stmt, STEP13_COLID_SIZE),
                sizebuf));

        /*
         * TODO:
         *  This is dangerous! the true way is to copy the file under a temp
         *  name, remove the target and rename the temp name to the target.
        */

        if(!access(target, F_OK)){

            //update the record; use STEP13_SET_STAT_BY_PATH

            if(stat(target, &st) == -1){
                finish_trans(h);
                lineout(h, STEP13_ERROR, "could not get stat for \'%s\', updating failed.", target);
                return -1;
            }

            tmp = h->stmt;
            if(run_query2(  h, 1,
                            STEP13_MAIN_PATH_LIST_TABLE,
                            STEP13_SET_STAT_BY_PATH,
                            "iiiss",
                            st.st_size,
                            st.st_mtime,
                            st.st_ctime,
                            STEP13_STAT_SYNC,
                            target)){

                finish_trans(h);

                lineout(h, STEP13_ERROR, "could not update record for \'%s\', updating failed.", target);
                return -1;

            }
            h->stmt = tmp;

            remove(target);
        } else {
            //insert the record; use STEP13_INSERT_PATH_DATA
            tmp = h->stmt;
            if(run_query2(  h, 1,
                            STEP13_MAIN_PATH_LIST_TABLE,
                            STEP13_INSERT_PATH_DATA,
                            STEP13_INSERT_FMT,
                            target,
                            LITE_CHAR(tmp, STEP13_COLID_TYPE),
                            LITE_INT(tmp, STEP13_COLID_CTIME),
                            LITE_INT(tmp, STEP13_COLID_MTIME),
                            LITE_INT(tmp, STEP13_COLID_SIZE),
                            LITE_CHAR(tmp, STEP13_COLID_HASH),
                            LITE_CHAR(tmp, STEP13_COLID_COMMENT),
                            LITE_CHAR(tmp, STEP13_COLID_TAG),
                            LITE_INT(tmp, STEP13_COLID_LEVEL),
                            STEP13_STAT_SYNC)){

                finish_trans(h);

                lineout(h, STEP13_ERROR, "could not update record for \'%s\', updating failed.", target);
                return -1;

            }
            h->stmt = tmp;

        }

        if(io13_copy_file(target, source, 0) < 0){
            lineout(h, STEP13_ERROR, "failed to copy \'%s\' to \'%s\'.",
                    source, target);

            finish_trans(h);

            return -1;
        }
    }

    finish_trans(h);

    return 1;
}

int commit(struct step13* h){

    lineout(h, STEP13_INFO, "committing from \'%s\' to \'%s\'...",
            h->sw_arg[STEP13_SW_SOURCE], h->sw_arg[STEP13_SW_ROOT]);

    if(!strcmp(h->sw_arg[STEP13_SW_ROOT], h->sw_arg[STEP13_SW_SOURCE])){
        lineout(h, STEP13_ERROR, "cannot commit a path to itself!, use --source switch to define the source.");
        return -1;
    }

    if(compare(h)){
        lineout(h, STEP13_ERROR, "backup failed.");
        return -1;
    }

    if(!(h->switches & STEP13_SW(STEP13_SW_NOBACKUP))){
        if(backup(h)){
            lineout(h, STEP13_ERROR, "failed to commit.");
            return -1;
        }
    }

    h->run_query_fn = &copy_new_files;

    if(run_query2(h, 1, STEP13_COMPARE_PATH_LIST_TABLE,
                        STEP13_SELECT_NEW, "sss",
                        P13_TYPE_FILE,
                        STEP13_STAT_UPDATE,
                        STEP13_STAT_ADD)){

        lineout(h, STEP13_ERROR, "failed to commit.");
        return -1;

    }

    lineout(h, STEP13_INFO, "commit done.")

    return 0;
}
