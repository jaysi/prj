#ifndef STEP13_H
#define STEP13_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "../../sqlite/sqlite3.h"
#include "../../lib13/include/lib13.h"
#include "args.h"
#include "db.h"
#include "ui.h"

#define PROGNAME    "step13"
#define PROGTITLE    "StepperMotor13"

#define VERSION     "0.0.1 alpha"

#define EMPTY_DIR   ""
#define STEP13_LIST_DELIM ";"

#define STEP13_DIR      ".motor13"
#define STEP13_BACKUP_DIR      "bak"
#define STEP13_BACKUP_COMMENT  ".step13-comment"
#define MAIN_DB         "sm13.db"

#ifndef __WIN32
#define MKDIRPERM	S_IRWXU|S_IRWXG|S_IRWXO
#endif

//tables
#define STEP13_NA "n/a"

#define STEP13_STAT_ADD       "ADDED"
#define STEP13_STAT_REMOVE    "REMOVED"
#define STEP13_STAT_UPDATE    "UPDATED"
#define STEP13_STAT_SYNC      "SYNCHRONISED"
#define STEP13_STAT_DUP       "DUPLICATED"

#define STEP13_SIGN_ADD       "++ "
#define STEP13_SIGN_REMOVE    "-- "
#define STEP13_SIGN_UPDATE    "UP "
#define STEP13_SIGN_SYNC      "SY "
#define STEP13_SIGN_DUP       "XX "
#define STEP13_SIGN_LIST      "LI "
#define STEP13_SIGN_TAG       "TG "
#define STEP13_SIGN_BACKUP    "BK "
#define STEP13_SIGN_QUERY     "?? "

#define STEP13_LOGIC_NONE     (0x00)
#define STEP13_LOGIC_AND      (0x01<<0)
#define STEP13_LOGIC_OR       (0x01<<1)
#define STEP13_LOGIC_NOT      (0x01<<2)

//limits
#define MAXSQL      1024
#define MAXHASH     128 //sha-1
#define MAXSIZEBUF  256
#define MAXTAGS     1024
#define MAXCOMMENT  1024
#define MAXTAG      50
#define MAXPATHTYPE 20
#define MAXNUM      20
#define MAXTABLENAME 20
#define MAXSWITCHARG 256

struct step13 {

    short flags;
    short switches;

    int vlevel;//verbose level;
    int dlevel;//detail level;

    enum step13_cmd_code cmd, cmd_arg;

    //these are absolute path

    //char* root;

    //switch args
    char sw_logic[STEP13_SW_NUM];
    char** sw_arg;//[STEP13_SW_NUM][MAXSWITCHARG];

    sqlite3* db;
    sqlite3_stmt* stmt;

    char sql[MAXSQL];

    size_t files, dirs, others;//number of TYPE in query
    size_t qsize;//query size

    char* list_sign;

    int(*run_query_fn)(void* h);
    void* run_query_arg;
    size_t nresults;

};

#ifdef __cplusplus
    extern "C" {
#endif

int init(struct step13* h);
int list(struct step13* h);
int compare(struct step13* h);
int add_remove(struct step13* h);
int query(struct step13* h);
int update(struct step13* h);
int backup(struct step13* h);
int commit(struct step13* h);
char* convert_source_to_target_path(char* sourcedir,
                                    char* source,
                                    char* target,
                                    char* buf);
int version(struct step13* h);
int set_default_dirs(struct step13* h);

#ifdef __cplusplus
    }
#endif


#endif // STEP13_H
