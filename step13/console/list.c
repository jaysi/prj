#include "step13.h"

int print_bak_list(struct step13* h){

    char path[MAXPATHNAME], bakpath[MAXPATHNAME];
    char comment[MAXCOMMENT];
    FILE* commentfile;
    int n;
    struct dirent **namelist;
    struct stat sb;

    s13_strcpy(bakpath, h->sw_arg[STEP13_SW_ROOT], MAXPATHNAME);
    p13_join_path(bakpath, STEP13_DIR, bakpath);
    p13_join_path(bakpath, STEP13_BACKUP_DIR, bakpath);

    lineout(h, STEP13_INFO, "listing backups...");


    n = scandir(bakpath, &namelist, 0, alphasort);
    lineout(h, STEP13_DEBUG, "enumurating < %s >, n = %i", bakpath, n);
    if (n < 0){
        lineout(h, STEP13_ERROR, "listing failed.");
        return n;
    } else {
        while (n--) {

            p13_merge_path(bakpath, namelist[n]->d_name, path);

            if(stat(path, &sb) != -1){
                switch (sb.st_mode & S_IFMT) {
                    case S_IFDIR:

                        s13_strcpy(comment, "NO COMMENTS", MAXCOMMENT);

                        p13_join_path(path, STEP13_BACKUP_COMMENT, path);

                        if((commentfile = fopen(path, "r"))){
                            fgets(comment, MAXCOMMENT, commentfile);
                            fclose(commentfile);
                        }

                        lineout(h, STEP13_INFO, "%s /* %s */", namelist[n]->d_name, comment);

                    break;
                    default:
                    break;
                }
            }


        }

    }

    lineout(h, STEP13_INFO, "finished listing backups.");

    return 0;

}

int list(struct step13* h){

    h->list_sign = STEP13_SIGN_LIST;

    switch(h->cmd_arg){

        case STEP13_CMDARG_ALL:
        print_path_list(h, STEP13_MAIN_PATH_LIST_TABLE);
        break;

        case STEP13_CMDARG_COMPARE:
        print_path_list(h, STEP13_COMPARE_PATH_LIST_TABLE);
        break;

        case STEP13_CMDARG_TAG:
        print_tag_list(h, STEP13_MAIN_TAG_LIST_TABLE);
        break;

        case STEP13_CMDARG_BACKUP:
        print_bak_list(h);
        break;

        default:
        lineout(h, STEP13_ERROR, "command argument not implemented.");
        break;
    }

    return 0;
}
