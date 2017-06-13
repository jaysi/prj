#ifndef MONETR_H
#define MONETR_H

#include "../infolink/include/infolink.h"
#include "monet_fifo.h"
#include "monet_poll.h"
#include "monet_sess.h"
#include "monet_user.h"

/*
 * This is Modular Network Relay, a library to share modules between groups of
 * users.
 *
*/

//log console elements [clog]

#define MN_CONF_CLOG_BUFSIZE 1024

typedef uint32_t mn_clogid_t;
#define MN_CLOGID_NONE	0x00000000
#define MN_CLOGID_CON	0xC0A6501E
#define MN_CLOGID_ALL	0xFFFFFFFF

enum mn_clog_type {
    MN_CLOG_T_EMPTY,
    MN_CLOG_T_STDOUT,
    MN_CLOG_T_ILINK,
    MN_CLOG_T_FILE,
    MN_CLOG_T_INVAL
};

struct monet_clog_entry {
    enum mn_clog_type type;
    mn_clogid_t id;
    void* ptr;
    struct monet_clog_entry* next;
};

struct monet_clog_list {
	mn_clogid_t n;
    struct monet_clog_entry* first, *last;
};

//locale
enum mn_locale_date {
	MN_DATE_EMPTY,
	MN_DATE_PERSIAN,
	MN_DATE_GREGORIAN,
	MN_DATE_INVAL
};

enum mn_locale_lang {
	MN_LANG_EMPTY,
	MN_LANG_EN,
	MN_LANG_FA,
	MN_LANG_INVAL
};

//message
enum mn_msgid_t {
	MN_MSGID_EMPTY,
	MN_MSGID_SYSTEM_INSTALLED,
	MN_MSGID_INIT_DONE,
	MN_MSGID_LOGIN_FAILED,
	MN_MSGID_LOGIN_SUCCESS,
	MN_MSGID_ACCEPTING,
	MN_MSGID_CONNECTING,
	MN_MSGID_CONNECTING_FAILED,
	MN_MSGID_CONNECTED,
	MN_MSGID_INTERNAL,
	MN_MSGID_CONSOLE_CONNECTED,
	MN_MSGID_INVAL
};

struct monet_msg {
	enum mn_msgid_t id;
	char* msg;
};

// operational modes

#define MONET_LOCALHOST "localhost"

enum monet_op_mode {
    MONET_OPMODE_EMPTY,
    MONET_OPMODE_CLIENT,
    MONET_OPMODE_SERVER,
    MONET_OPMODE_INVAL
};

typedef uint64_t monet_reqid_t;
typedef uint8_t mn_conf_flag_t;

//struct monet_poll;
//struct monet_user;
//struct monet_user_list;
//struct monet_session_array;
//struct monet_poll;
//struct monet_fifo_entry;

#define MN_CONF_FLAG_DEBUG  (0x01<<0)   //run on debug mode
#define MN_CONF_FLAG_DEF    (0x00)

//*** update def_conf in monet.c on change
struct monet_conf {

    mn_conf_flag_t flags;

    enum monet_op_mode mode;
    uid13_t max_sess;//100
    size_t sess_bulk_size;//10
    size_t max_sessname;
    size_t sess_def_pipe_read;//1024? default session thread read pipe buffer

    char* port;
    char* pipe_port;
    char* console_port;

    //path, files
    char* path;
    char* conf_file;
    char* acl_file;
    char* ctl_file;

    //access
    char* root_user;
    char* root_pass;
    char* guest_user;
    char* guest_pass;
    regid_t monet_acc_regid;

    //server
    uid13_t max_users;//1000000
    uid13_t max_sess_users;//1000
    size_t user_bulk_size;//10

    //client

    //locale
	enum mn_locale_date date_fmt;
	enum mn_locale_lang lang_fmt;

};

struct monet {

	magic13_t magic;

    struct monet_conf conf;
    struct monet_msg* msg;

    th13_sem_t* wait_sem;

    struct infolink acceptlink,
					*pipelink_recv, pipelink_send,
					*console;
					/*
						this is an ordinary link with some special behaviour,
						once someone connected as the console he'll get all
						system logs and will be given GOD ability!
					*/

    struct access13 ac;

    struct db13 ctldb;

    struct monet_mod_list mod_list;
    struct monet_clog_list clog_list;
    struct monet_user_array user_array; //the BIG user array,everyone's invited!
    struct monet_session_array sess_array;
    struct monet_poll poll;

};

#ifdef __cplusplus
    extern "C" {
#endif


#define mn_init_list(list) MACRO( (list).n = 0UL; (list).first = NULL; )

#define _mn_is_init(mn) (mn)->magic==MAGIC13_MONET?1:0

    error13_t mn_install(struct monet* mn, struct monet_conf* conf);
    error13_t mn_init(struct monet* mn, struct monet_conf* conf);
    error13_t mn_destroy(struct monet* mn);
    error13_t mn_reconf(struct monet* mn, struct monet_conf* conf);
    error13_t mn_def_conf(struct monet_conf* conf);

    /*                  server mode                 */
    error13_t mn_wait(struct monet* mn);

    /*                  client mode                 */
    //connection
    error13_t mn_connect(struct monet* mn,
                         char* username,
                         char* password,
                         char* address,
                         char* port,
                         struct monet_user **user);
    error13_t mn_disconnect(struct monet* mn);

    //session management
    error13_t mn_open_sess(struct monet* mn, char* name, mn_sessflag_t flags);
    error13_t mn_close_sess(struct monet* mn, char* name);
    error13_t mn_list_sess(struct monet* mn, char** namelist);
    error13_t mn_join_sess(struct monet* mn, char* name);
    error13_t mn_leave_sess(struct monet* mn, char* name);
    error13_t mn_pause_sess(struct monet* mn, char* name);
    error13_t mn_resume_sess(struct monet* mn, char* name);
    error13_t mn_setflag_sess(struct monet* mn, char* name, mn_sessflag_t flags);
    error13_t mn_getflag_sess(struct monet* mn, char* name, mn_sessflag_t* flags);

    //module management, for session super users
    error13_t mn_open_mod(struct monet* mn, char* sessname, char* name, mn_modflag_t flags);
    error13_t mn_close_mod(struct monet* mn, char* sessname, char* name);
    error13_t mn_pause_mod(struct monet* mn, char* sessname, char* name);
    error13_t mn_resume_mod(struct monet* mn, char* sessname, char* name);

    //request / reply
    error13_t mn_request(struct monet* mn,
                         char* sessname,
                         char* name,
                         struct monet_fifo_entry* request,
                         monet_reqid_t* id);
    error13_t mn_wait_reply(struct monet* mn,
                            monet_reqid_t id,
                            struct monet_fifo_entry* reply);
    error13_t mn_cancel(struct monet* mn, monet_reqid_t id);
    error13_t mn_undo(struct monet* mn, monet_reqid_t id);

    //logging
    error13_t mn_clog_init(struct monet* mn);
    error13_t mn_clog_add(	struct monet* mn, enum mn_clog_type type,
							mn_clogid_t id, void* ptr);
    error13_t mn_clog_rm(struct monet* mn, mn_clogid_t id);
    error13_t mn_clog(struct monet* mn, mn_clogid_t id, char* format, ...);
    char* mn_msg(struct monet* mn, enum mn_msgid_t msgid);

#ifdef __cplusplus
    }
#endif

#endif
