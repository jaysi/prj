#include "step13.h"

#include <stdarg.h>
#include <assert.h>

#define _deb1(f, a...)
#define _deb2(f, a...)
#define _deb3(f, a...)
#define _deb4(f, a...)
#define _deb5(f, a...)
#define _deb6(f, a...)
#define _deb7(f, a...)

int run_query(struct step13* h, int do_cleanup){

    int ret;

    if(sqlite3_prepare_v2(h->db, h->sql, strlen(h->sql), &h->stmt, NULL) != SQLITE_OK){
        lineout(h, STEP13_ERROR, "failed to precompile sql statement \'%s\', dbms says: %s", h->sql, sqlite3_errmsg(h->db));
        return -1;
    }

    h->nresults = 0;

    if(h->run_query_fn){
        while(!(ret = h->run_query_fn(h)));
    } else {
again:
        switch(sqlite3_step(h->stmt)){
            case SQLITE_ROW:
                show_row(h);
                h->nresults++;
                goto again;
            case SQLITE_DONE:
            case SQLITE_OK:
                ret = 0;
            break;
            default:
                lineout(h, STEP13_ERROR, "failed to run precompile sql statement \'%s\', dbms says: %s", h->sql, sqlite3_errmsg(h->db));
                ret = -1;
            break;

        }
    }

    if(do_cleanup){
        //sqlite3_reset(h->stmt);
        sqlite3_finalize(h->stmt);
        h->sql[0]='\0';
    }

    if(ret > 0) ret = 0;
    return ret;
}

int run_query2( struct step13* h,
                int do_cleanup,
                char* table,
                char* basesql,
                char* bind_fmt,
                ...
                ){

    int ret = 0, i, intval;
    va_list vl;
    char* text;

    snprintf(h->sql, MAXSQL, basesql, table);

    _deb5("sql: %s", h->sql);

    if(sqlite3_prepare_v2(h->db, h->sql, strlen(h->sql), &h->stmt, NULL) != SQLITE_OK){
        lineout(h, STEP13_ERROR, "failed to precompile sql statement \'%s\', dbms says: %s", h->sql, sqlite3_errmsg(h->db));
        return -1;
    }

    va_start(vl, bind_fmt);

    for(i = 0; bind_fmt[i] != '\0'; ++i){

        if(ret < 0) break;

        switch(bind_fmt[i]){

            case 'i':
            intval = va_arg(vl, int);
            if(sqlite3_bind_int(h->stmt, i + 1, intval) != SQLITE_OK){
                lineout(h, STEP13_ERROR, "failed to bind int, dbms says: %s", sqlite3_errmsg(h->db));
                ret = -1;
            }
            break;

            case 't':
            case 's':

            text = va_arg(vl, char*);

            _deb5("binding %s to posotion %i", text, i+1);

            if(sqlite3_bind_text(h->stmt, i + 1, text, strlen(text)+1, SQLITE_STATIC) != SQLITE_OK){
                lineout(h, STEP13_ERROR, "failed to bind text, dbms says: %s", sqlite3_errmsg(h->db));
                ret = -1;

            }
            break;

            default:
                lineout(h, STEP13_ERROR, "could not understand format specified \'%c\'", bind_fmt[i]);
                ret = -1;
            break;
        }
    }

    va_end(vl);

    h->nresults = 0;

    if(!ret){

        if(h->run_query_fn){
            while(!(ret = h->run_query_fn(h)));
        } else {
again:
            switch(sqlite3_step(h->stmt)){
                case SQLITE_ROW:
                    h->nresults++;
                    show_row(h);
                    goto again;
                case SQLITE_DONE:
                case SQLITE_OK:
                    ret = 0;
                break;
                default:
                    lineout(h, STEP13_ERROR, "failed to run precompile sql statement \'%s\', dbms says: %s", h->sql, sqlite3_errmsg(h->db));
                    ret = -1;
                break;

            }
        }
    }

    if(do_cleanup){
        //sqlite3_reset(h->stmt);
        sqlite3_finalize(h->stmt);
        h->sql[0]='\0';
    }

    if(ret > 0) ret = 0;
    return ret;
}

