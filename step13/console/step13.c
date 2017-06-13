#include "step13.h"

#define _deb1(f, a...) //for debugging backupdir create in linux

int set_default_dirs(struct step13* h){

    if(!(h->switches & STEP13_SW(STEP13_SW_SOURCE)))
        strcpy(h->sw_arg[STEP13_SW_SOURCE], h->sw_arg[STEP13_SW_ROOT]);

    if(!(h->switches & STEP13_SW(STEP13_SW_BACKUPDIR))){
    
        strcpy(h->sw_arg[STEP13_SW_BACKUPDIR], h->sw_arg[STEP13_SW_ROOT]);
        
        _deb1("backupdir1: %s", h->sw_arg[STEP13_SW_BACKUPDIR]);
        
        p13_join_path(  h->sw_arg[STEP13_SW_BACKUPDIR],
                        STEP13_DIR,
                        h->sw_arg[STEP13_SW_BACKUPDIR]);
                        
	_deb1("backupdir1: %s", h->sw_arg[STEP13_SW_BACKUPDIR]);

        p13_join_path(  h->sw_arg[STEP13_SW_BACKUPDIR],
                        STEP13_BACKUP_DIR,
                        h->sw_arg[STEP13_SW_BACKUPDIR]);
        _deb1("backupdir1: %s", h->sw_arg[STEP13_SW_BACKUPDIR]);
    }

    if(!(h->switches & STEP13_SW(STEP13_SW_PATH))){

        strcpy(h->sw_arg[STEP13_SW_PATH], h->sw_arg[STEP13_SW_ROOT]);
    }
    
    _deb1(	"root: %s, source: %s, backupdir: %s, path: %s",
    		h->sw_arg[STEP13_SW_ROOT],
    		h->sw_arg[STEP13_SW_SOURCE],
    		h->sw_arg[STEP13_SW_BACKUPDIR],
    		h->sw_arg[STEP13_SW_PATH]);

    return 0;

}

int set_init_vals(struct step13* h){

    enum step13_switch sw;

    h->flags = 0;

    //turn on backups by defaulth->switches = STEP13_SW(STEP13_SW_BACKUP);

    h->switches = 0x00;

    h->vlevel = 2;//info
    h->dlevel = 1;//low

    if(!(h->sw_arg = (char**)malloc(STEP13_SW_NUM*sizeof(char**)))){
        lineout(h, STEP13_ERROR, "memmory allocation failed.");
        return -1;
    }

    for(sw = STEP13_SW_VERBOSE; sw < STEP13_SW_ERROR; sw++)
        h->sw_arg[sw] = NULL;

    h->sw_arg[STEP13_SW_ROOT] = (char*)malloc(MAXPATHNAME);
    h->sw_arg[STEP13_SW_SOURCE] = (char*)malloc(MAXPATHNAME);
    h->sw_arg[STEP13_SW_BACKUPDIR] = (char*)malloc(MAXPATHNAME);
    h->sw_arg[STEP13_SW_PATH] = (char*)malloc(MAXPATHNAME);
    
/*
    h->sw_arg[STEP13_SW_ROOT] = h->sw_arg[STEP13_SW_ROOT];
    h->source = h->sw_arg[STEP13_SW_SOURCE];
    h->backup_dir = h->sw_arg[STEP13_SW_BACKUPDIR];
*/
    if(!h->sw_arg[STEP13_SW_ROOT]){
        lineout(h, STEP13_ERROR, "memmory allocation failed.");
        return -1;
    }

    p13_get_abs_path(THIS_DIR, h->sw_arg[STEP13_SW_ROOT]);
    
    set_default_dirs(h);

    h->dirs = 0UL;
    h->files = 0UL;
    h->others = 0UL;
    h->qsize = 0UL;

    h->list_sign = STEP13_SIGN_QUERY;

    h->run_query_fn = NULL;
    h->run_query_arg = NULL;

    return 0;

}

/*
replaced with p13_merge_path2
char* convert_source_to_target_path(char* sourcedir,
                                    char* source,
                                    char* target,
                                    char* buf){

    if(!strcmp(target, source)){

        strcpy(buf, target);

    } else {

        p13_merge_path( target,
                        p13_cut_from_start(source, sourcedir),
                        buf);
    }

    return buf;

}
*/
/*
void test1(){
    char sd[100], s[100], t[100], buf[100];

    printf("srcdir: ");
    scanf("%s", sd);
    printf("src: ");
    scanf("%s", s);
    printf("trgt: ");
    scanf("%s", t);

    printf("%s\n", convert_source_to_target_path(sd, s, t, buf));

}
*/

int main(int argc, char* argv[])
{

    struct step13 h;
    char sizebuf[MAXSIZEBUF];
    int date[3];

    if(set_init_vals(&h) < 0) return -1;

    if(argc < 2){
        //test1();
        help(&h, argv[0]);
        return 0;
    }

    if(parse_args(&h, argc, argv) < 0) return -1;
    
    switch(h.cmd){
    case STEP13_CMD_INIT:
    case STEP13_CMD_VERSION:
    break;
    default:
        if(open_db(&h)){
            lineout(&h, STEP13_ERROR, "could not open db.");
            return -1;
        }
    }

    switch(h.cmd){

        case STEP13_CMD_INIT:

            init(&h);

        break;

        case STEP13_CMD_LIST:

            list(&h);

        break;

        case STEP13_CMD_COMPARE:

            compare(&h);

        break;

        case STEP13_CMD_ADD:
        case STEP13_CMD_RM:

            add_remove(&h);

        break;

        case STEP13_CMD_QUERY:

            query(&h);

        break;

        case STEP13_CMD_UPDATE:

            update(&h);

        break;

        case STEP13_CMD_COMMIT:

            commit(&h);

        break;

        case STEP13_CMD_VERSION:

            version(&h);

        break;

        default:
            lineout(&h, STEP13_ERROR, "Unknown command \'%s\'.", argv[1]);
            return -1;
        break;
    }

    d13_today(date);
    lineout(&h, STEP13_INFO, "Total query size \"%s\";",
            get_size(h.qsize, sizebuf));
    lineout(&h, STEP13_INFO, "%i Files, %i Directories, %i Others;",
            h.files, h.dirs, h.others);

    lineout(&h, STEP13_INFO, "Query finished in %i/%i/%i;",
            date[0], date[1], date[2]
            );
/*
    d13_j2g(date[0], date[1], date[2], date);

    lineout(&h, STEP13_INFO, "Query finished in gregorian %i/%i/%i;",
            date[0], date[1], date[2]
            );
*/
//    d13_test();

    sqlite3_close(h.db);

    return 0;
}
