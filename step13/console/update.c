#include "step13.h"

int update(struct step13* h){

    lineout(h, STEP13_INFO, "updating \'%s\'...", h->sw_arg[STEP13_SW_ROOT]);

    if(compare(h)){
        lineout(h, STEP13_ERROR, "update failed.");
        return -1;
    }
    
    empty_table(h, STEP13_MAIN_PATH_LIST_TABLE);

    copy_table( h, STEP13_SELECT_EXISTS, STEP13_COMPARE_PATH_LIST_TABLE,
                STEP13_MAIN_PATH_LIST_TABLE);

    run_query2( h, 1, STEP13_MAIN_PATH_LIST_TABLE, STEP13_SET_STAT,
                "s", STEP13_STAT_SYNC);

    lineout(h, STEP13_INFO, "update done.");

    return 0;

}