int open_db(struct step13* h){
    char path[MAXPATHNAME];

    p13_join_path(h->sw_arg[STEP13_SW_ROOT], STEP13_DIR, path);
    p13_join_path(path, MAIN_DB, path);

    lineout(h, STEP13_DEBUG, "openning \'%s\'...", path);

    if(sqlite3_open(path, &h->db) != SQLITE_OK){
        lineout(h, STEP13_ERROR, "failed to open database %s, dbms says: %s", path, sqlite3_errmsg(h->db));
        return -1;
    }

    return 0;
}

int open_source_db(struct step13* h){
    char path[MAXPATHNAME];

    assert(h->sw_arg[STEP13_SW_SOURCE]);

    p13_join_path(h->sw_arg[STEP13_SW_SOURCE], STEP13_DIR, path);
    p13_join_path(path, MAIN_DB, path);

    lineout(h, STEP13_DEBUG, "openning \'%s\'...", path);

    return run_query2(h, 1, STEP13_SOURCE_DB, STEP13_ATTACH_DB, "s", path);

}

int enum_dir(struct step13* h, char* root, void* callback_fn, void* param){

    struct dirent **namelist;
    int n;
    char path[MAXPATHNAME];
    struct stat sb;
    //int level = p13_get_path_depth(root);
    int (*callback)(struct step13* h, struct stat* st, char* path, void* param);
    callback = callback_fn;

    n = scandir(root, &namelist, 0, alphasort);
    lineout(h, STEP13_DEBUG, "enumurating < %s >, n = %i", root, n);
    if (n < 0){
        lineout(h, STEP13_ERROR, "scandir() failed with err #%i, working directory is < %s >", n, root);
        return n;
    } else {
        while (n--) {

            p13_merge_path(root, namelist[n]->d_name, path);

            if(stat(path, &sb) != -1){
                switch (sb.st_mode & S_IFMT) {
                    case S_IFDIR:

                        if( !strcmp(namelist[n]->d_name, PREV_DIR)||
                            !strcmp(namelist[n]->d_name, THIS_DIR)||
                            !strcmp(namelist[n]->d_name, STEP13_DIR))
                                break;
                        if(callback(h, &sb, path, param) < 0){
                            _deb1("callback failed");
                            return -1;
                        }
                        //level++;
                        if(enum_dir(h, path, callback_fn, param)){
                            _deb1("enum_dir failed");
                            return -1;
                        }
                        break;
                    case S_IFREG:
                        if(callback(h, &sb, path, param) < 0){
                            _deb1("callback failed");
                            return -1;
                        }
                        break;
                    default:
                        break;
                }
            }
            free(namelist[n]);
        }
        free(namelist);
    }

    //level--;
    return 0;
}

