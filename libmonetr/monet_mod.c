#include "monet_internal.h"
#ifndef _WIN32
#include <dlfcn.h>
#else
#include <windows.h>
#endif

#define MONET_MOD_PATH  "mod"
#ifndef _WIN32
#define MONET_MOD_EXT   ".so"
#else
#define MONET_MOD_EXT   ".dll"
#endif

#ifndef _WIN32
#define dl_open dlopen
#define dl_sym  dlsym
#define dl_close dlclose
#else
#define dl_open(name, flags) LoadLibrary(name)
#define dl_sym  GetProcAddress
#define dl_close CloseHandle
#define RTLD_LAZY 0
#endif

//error13_t _mn_lock_modnamelist(struct monet* mn){
//    //TODO
//    return e13_error(E13_IMPLEMENT);
//}

//error13_t _mn_unlock_modnamelist(struct monet* mn){
//    //TODO
//    return e13_error(E13_IMPLEMENT);
//}

struct monet_mod_fn_name mn_mod_fn_name[] = {
	{""},
    {"mnmod_check"},
    {"mnmod_open"},
    {"mnmod_init"},
    {"mnmod_deinit"},
    {"mnmod_close"},
    {"mnmod_pause"},
    {"mnmod_resume"},
    {"mnmod_undo"},
    {"mnmod_redo"},
    {"mnmod_proc"},
    {NULL}
};

int _mn_select_mod(char* filename){
    if(!strcmp(p13_get_ext(filename), MONET_MOD_EXT)){
        return true_;
    }
    return false_;
}

error13_t _mn_init_mod_list(struct monet* mn){
	mn->mod_list.first = NULL;
	mn->mod_list.n = 0UL;
	return E13_OK;
}

error13_t _mn_register_all_mods(struct monet* mn, int rebuild){

    char base[MAXPATHNAME], path[MAXPATHNAME];
    struct dirent** namelist;
    int n;
    struct monet_mod* mod;
    mn_mod_handle handle;
    error13_t (*csum)(void);

    n = scandir(p13_join_path(mn->conf.path, MONET_MOD_PATH, base),
                &namelist,
                _mn_select_mod,
                alphasort);

    if(n < 0){
        return e13_error(E13_SYSE);
    }

    //TODO: lock

    while(n--){
        handle = dl_open(p13_join_path(base, namelist[n]->d_name, path),
						 RTLD_LAZY);
        csum = (error13_t*)(dl_sym(handle, mn_mod_fn_name[MN_MOD_CHECK_FN].name));
        if(csum && csum() == E13_OK){

            //mn->modnamelist[i] = namelist[n];
            //i++;

            if(!rebuild){
					_mn_add_mod_ptr(mn, namelist[n]->d_name, handle);
            } else {
					if(_mn_find_mod_ptr(mn, namelist[n]->d_name) != E13_OK){
						_mn_add_mod_ptr(mn, namelist[n]->d_name, handle);
					}
            }

        } else {
            free(namelist[n]);
        }
    }

    //TODO: unlock

    free(namelist);

}

error13_t _mn_register_mod(struct monet* mn, char* name, int rebuild){

	char base[MAXPATHNAME], path[MAXPATHNAME];
	struct monet_mod* mod;
    mn_mod_handle handle;
    error13_t (*csum)(void);

	handle = dl_open(p13_join_path(base, name, path), RTLD_LAZY);
	csum = (error13_t*)(dl_sym(handle, mn_mod_fn_name[MN_MOD_CHECK_FN].name));
	if(csum && csum() == E13_OK){

		//mn->modnamelist[i] = namelist[n];
		//i++;

		if(!rebuild){
				_mn_add_mod_ptr(mn, name, handle);
		} else {
				if(_mn_find_mod_ptr(mn, name) != E13_OK){
					_mn_add_mod_ptr(mn, name, handle);
				}
		}

		return E13_OK;

	}

    return e13_error(E13_IMPLEMENT);
}

error13_t _mn_add_mod_ptr(struct monet* mn, char* name, mn_mod_handle handle){

	struct monet_mod* mod;

	mod = (struct monet_mod*)m13_malloc(sizeof(struct monet_mod));
	if(!mod){
			return e13_error(E13_NOMEM);
	}

	mod->handle = handle;
	mod->next = NULL;
	mod->name = s13_malloc_strcpy(name, MN_MAX_MOD_NAME);
	if(!mod->name){
		m13_free(mod);
		return e13_error(E13_NOMEM);
	}

	if(!mn->mod_list.first){
		mn->mod_list.n = 1UL;
		mn->mod_list.first = mod;
		mn->mod_list.last = mod;
		mn->mod_list.curr = NULL;
	} else {
		mn->mod_list.n++;
		mn->mod_list.last->next = mod;
		mn->mod_list.last = mod;
	}

	return E13_OK;

}

error13_t _mn_find_mod_ptr(struct monet* mn, char* name, struct monet_mod** mod){

    for(*mod = mn->mod_list.first; *mod; (*mod) = (*mod)->next) {
        if(!strcmp((*mod)->name, name)) {
			return E13_OK;
        }
    }

    return e13_error(E13_NOTFOUND);

}

error13_t _mn_rm_mod_ptr(struct monet* mn, char* name) {
	struct monet_mod* mod, *prev;
	for(mod = mn->mod_list.first; mod; mod = mod->next) {
		if(!strcmp(mod->name, name)) {

			if(mod == mn->mod_list.first) {
				mn->mod_list.first = mn->mod_list.first->next;
			} else if (mod == mn->mod_list.last) {
				mn->mod_list.last = prev;
			} else {
				prev->next = mod->next;
			}
			mn->mod_list.n--;

			//TODO: request cleanup
			//mod->mnm_cleanup(mn, );

			dl_close(mod->handle);
			m13_free(mod->name);
			m13_free(mod);

			return E13_OK;
		}
		prev = mod;
	}

	return e13_error(E13_NOTFOUND);
}
