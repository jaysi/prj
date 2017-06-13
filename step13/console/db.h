#ifndef DB_H
#define DB_H

struct step13;

#define SQLWHEREBUFCHUNK    1024

#define STEP13_MAIN_PATH_LIST_TABLE "main"
#define STEP13_MAIN_TAG_LIST_TABLE "tags"
#define STEP13_COMPARE_PATH_LIST_TABLE "compare"

#define STEP13_MAIN_DB      "main"
#define STEP13_SOURCE_DB    "source"

#define STEP13_INSERT_FMT "ssiiisssis"

#define STEP13_CREATE_PATH_LIST "CREATE TABLE IF NOT EXISTS %s(path TEXT, type TEXT, ctime INT, mtime INT, size INT, hash TEXT, comment TEXT, tags TEXT, level INT, stat TEXT);"
#define STEP13_INSERT_PATH_DATA "INSERT INTO %s(path, type, ctime, mtime, size, hash, comment, tags, level, stat) VALUES(?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10);"
#define STEP13_SELECT_ALL "SELECT * FROM %s;"
#define STEP13_SELECT_EXISTS "SELECT * FROM %s WHERE stat NOT LIKE \'REMOVED\';"
#define STEP13_SELECT_BY_PATH "SELECT * FROM %s WHERE path LIKE ?1;"
#define STEP13_UPDATE_PATH_COMMENT_TAG "UPDATE %s SET comment=?1, tags=?2 WHERE path LIKE ?3;"
#define STEP13_EMPTY_TABLE "DELETE FROM %s;"
#define STEP13_CREATE_TAG_LIST "CREATE TABLE IF NOT EXISTS %s(tags TEXT);"
#define STEP13_SELECT_BY_TAG "SELECT * FROM %s WHERE tags LIKE ?1;"
#define STEP13_INSERT_TAG "INSERT INTO %s(tags) VALUES(?1);"
#define STEP13_DELETE_TABLE "DROP TABLE %s;"
#define STEP13_CREATE_COPY_TABLE "CREATE TABLE %s AS (SELECT * FROM %s);"
#define STEP13_COPY_TABLE "INSERT INTO %s(SELECT * FROM %s);"
#define STEP13_ATTACH_DB "ATTACH DATABASE ?1 AS %s;"
#define STEP13_SET_STAT "UPDATE %s SET stat=?1;"
#define STEP13_SET_STAT_BY_PATH "UPDATE %s SET size=?1, mtime=?2, ctime=?3, stat=?4 WHERE path LIKE ?5;"
#define STEP13_SELECT_BY_TYPE_STAT "SELECT * FROM %s WHERE(type LIKE ?1 AND stat LIKE ?2);"
#define STEP13_SELECT_NEW "SELECT * FROM %s WHERE(type LIKE ?1 AND (stat LIKE ?2 OR stat LIKE ?3));"

enum step13_colid{
    STEP13_COLID_PATH = 0,
    STEP13_COLID_TYPE,
    STEP13_COLID_CTIME,
    STEP13_COLID_MTIME,
    STEP13_COLID_SIZE,
    STEP13_COLID_HASH,
    STEP13_COLID_COMMENT,
    STEP13_COLID_TAG,
    STEP13_COLID_LEVEL,
    STEP13_COLID_STAT
};

#define STEP13_BIND_COLID(id) ((id)+1)

#ifdef __cplusplus
    extern "C" {
#endif

int open_db(struct step13* h);
int open_source_db(struct step13* h);
int create_main_list(struct step13* h, int create_table);
int create_main_tag_list(struct step13* h);
int print_path_list(struct step13* h, char* table);
int print_tag_list(struct step13* h, char* table);
int delete_table(struct step13* h, char* table);
int empty_table(struct step13* h, char* table);
int copy_table( struct step13* h,
                char* select_sql,
                char* source,
                char* target);
int create_table(struct step13* h, char* table, char* basesql);

int insert_record_by_path(  struct step13* h, char* table,
                            char* path, int recursive);

int insert_tag(struct step13* h, char* table, char* tag);

int rm_record_by_path(  struct step13* h,
                        char* table,
                        char* path,
                        int recursive);

int is_path_exists( struct step13* h,
                    char* table,
                    char* path);
int compare_source_and_root_path_tables(struct step13* h);

int run_query(  struct step13* h,
                int do_cleanup);

int run_query2( struct step13* h,
                int do_cleanup,
                char* table,
                char* basesql,
                char* bind_fmt,
                ...
                );

int begin_trans(struct step13* h);
int finish_trans(struct step13* h);
int rollback(struct step13* h);

#ifdef __cplusplus
    }
#endif

#endif // DB_H