int bind_path_data_rec(sqlite3_stmt* stmt, char* path,
                       struct stat* st, char* comment, char* tags, int level,
                       char* stat, int update){

    //char t[MAXTIME];
    char hash[MAXHASH];
    char* type = p13_type_str(st);
    int ret = 0;

    //path = p13_get_unix_path(path);

    //useful when updating
    if(!update){
        if(sqlite3_bind_text(stmt, 1, path, strlen(path)+1, SQLITE_STATIC) != SQLITE_OK){
            ret = -1;
            goto end;
        }
    }

    if(sqlite3_bind_text(stmt, 2, type, strlen(type)+1, SQLITE_STATIC) != SQLITE_OK){ret = -2; goto end;}

    if(sqlite3_bind_int(stmt, 3, (int)st->st_ctime) != SQLITE_OK){ret = -3; goto end;}

    if(sqlite3_bind_int(stmt, 4, (int)st->st_mtime) != SQLITE_OK){ret = -4; goto end;}

    if(sqlite3_bind_int(stmt, 5, (int)st->st_size) != SQLITE_OK){ret = -5; goto end;}


    if(!strcmp(type, P13_TYPE_FILE)){
        h13_md5_file(path, hash);
    } else {
        strcpy(hash, STEP13_NA);
    }
    if(sqlite3_bind_text(stmt, 6, hash, strlen(hash)+1, SQLITE_STATIC) != SQLITE_OK){ret = -6; goto end;}


    if(!comment){
        if(sqlite3_bind_text(stmt, 7, STEP13_NA, strlen(STEP13_NA)+1, SQLITE_STATIC) != SQLITE_OK){ret = -7; goto end;}

    } else {
        if(sqlite3_bind_text(stmt, 7, comment, strlen(comment)+1, SQLITE_STATIC) != SQLITE_OK){ret = -8; goto end;}

    }

    if(!tags){
        if(sqlite3_bind_text(stmt, 8, STEP13_NA, strlen(STEP13_NA)+1, SQLITE_STATIC) != SQLITE_OK){ret = -9; goto end;}

    } else {
        if(sqlite3_bind_text(stmt, 8, tags, strlen(tags)+1, SQLITE_STATIC) != SQLITE_OK){ret = -10; goto end;}

    }

    if(sqlite3_bind_int(stmt, 9, level) != SQLITE_OK){ret = -11; goto end;}


    if(!stat){
        if(sqlite3_bind_text(stmt, 10, STEP13_NA, strlen(STEP13_NA)+1, SQLITE_STATIC) != SQLITE_OK){ret = -12; goto end;}

    } else {
        if(sqlite3_bind_text(stmt, 10, stat, strlen(stat)+1, SQLITE_STATIC) != SQLITE_OK){ret = -13; goto end;}

    }

    if(update) if(sqlite3_bind_text(stmt, 11, path, strlen(path)+1, SQLITE_STATIC) != SQLITE_OK){ret = -14; goto end;}

    end:

    _deb1("ret = %i", ret);

    return ret;
}

int create_list_enum_callback(struct step13* h, struct stat* st, char* path, sqlite3_stmt* insert_stmt){

    _deb1("called for \'%s\'...", path);

    if(bind_path_data_rec( insert_stmt, path, st,
                           h->switches&STEP13_SW(STEP13_SW_COMMENT)?h->sw_arg[STEP13_SW_COMMENT]:NULL,
                           h->switches&STEP13_SW(STEP13_SW_TAG)?h->sw_arg[STEP13_SW_TAG]:NULL,
                           p13_get_path_depth(path), STEP13_STAT_SYNC, 0))
                            return -1;

    if(sqlite3_step(insert_stmt) != SQLITE_DONE){
        lineout(h, STEP13_ERROR, "failed to run precompile sql statement \'%s\', dbms says: %s", h->sql, sqlite3_errmsg(h->db));
        return -1;
    }

    h->qsize += st->st_size;

    if(!strcmp(p13_type_str(st), P13_TYPE_DIR)) h->dirs++;
    else if(!strcmp(p13_type_str(st), P13_TYPE_FILE)) h->files++;
    else h->others++;

    lineout(h, STEP13_INFO, "added \"%s\";", path);

    sqlite3_reset(insert_stmt);

    return 0;

}

