#include "monet_internal.h"

#define MN_TABLE_CTL_MODLIST    "modlist"
#define MN_TABLE_CTL_MODLIST_COLS   5

#define _deb_def _DebugMsg

error13_t _mn_init_ctldb(struct monet* mn){

	error13_t ret;

    if(db_isinit(&mn->ctldb)) return e13_error(E13_MISUSE);

    if((ret = db_open(&mn->ctldb, NULL, NULL, NULL, NULL, mn->conf.ctl_file))
            != E13_OK){

        return ret;
    }

    _deb_def("init %s", MN_TABLE_CTL_MODLIST);

    if((ret = db_define_table(  &mn->ctldb,
                                MN_TABLE_CTL_MODLIST,
                                MN_TABLE_CTL_MODLIST,
                                DB_TABLEF_CORE|DB_TABLEF_ENCRYPT
                                |DB_TABLEF_MALLOC,
                                MN_TABLE_CTL_MODLIST_COLS,

                                "RegID",
                                DB_T_INT,
                                "شماره ثبت",
                                "",
                                DB_COLF_HIDE|DB_COLF_AUTO,

                                "owneruid",
                                DB_T_BIGINT,
                                "مالک",
                                "",
                                0,

                                "id",
                                DB_T_INT,
                                "شناسه",
                                "",
                                0,

                                "path",
                                DB_T_TEXT,
                                "مسیر",
                                "",
                                0,

                                "flags",
                                DB_T_INT,
                                "پرچم",
                                "",
                                DB_COLF_VIRTUAL|DB_COLF_HIDE

                    )) != E13_OK){

        return e13_error(E13_NOLFS);

    }

    return E13_OK;


}

error13_t _mn_make_ctldb(struct monet* mn){
	error13_t ret;

	//1. create table
	if((ret=db_create_table(&mn->ctldb, db_get_tid_byname(
														&mn->ctldb,
														MN_TABLE_CTL_MODLIST))
		!= E13_OK)){

		return ret;

	}

	//2. create default
	struct db_stmt stmt;
	//TODO
	//db_insert(mn->ctldb, db_get_tid_byname(MN_TABLE_CTL_MODLIST), );

	return E13_OK;
}

error13_t _monet_check_perm(struct monet* mn, uid13_t uid, gid13_t gid, ilink_pkt_type_t pkttype, ilink_data_t* pktdata, ilink_datalen_t datalen){
    //TODO
    return E13_OK;
}
