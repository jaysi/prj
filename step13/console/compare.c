#include "step13.h"

int compare(struct step13* h){

    lineout(h, STEP13_INFO, "comparing...")

    /*
    if(open_source_db(h)){
        lineout(h, STEP13_ERROR, "comparing failed.");
        return -1;
    }
    */

    compare_source_and_root_path_tables(h);

    //print_path_list(h, STEP13_COMPARE_PATH_LIST_TABLE);

    lineout(h, STEP13_INFO, "done comparing.");
    return 0;
}