int set_update_stat(void* vh){
    sqlite3_stmt* tmp;
    struct step13* h = (struct step13*)vh;
    struct stat* st = (struct stat*)h->run_query_arg;

    tmp = h->stmt;

    switch(sqlite3_step(tmp)){

    case SQLITE_ROW:

        if(
            st->st_size!= (off_t)sqlite3_column_int(tmp, STEP13_COLID_SIZE) ||
            st->st_mtime!= (time_t)sqlite3_column_int(tmp, STEP13_COLID_MTIME)||
            st->st_ctime!= (time_t)sqlite3_column_int(tmp, STEP13_COLID_CTIME)
        ){

            lineout( h, STEP13_INFO, "%s\'%s\'.", STEP13_SIGN_UPDATE,
                    (char*)sqlite3_column_text(tmp, STEP13_COLID_PATH));

            run_query2( h, 1, STEP13_COMPARE_PATH_LIST_TABLE,
                        STEP13_SET_STAT_BY_PATH, "iiiss",
                        (int)st->st_size, (int)st->st_mtime, (int)st->st_ctime,
                        STEP13_STAT_UPDATE,
                        (char*)sqlite3_column_text(tmp, STEP13_COLID_PATH));
        } else {

            lineout( h, STEP13_INFO, "%s\'%s\'.", STEP13_SIGN_SYNC,
                    (char*)sqlite3_column_text(tmp, STEP13_COLID_PATH));

            run_query2( h, 1, STEP13_COMPARE_PATH_LIST_TABLE,
                        STEP13_SET_STAT_BY_PATH, "iiiss",
                        (int)st->st_size, (int)st->st_mtime, (int)st->st_ctime,
                        STEP13_STAT_SYNC,
                        (char*)sqlite3_column_text(tmp, STEP13_COLID_PATH));

        }

    break;

    case SQLITE_DONE:

        h->stmt = tmp;

        return -2;

    break;

    default:
        return -1;
    break;

    }

    h->stmt = tmp;

    return 1;//to finish the loop

}

int update_list_enum_callback(struct step13* h, struct stat* st, char* path, sqlite3_stmt* insert){

    char comp_path[MAXPATHNAME];
    _deb1("called for \'%s\'...", path);

    //1.check to see if the path already exists
    //2.if not exists add it with stat set as NEW
    //3.else, check to see if the path is different from the table entry
    //4.if different, set stat as UPDATED
    //5.else set stat as SYNC

    //to prepare the correct path for COMPARING:
    // . the given path is the path of the source file.
    //1. get unix path
    //2. strip the source's dir from the path
    //3. join it with root path
    //4. the file put in the
    //5. the original path must put on the table to make copying, etc possible

    //upath = p13_get_unix_path(path);

    _deb7("comparing %s with ...", path);

    p13_merge_path2(  h->sw_arg[STEP13_SW_SOURCE],
                      path,
                      h->sw_arg[STEP13_SW_ROOT],
                      comp_path);

    _deb7("... %s", comp_path);


    h->run_query_fn = &set_update_stat;
    h->run_query_arg = st;

    switch(run_query2(h, 1, STEP13_COMPARE_PATH_LIST_TABLE, STEP13_SELECT_BY_PATH, "s", comp_path)){

    case -2:

        //new file! add it!

        lineout( h, STEP13_INFO, "%s\'%s\'.", STEP13_SIGN_ADD,
                 path);

        if(bind_path_data_rec( insert, path, st,
                               h->switches&STEP13_SW(STEP13_SW_COMMENT)?h->sw_arg[STEP13_SW_COMMENT]:NULL,
                               h->switches&STEP13_SW(STEP13_SW_TAG)?h->sw_arg[STEP13_SW_TAG]:NULL,
                               p13_get_path_depth(path), STEP13_STAT_ADD, 0)){
                                return -1;
                                }

        if(sqlite3_step(insert) != SQLITE_DONE){
            lineout(h, STEP13_ERROR, "failed to run precompile sql statement \'%s\', dbms says: %s", h->sql, sqlite3_errmsg(h->db));           
            return -1;
        }

        sqlite3_reset(insert);

        break;
    case 0:
    break;
    case -1:
    return -1;
    break;

    }

    h->qsize += st->st_size;

    if(!strcmp(p13_type_str(st), P13_TYPE_DIR)) h->dirs++;
    else if(!strcmp(p13_type_str(st), P13_TYPE_FILE)) h->files++;
    else h->others++;

    return 0;

}

int create_main_tag_list(struct step13* h){

    snprintf(h->sql, MAXSQL, STEP13_CREATE_TAG_LIST, STEP13_MAIN_TAG_LIST_TABLE);

    return run_query(h, 1);
}

