#ifndef MONET_MOD_H
#define MONET_MOD_H

#include "../lib13/include/lib13.h"

#ifdef _WIN32
#include <windows.h>
#endif

struct monet;

typedef uint32_t mn_modid_t;
typedef uint16_t mn_modflag_t;
#ifndef _WIN32
typedef void* mn_mod_handle;
#else
typedef HINSTANCE mn_mod_handle;
//removed old HANDLE type
#endif

#define MN_MAX_MOD_NAME 1024

//#define MN_MOD_CHECK_FN "mnmod_check"
//#define MN_MOD_OPEN_FN  "mnmod_open"
//#define MN_MOD_INIT_FN  "mnmod_init"
//#define MN_MOD_DEINIT_FN "mnmod_deinit"
//#define MN_MOD_CLOSE_FN "mnmod_close"
//#define MN_MOD_PAUSE_FN "mnmod_pause"
//#define MN_MOD_RESUME_FN "mnmod_resume"
//#define MN_MOD_UNDO_FN "mnmod_undo"//could be implemented under the proc requests
//#define MN_MOD_REDO_FN "mnmod_redo"
//#define MN_MOD_PROC_FN  "mnmod_proc"

enum monet_mod_fn_code {
	MN_MOD_EMPTY_FN,
	MN_MOD_CHECK_FN,
	MN_MOD_OPEN_FN,
	MN_MOD_INIT_FN,
	MN_MOD_DEINIT_FN,
	MN_MOD_CLOSE_FN,
	MN_MOD_PAUSE_FN,
	MN_MOD_RESUME_FN,
	MN_MOD_UNDO_FN,
	MN_MOD_REDO_FN,
	MN_MOD_PROC_FN,
	MN_MOD_INVAL_FN
};

struct monet_mod_fn_name {
	char* name;
};

struct monet_mod {

    //char* path;
    mn_modid_t id;

    mn_mod_handle handle;		//file handle, see dl_open()

    char* name;

    //mn_h*, mn_comod_entry*, mn_fifo, void*
    int (*mnm_init) (struct monet *h, char* sess_name);
    int (*mnm_cleanup) (struct monet *h, char* sess_name);
    int (*mnm_handle_request) (struct monet *h, struct monet_fifo_entry* req);

    struct monet_mod* next;

};

struct monet_mod_list {
    mn_modid_t n;
    char* nameliststr;
    struct monet_mod* first, *last, *curr;
};

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

error13_t _mn_init_mod_list(struct monet* mn);
error13_t _mn_register_all_mods(struct monet* mn, int rebuild);
error13_t _mn_register_mod(struct monet* mn, char* name, int rebuild);
error13_t _mn_add_mod(struct monet* mn, char* name, mn_mod_handle handle);
error13_t _mn_find_mod(struct monet* mn, char* name, struct monet_mod** mod);
error13_t _mn_rm_mod(struct monet* mn, char* name);

#ifdef __cplusplus
}
#endif // __cplusplus


#endif // MONET_MOD_H