int create_table(struct step13* h, char* table, char* basesql){

    snprintf(h->sql, MAXSQL, basesql, table);

    h->run_query_fn = NULL;

    switch(run_query(h, 1)){
        case 0:
        lineout(h, STEP13_INFO, "created table.");
        return 0;
        break;
        default:
        lineout(h, STEP13_ERROR, "failed to create table.");
        return -1;
        break;
    }

    return -1;

}

int create_main_list(struct step13* h, int create_table){

    sqlite3_stmt* insert_stmt;
    int ret;

    if(create_table){
        lineout(h, STEP13_INFO, "creating main list...");

        snprintf(h->sql, MAXSQL, STEP13_CREATE_PATH_LIST, STEP13_MAIN_PATH_LIST_TABLE);

        h->run_query_fn = NULL;

        switch(run_query(h, 1)){
            case 0:
            lineout(h, STEP13_INFO, "created main list.");
            break;
            default:
            lineout(h, STEP13_ERROR, "failed to create main list.");
            return -1;
            break;
        }
    }

    lineout(h, STEP13_INFO, "filling list...");

    snprintf(h->sql, MAXSQL, STEP13_INSERT_PATH_DATA, STEP13_MAIN_PATH_LIST_TABLE);

    _deb1("sql: %s", h->sql);

    if(sqlite3_prepare_v2(h->db, h->sql, strlen(h->sql), &insert_stmt, NULL) != SQLITE_OK){
        lineout(h, STEP13_ERROR, "failed to precompile sql statement \'%s\', dbms says: %s", h->sql, sqlite3_errmsg(h->db));
        return -1;
    }

    begin_trans(h);

    switch(enum_dir(h, h->sw_arg[STEP13_SW_ROOT], &create_list_enum_callback, insert_stmt)){
        case 0:
        lineout(h, STEP13_INFO, "done enumurating.");
        ret = 0;
        break;
        default:
        lineout(h, STEP13_INFO, "enumurating failed.");
        ret = -1;
        break;
    }

    sqlite3_finalize(insert_stmt);
    finish_trans(h);

    return ret;
}

int print_list(struct step13* h, char* table, void* show){
    lineout(h, STEP13_INFO, "getting list...");

    snprintf(h->sql, MAXSQL, STEP13_SELECT_ALL, table);

    h->run_query_fn = show;

    switch(run_query(h, 1)){
        case 0:
        lineout(h, STEP13_INFO, "got list.");
        return 0;
        break;
        default:
        lineout(h, STEP13_ERROR, "failed to get list.");
        return -1;
        break;
    }

    return 0;
}

int print_path_list(struct step13* h, char* table){

    return print_list(h, table, &show_row);
}

int print_tag_list(struct step13* h, char* table){

    return print_list(h, table, &show_tags);
}

int delete_table(struct step13* h, char* table){

    snprintf(h->sql, MAXSQL, STEP13_DELETE_TABLE, table);
    return run_query(h, 1);

}

int empty_table(struct step13* h, char* table){

    snprintf(h->sql, MAXSQL, STEP13_EMPTY_TABLE, table);
    return run_query(h, 1);

}

int copy_table(struct step13* h, char* select_sql, char* source, char* target){

    sqlite3_stmt* select;

    empty_table(h, target);

    /*
    //CREATE TABLE dest_table AS (SELECT * FROM source_table); if not exists
    //INSERT INTO dest_table (SELECT * FROM source_table); if exists

    snprintf(h->sql, MAXSQL, STEP13_COPY_TABLE, target, source);
    return run_query(h, 1);
    */

    snprintf(h->sql, MAXSQL, select_sql, source);

    if(sqlite3_prepare_v2(h->db, h->sql, strlen(h->sql), &select, NULL) != SQLITE_OK){
        lineout(h, STEP13_ERROR, "failed to precompile sql statement \'%s\', dbms says: %s", h->sql, sqlite3_errmsg(h->db));
        return -1;
    }

    while(sqlite3_step(select) == SQLITE_ROW){

        if( run_query2( h, 1, target,
                        STEP13_INSERT_PATH_DATA,
                        STEP13_INSERT_FMT,
                        (char*)sqlite3_column_text(select, STEP13_COLID_PATH),
                        (char*)sqlite3_column_text(select, STEP13_COLID_TYPE),
                        sqlite3_column_int(select, STEP13_COLID_CTIME),
                        sqlite3_column_int(select, STEP13_COLID_MTIME),
                        sqlite3_column_int(select, STEP13_COLID_SIZE),
                        (char*)sqlite3_column_text(select, STEP13_COLID_HASH),
                        (char*)sqlite3_column_text(select, STEP13_COLID_COMMENT),
                        (char*)sqlite3_column_text(select, STEP13_COLID_TAG),
                        sqlite3_column_int(select, STEP13_COLID_LEVEL),
                        (char*)sqlite3_column_text(select, STEP13_COLID_STAT)
                        )){

            sqlite3_finalize(select);
            return -1;

        }

    }

    sqlite3_finalize(select);

    return 0;

}

int count_results(void* vh){
    struct step13* h = (struct step13*)vh;
    if(sqlite3_step(h->stmt) == SQLITE_ROW){

        h->nresults++;
        return 0;
    } else return 1;
}

int is_path_exists(struct step13* h, char* table, char* path){

    lineout(h, STEP13_DEBUG, "checking for path \'%s\'@\'%s\' existance...",
            path, table);

    h->run_query_fn = &count_results;

    if(!run_query2(h, 1, table, STEP13_SELECT_BY_PATH, "s", path)){
        _deb4("results: %i", h->nresults);

        if(h->nresults) return 0;
        else return 1;
    } else {
        return -1;
    }

}

char* get_path_tags_enum_callback(void* handle){
    struct step13* h = (struct step13*)handle;

    return (char*)sqlite3_column_text(h->stmt, STEP13_COLID_TAG);

}

int insert_tag(struct step13* h, char* table, char* tag){
    char** array;
    int n = s13_exparts(tag, STEP13_LIST_DELIM, 0, 0);
    if(n>1){
        array = s13_exmem(n);
        s13_explode(tag, STEP13_LIST_DELIM, 0, 0, array);
        while(n--) insert_tag(h, table, array[n]);
    }

    lineout(h, STEP13_DEBUG, "checking for existance of tag \'%s\' @ \'%s\'...",
            tag, table);

    h->run_query_fn = &count_results;

    if(!run_query2(h, 1, table, STEP13_SELECT_BY_TAG, "s", tag)){
        if(h->nresults) return 0;//already exists;
    }

    return run_query2(h, 1, table, STEP13_INSERT_TAG, "s", tag);

}

int create_list_if_not_exists_enum_callback(struct step13* h, struct stat* st,
                                            char* path,
                                            sqlite3_stmt* insert_stmt){

    sqlite3_stmt* select;
    char* tags, *oldtags, *newtags, *comment;
    int ret;

    if(!is_path_exists(h, STEP13_MAIN_PATH_LIST_TABLE, path)){

        //update tag/comment of the path
        //if updating tags, get old tags and mix them with new tags

        _deb5("path exists!");

        if(h->switches&STEP13_SW(STEP13_SW_TAG)) newtags=h->sw_arg[STEP13_SW_TAG];
        else newtags=STEP13_NA;
        if(h->switches&STEP13_SW(STEP13_SW_COMMENT)) comment=h->sw_arg[STEP13_SW_COMMENT];
        else comment=STEP13_NA;

        //don't do anything
        if(comment == STEP13_NA && newtags == STEP13_NA){
            lineout(h, STEP13_WARNING, "path \'%s\' already exists.", path);
            return 1;
        }

        snprintf(h->sql, MAXSQL, STEP13_SELECT_BY_PATH, STEP13_MAIN_PATH_LIST_TABLE);

        if(sqlite3_prepare_v2(h->db, h->sql, strlen(h->sql), &select, NULL) != SQLITE_OK){
            lineout(h, STEP13_ERROR, "failed to precompile sql statement \'%s\', dbms says: %s", h->sql, sqlite3_errmsg(h->db));
            ret = -1;
            goto end;
        }

        if(sqlite3_bind_text(select, 1, path, strlen(path)+1, SQLITE_STATIC) != SQLITE_OK){
            lineout(h, STEP13_ERROR, "failed to bind statement, dbms says: %s", sqlite3_errmsg(h->db));
            ret = -1;
            goto end;
        }

        if(sqlite3_step(select) != SQLITE_ROW){
            lineout(h, STEP13_ERROR, "could not find the path, dbms says: %s", sqlite3_errmsg(h->db));
            ret = -1;
            goto end;
        }

        oldtags = (char*)sqlite3_column_text(select, STEP13_COLID_TAG);
        if(comment == STEP13_NA) comment = (char*)sqlite3_column_text(select, STEP13_COLID_COMMENT);

        if(newtags!=STEP13_NA) tags=(char*)malloc(strlen(oldtags)+strlen(newtags)+1);
        else tags=oldtags;

        if(!tags){
            lineout(h, STEP13_ERROR, "memmory allocation failed.");
            ret = -1;
            sqlite3_finalize(select);
            goto end;
        }

        if(!strcmp(oldtags, STEP13_NA)) oldtags[0] = '\0';
        if(newtags!=STEP13_NA){

            insert_tag(h, STEP13_MAIN_TAG_LIST_TABLE, newtags);

            s13_merge_exparts(  oldtags,
                                newtags,
                                0,
                                STEP13_LIST_DELIM,
                                0,
                                tags);
        }

        _deb6("old tags = %s, new tags = %s, merged as %s", oldtags, newtags, tags);

        ret = run_query2( h, 1, STEP13_MAIN_PATH_LIST_TABLE,
                    STEP13_UPDATE_PATH_COMMENT_TAG, "sss",
                    comment,
                    tags,
                    path
                    );

        if(ret){
            _deb5("fails here!");
            sqlite3_finalize(select);
            goto end;
        }

        h->qsize += st->st_size;

        if(!strcmp(p13_type_str(st), P13_TYPE_DIR)) h->dirs++;
        else if(!strcmp(p13_type_str(st), P13_TYPE_FILE)) h->files++;
        else h->others++;

        lineout(h, STEP13_INFO, "updated \'%s\'.", path);

        if(newtags!=STEP13_NA) free(tags);
        sqlite3_finalize(select);

    } else {
        ret = create_list_enum_callback(h, st, path, insert_stmt);
        _deb5("reached here for %s and ret = %i", path, ret);
    }

end:
    _deb5("returns: %i", ret);
    return ret;

}

int insert_record_by_path(  struct step13* h, char* table,
                            char* path, int recursive){

    char tmpbuf[MAXPATHNAME], *pathbuf;
    struct stat st;
    int ret = 0;
    sqlite3_stmt* insert_stmt;

    if(!p13_is_abs_path(path)){
        if(p13_get_abs_path(path, tmpbuf)){
            if(!p13_merge_path(h->sw_arg[STEP13_SW_ROOT], tmpbuf, tmpbuf)){
                lineout(h, STEP13_ERROR, "failed to merge path.");
                return -1;
            }
        }
    } else strcpy(tmpbuf, path);

    if(stat(tmpbuf, &st) == -1){
        lineout(h, STEP13_ERROR, "failed to get path info for \'%s\'.", tmpbuf);
        return -1;
    }

    //pathbuf = p13_get_unix_path(tmpbuf);
    pathbuf = tmpbuf;

    snprintf(h->sql, MAXSQL, STEP13_INSERT_PATH_DATA, table);

    if(sqlite3_prepare_v2(h->db, h->sql, strlen(h->sql), &insert_stmt, NULL) != SQLITE_OK){
        lineout(h, STEP13_ERROR, "failed to precompile sql statement \'%s\', dbms says: %s", h->sql, sqlite3_errmsg(h->db));
        ret = -1;
        goto end;
    }

    //this is not a callback! but the function does the job
    switch(create_list_if_not_exists_enum_callback(h, &st,
                                                pathbuf,
                                                insert_stmt)){
        case 0://success
        lineout(h, STEP13_INFO, "added \"%s\";", pathbuf);
        break;
        case 1://warning
        lineout(h, STEP13_WARNING, "not added \"%s\";", pathbuf);
        break;
        default://error
            return -1;
        break;
    }

    if(recursive && !strcmp(p13_type_str(&st), P13_TYPE_DIR)){

        sqlite3_reset(insert_stmt);

        switch(enum_dir(h, pathbuf, &create_list_if_not_exists_enum_callback, insert_stmt)){
            case 0:
            lineout(h, STEP13_INFO, "done enumurating.");
            ret = 0;
            break;
            default:
            lineout(h, STEP13_INFO, "enumuration failed.");
            ret = -1;
            break;
        }
    }

end:
    sqlite3_finalize(insert_stmt);
    return ret;
}

int rm_record_by_path(  struct step13* h, char* table,
                        char* path, int recursive){

}

int query(struct step13* h){

    lineout(h, STEP13_INFO, "running query...");

    h->run_query_fn = &show_row;

    switch(run_query(h, 1)){
        case 0:
        lineout(h, STEP13_INFO, "query run successfully.");
        return 0;
        break;
        default:
        lineout(h, STEP13_ERROR, "failed to run query.");
        return -1;
        break;
    }

    return 0;
}

int compare_source_and_root_path_tables(struct step13* h){

    sqlite3_stmt* insert;

    begin_trans(h);

    if(copy_table(  h,
                    STEP13_SELECT_ALL,
                    STEP13_MAIN_PATH_LIST_TABLE,
                    STEP13_COMPARE_PATH_LIST_TABLE
                    )){
        finish_trans(h);
        return -1;
    }

    if(run_query2(  h,
                    1,
                    STEP13_COMPARE_PATH_LIST_TABLE,
                    STEP13_SET_STAT,
                    "s",
                    STEP13_STAT_REMOVE)){
        finish_trans(h);
        return -1;
    }

    snprintf(h->sql, MAXSQL, STEP13_INSERT_PATH_DATA,
             STEP13_COMPARE_PATH_LIST_TABLE);

    _deb1("sql: %s", h->sql);

    if(sqlite3_prepare_v2(h->db, h->sql, strlen(h->sql), &insert, NULL) != SQLITE_OK){
        finish_trans(h);
        lineout(h, STEP13_ERROR, "failed to precompile sql statement \'%s\', dbms says: %s", h->sql, sqlite3_errmsg(h->db));
        return -1;
    }    

    switch(enum_dir(h, h->sw_arg[STEP13_SW_SOURCE], &update_list_enum_callback, insert)){
            case 0:
            lineout(h, STEP13_INFO, "done enumurating.");
            sqlite3_finalize(insert);
            finish_trans(h);
            return 0;
            break;
            default:
            lineout(h, STEP13_INFO, "enumuration failed.");            
            sqlite3_finalize(insert);
            finish_trans(h);
            return -1;
            break;
    }

    sqlite3_finalize(insert);

    finish_trans(h);

    return 0;
}

int begin_trans(struct step13* h){
    if(sqlite3_exec(h->db, "BEGIN TRANSACTION;", NULL, NULL, NULL) != SQLITE_OK) return -1;
    return 0;
}

int finish_trans(struct step13* h){
    if(sqlite3_exec(h->db, "COMMIT TRANSACTION;", NULL, NULL, NULL) != SQLITE_OK) return -1;
    return 0;
}

int rollback(struct step13* h){
    if(sqlite3_exec(h->db, "ROLLBACK TRANSACTION;", NULL, NULL, NULL) != SQLITE_OK) return -1;
    return 0;
}
