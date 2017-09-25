#define NDEBUG
#include "include/lib13.h"

#define _is_init(ac)    ((ac)->magic == MAGIC13_AC13?true_:false_)

#define ACC_TABLE_INFO              "core"
#define ACC_TABLE_GROUP             "grp"
#define ACC_TABLE_USER              "usr"
#define ACC_TABLE_MEMBERSHIP        "memb"
#define ACC_TABLE_ACL               "acc"
#define ACC_TABLE_FREEOBJ           "objfree"

#define _ACC_FREEGIDF_INIT	(0x00)
#define _ACC_FREEGIDF_ANY	(0x01<<0)
#define _ACC_FREEUIDF_ANY	(0x01<<0)
#define _ACC_FREEUIDF_INIT	(0x00)

#define ACC_NTABLES 6

#define _deb_acc_init   _DebugMsg
#define _deb_grp_list   _DebugMsg
#define _deb_grp_add    _DebugMsg
#define _deb_get_free_gid _DebugMsg
#define _deb_grp_chk    _DebugMsg
#define _deb_grp_rm     _DebugMsg
#define _deb_usr_list   _DebugMsg
#define _deb_usr_add    _DebugMsg
#define _deb_get_free_uid _DebugMsg
#define _deb_usr_chk    _DebugMsg
#define _deb_usr_rm     _DebugMsg
#define _deb_acc_login	_DebugMsg

//CONTROL COLUMNS

//copyinfo
#define ACC_TABLE_INFO_COLS    4
//RegID, owner, masterpass(hash), FLAGS(FAKE)

//group
#define ACC_TABLE_GROUP_COLS    5
//RegID, id, name, stat, FLAGS(FAKE)

//user
#define ACC_TABLE_USER_COLS    8
//RegID, id, name, pass(hashSHA512), lastdate(jdayno in shamsi), lasttime, stat, FLAGS(FAKE)

//membership
#define ACC_TABLE_MEMBERSHIP_COLS    6
//RegID, nrow, gid, uid, stat, FLAGS(FAKE)

//acl
#define ACC_TABLE_ACL_COLS    7
//RegID, nrow, gid, uid, objid, perm, FLAGS(FAKE)struct db_stmt st;

//freeobj - free'ed object strs
#define ACC_TABLE_FREEOBJ_COLS  4
//RegID, objid, type, FLAGS(FAKE)

static inline error13_t _acc_assert(char* name, aclid13_t id, int user){
	switch(user){
	case 0:
		if(name){
				if(!strcmp(name, ACC_GROUP_ALL)) return e13_error(E13_NOTVALID);
		} else {
			if(id == GID13_ANY || id == GID13_INVAL || id == GID13_NONE)
				return e13_error(E13_NOTVALID);
		}
		break;
	default:
		if(name){
				if(!strcmp(name, ACC_USER_ALL)) return e13_error(E13_NOTVALID);
		} else {
			if(id == UID13_ANY || id == UID13_INVAL || id == UID13_NONE)
				return e13_error(E13_NOTVALID);
		}
		break;
	}
	return E13_OK;
}

error13_t acc_init(struct access13* ac, struct db13* db, regid_t regid){

    error13_t ret;

    if(!db_isopen(db)){
        return e13_error(E13_MISUSE);
    }

    if((ret = db_set_table_slots(db, ACC_NTABLES)) != E13_OK){
        return ret;
    }

    _deb_acc_init("init %s", ACC_TABLE_INFO);

    if((ret = db_define_table(  db,
                                ACC_TABLE_INFO,
                                ACC_TABLE_INFO,
                                DB_TABLEF_CORE|DB_TABLEF_ENCRYPT|DB_TABLEF_MALLOC,
                                ACC_TABLE_INFO_COLS,

                                "RegID",
                                DB_T_INT,
                                "شماره ثبت",
                                "",
                                DB_COLF_HIDE|DB_COLF_AUTO,

                                "owner",
                                DB_T_TEXT,
                                "مالک",
                                "",
                                0,

                                "masterpass",
                                DB_T_RAW,
                                "کلمه عبور",
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

    _deb_acc_init("init %s", ACC_TABLE_GROUP);

    if((ret = db_define_table(  db,
                                ACC_TABLE_GROUP,
                                ACC_TABLE_GROUP,
                                DB_TABLEF_CTL|DB_TABLEF_ENCRYPT|DB_TABLEF_MALLOC,
                                ACC_TABLE_GROUP_COLS,

                                "RegID",
                                DB_T_INT,
                                "شماره ثبت",
                                "",
                                DB_COLF_HIDE|DB_COLF_AUTO,

                                "id",
                                DB_T_INT,
                                "شناسه",
                                "",
                                DB_COLF_SINGULAR|DB_COLF_AUTO,

                                "name",
                                DB_T_TEXT,
                                "نام",
                                "",
                                DB_COLF_SINGULAR,

                                "stat",
                                DB_T_INT,
                                "وضعیت",
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

    _deb_acc_init("init %s", ACC_TABLE_USER);

    if((ret = db_define_table(  db,
                                ACC_TABLE_USER,
                                ACC_TABLE_USER,
                                DB_TABLEF_CTL|DB_TABLEF_ENCRYPT|DB_TABLEF_MALLOC,
                                ACC_TABLE_USER_COLS,

                                "RegID",
                                DB_T_INT,
                                "شماره ثبت",
                                "",
                                DB_COLF_HIDE|DB_COLF_AUTO,

                                "id",
                                DB_T_INT,
                                "شناسه",
                                "",
                                DB_COLF_SINGULAR|DB_COLF_AUTO,

                                "name",
                                DB_T_TEXT,
                                "نام",
                                "",
                                DB_COLF_SINGULAR,

                                "pass",
                                DB_T_RAW,
                                "کلمه عبور",
                                "",
                                0,

                                "lastdate",
                                DB_T_DATE,
                                "تاریخ آخرین ورود",
                                "",
                                0,

                                "lasttime",
                                DB_T_INT,
                                "ساعت آخرین ورود",
                                "",
                                0,

                                "stat",
                                DB_T_INT,
                                "وضعیت",
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

    _deb_acc_init("init %s", ACC_TABLE_MEMBERSHIP);

    if((ret = db_define_table(  db,
                                ACC_TABLE_MEMBERSHIP,
                                ACC_TABLE_MEMBERSHIP,
                                DB_TABLEF_CTL|DB_TABLEF_ENCRYPT|DB_TABLEF_MALLOC,
                                ACC_TABLE_MEMBERSHIP_COLS,

                                "RegID",
                                DB_T_INT,
                                "شماره ثبت",
                                "",
                                DB_COLF_HIDE|DB_COLF_AUTO,

                                "nrow",
                                DB_T_BIGINT,
                                "ردیف",
                                "",
                                DB_COLF_HIDE|DB_COLF_AUTO,

                                "gid",
                                DB_T_INT,
                                "گروه",
                                "@group:id>name",
                                DB_COLF_LIST|DB_COLF_TRANSL,

                                "uid",
                                DB_T_INT,
                                "کاربر",
                                "@user:id>name",
                                DB_COLF_LIST|DB_COLF_TRANSL,

                                "stat",
                                DB_T_INT,
                                "وضعیت",
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

    _deb_acc_init("init %s", ACC_TABLE_ACL);

    if((ret = db_define_table(  db,
                                ACC_TABLE_ACL,
                                ACC_TABLE_ACL,
                                DB_TABLEF_CTL|DB_TABLEF_ENCRYPT|DB_TABLEF_MALLOC,
                                ACC_TABLE_ACL_COLS,

                                "RegID",
                                DB_T_INT,
                                "شماره ثبت",
                                "",
                                DB_COLF_HIDE|DB_COLF_AUTO,

                                "nrow",
                                DB_T_BIGINT,
                                "ردیف",
                                "",
                                DB_COLF_HIDE|DB_COLF_AUTO,

                                "gid",
                                DB_T_INT,
                                "گروه",
                                "@group:id>name",
                                DB_COLF_LIST|DB_COLF_TRANSL,

                                "uid",
                                DB_T_INT,
                                "کاربر",
                                "@user:id>name",
                                DB_COLF_LIST|DB_COLF_TRANSL,

                                "objid",
                                DB_T_BIGINT,
                                "objid",
                                "",
                                0,

                                "perm",
                                DB_T_INT,
                                "دسترسی",
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

    _deb_acc_init("check db_istable_physical %s", ACC_TABLE_INFO);

    switch((ret = db_istable_physical(db, ACC_TABLE_INFO))){
    case E13_CONTINUE:
        _deb_acc_init("db_istable_physical %s CONTINUE", ACC_TABLE_INFO);
        if((ret = db_create_table(db, db_get_tid_byname(db, ACC_TABLE_INFO))) != E13_OK){
            _deb_acc_init("!db_create_table %s", ACC_TABLE_INFO);
            return ret;
        }

		//create default users and groups, TODO: handle errors better!
		ret = acc_group_add(ac, ACC_GROUP_ALL);
		if(ret == E13_OK) ret = acc_user_add(ac, ACC_USER_ALL, ACC_USER_ALL_PASSWORD);

        ret = E13_CONTINUE;
        break;
    case E13_OK:
        //TODO: the db struct must be ok! check it!
        break;
    default:
        _deb_acc_init("!db_istable_physical %s", ACC_TABLE_INFO);
        return ret;
        break;
    }

    if(ret == E13_OK){//the db struct must be ok! check it!

        if((ret = db_istable_physical(db, ACC_TABLE_GROUP)) != E13_OK){ _deb_acc_init("!db_istable_physical %s", ACC_TABLE_GROUP); return e13_error(E13_CORRUPT); }
        if((ret = db_istable_physical(db, ACC_TABLE_USER)) != E13_OK){ _deb_acc_init("!db_istable_physical %s", ACC_TABLE_USER); return e13_error(E13_CORRUPT); }
        if((ret = db_istable_physical(db, ACC_TABLE_MEMBERSHIP)) != E13_OK){ _deb_acc_init("!db_istable_physical %s", ACC_TABLE_MEMBERSHIP); return e13_error(E13_CORRUPT); }
        if((ret = db_istable_physical(db, ACC_TABLE_ACL)) != E13_OK){ _deb_acc_init("!db_istable_physical %s", ACC_TABLE_ACL); return e13_error(E13_CORRUPT); }
        //if((ret = db_istable_physical(db, ACC_TABLE_FREEOBJ)) != E13_OK){ _deb_acc_init("!db_istable_physical %s", ACC_TABLE_FREEOBJ); return e13_error(E13_CORRUPT); }

        //ok, extract regid

    } else {//everything must init.

        if((ret = db_create_table(db, db_get_tid_byname(db, ACC_TABLE_GROUP)))){ _deb_acc_init("!db_create_table %s", ACC_TABLE_GROUP); return ret; }
        if((ret = db_create_table(db, db_get_tid_byname(db, ACC_TABLE_USER)))){ _deb_acc_init("!db_create_table %s", ACC_TABLE_USER); return ret; }
        if((ret = db_create_table(db, db_get_tid_byname(db, ACC_TABLE_MEMBERSHIP)))){ _deb_acc_init("!db_create_table %s", ACC_TABLE_MEMBERSHIP); return ret; }
        if((ret = db_create_table(db, db_get_tid_byname(db, ACC_TABLE_ACL)))){ _deb_acc_init("!db_create_table %s", ACC_TABLE_ACL); return ret; }
        //if((ret = db_create_table(db, db_get_tid_byname(db, ACC_TABLE_FREEOBJ)))){ _deb_acc_init("!db_create_table %s", ACC_TABLE_FREEOBJ); return ret; }

    }

    ac->db = db;

    ac->hash = &sha256;
    ac->hashlen = SHA256_HASH_LEN;

    ac->magic = MAGIC13_AC13;

    return ret;

}

error13_t acc_set_hash(struct access13* ac, size_t hashlen, void *hashfn){
    ac->hash = hashfn;
    ac->hashlen = hashlen;

    return E13_OK;
}

/*********************************  GROUP FN  *********************************/

//return CONTINUE if an old gid reused
static inline error13_t _acc_get_free_gid(struct access13* ac, gid13_t* id,
										uint8_t flags){

    struct db_stmt st;
    struct db_logic_s logic;
    error13_t ret;
    db_table_id tid;
    db_colid_t cid;

    if(!_is_init(ac)){
        return e13_error(E13_MISUSE);
    }

    if(flags & _ACC_FREEGIDF_ANY){
		*id = GID13_ANY;
		return E13_OK;
    }

	*id = 1;

    tid = db_get_tid_byname(ac->db, ACC_TABLE_GROUP);

    _deb_get_free_gid("tid: %u", tid);

    logic.col = db_get_colid_byname(ac->db, tid, "stat");
    _deb_get_free_gid("logic.colid(stat): %u", logic.col);
    logic.comb = DB_LOGICOMB_NONE;
    logic.flags = DB_LOGICF_COL_CMP;
    logic.logic = DB_LOGIC_EQ;
    logic.ival = ACC_GRP_STT_REMOVED;

    if((ret = db_collect(ac->db, tid, NULL, 1, &logic, NULL, DB_SO_DONT, 0, &st)) != E13_OK){
        _deb_get_free_gid("collect fails %i", ret);
        return ret;
    }

    _deb_get_free_gid("collect rets %i", ret);

    cid = db_get_colid_byname(ac->db, tid, "id");

    _deb_get_free_gid("colid(id): %u", cid);

    switch((ret = db_step(&st))){
        case E13_CONTINUE:
        _deb_get_free_gid("step rets %i, CONTINUE", ret);
        ret = db_column_int(&st, cid, (int*)id);
        if(ret == E13_OK) ret = E13_CONTINUE;
        _deb_get_free_gid("col int ret %i, OK->CONTINUE", ret);
        break;
        case E13_OK:
        _deb_get_free_gid("step rets %i, OK", ret);
        break;
        default:
        _deb_get_free_gid("step rets %i, fin", ret);
        db_finalize(&st);
        return ret;
        break;
    }

    if(ret == E13_CONTINUE) return ret;

    db_finalize(&st);

    _deb_get_free_gid("collecting max id");

    //find max id
    if((ret = db_collect(ac->db, tid, NULL, 0, NULL, "id", DB_SO_DEC, 1, &st)) != E13_OK){
        _deb_get_free_gid("collecting max id fails %i", ret);
        return ret;
    }

    _deb_get_free_gid("collecting max id done");

    switch((ret = db_step(&st))){
        case E13_CONTINUE:
        _deb_get_free_gid("step CONTINUE");
        ret = db_column_int(&st, cid, (int*)id);
        (*id)++;
        if(*id == GID13_ANY) ret = e13_error(E13_FULL);
        _deb_get_free_gid("id %u, fin", *id);
        db_finalize(&st);
        break;
        case E13_OK:
        _deb_get_free_gid("step OK, fin");
        db_finalize(&st);
        break;
        default:
        _deb_get_free_gid("ret %i, fin", ret);
        db_finalize(&st);
        return ret;
        break;
    }

    return ret;
}

error13_t acc_group_set_stat(struct access13* ac, char* name, gid13_t gid, int stt){

    struct db_stmt st;
    db_table_id tid;
    int stat[1];
    struct db_logic_s logic;
    db_colid_t colid;

    if(!_is_init(ac)){
        return e13_error(E13_MISUSE);
    }

    if(_acc_assert(name, gid, 0) != E13_OK) return e13_error(E13_NOTVALID);

    tid = db_get_tid_byname(ac->db, ACC_TABLE_GROUP);

    if(name){
		logic.col = db_get_colid_byname(ac->db, tid, "name");
		logic.comb = DB_LOGICOMB_NONE;
		logic.flags = 0;
		logic.sval = name;
		logic.logic = DB_LOGIC_LIKE;
    } else {
		logic.col = db_get_colid_byname(ac->db, tid, "id");
		logic.comb = DB_LOGICOMB_NONE;
		logic.flags = DB_LOGICF_COL_CMP;
		logic.ival = gid;
		logic.logic = DB_LOGIC_EQ;
    }

    colid = db_get_colid_byname(ac->db, tid, "stat");

    stat[0] = stt;

    return db_update(ac->db, tid, logic, 1, &colid, (uchar**)&stat, NULL,&st);

}

error13_t acc_group_add(struct access13 *ac, char *name){

    struct db_stmt st;
    db_table_id tid;
    db_colid_t colids[2];
    uchar* cols[ACC_TABLE_GROUP_COLS];
    size_t size[ACC_TABLE_GROUP_COLS];
    gid13_t gid;
    error13_t ret;
    int stat;
    struct db_logic_s logic;
    struct group13 group;
    uint8_t flags = _ACC_FREEGIDF_INIT;

    if(!_is_init(ac)){
        return e13_error(E13_MISUSE);
    }

    if(strcmp(name, ACC_GROUP_ALL)){
		if(_acc_assert(name, GID13_NONE, 0) != E13_OK)
			return e13_error(E13_NOTVALID);
    } else {
    	flags |= _ACC_FREEGIDF_ANY;
    }

    tid = db_get_tid_byname(ac->db, ACC_TABLE_GROUP);
    _deb_grp_add("tid: %u", tid);
    if(tid == DB_TID_INVAL) return e13_error(E13_CORRUPT);

    //1. check for existence
    if(acc_group_chk(ac, name, GID13_NONE, &group) == E13_OK){
        _deb_grp_add("group exists");
        return e13_error(E13_EXISTS);
    }

    _deb_grp_add("group not exists");

    //2. add it

//    cols = (char**)m13_malloc(sizeof(char*)*ACC_TABLE_GROUP_COLS);
//    if(!cols) return e13_error(E13_NOMEM);

    switch((ret = _acc_get_free_gid(ac, &gid, flags))){
    case E13_CONTINUE://an old gid is re-used, update
        _deb_grp_add("reusing old id, id = %u", gid);
        stat = ACC_GRP_STT_ACTIVE;
        logic.col = db_get_colid_byname(ac->db, tid, "id");
        _deb_grp_add("logic.colid(id): %u", logic.col);
        logic.comb = DB_LOGICOMB_NONE;
        logic.ival = gid;
        logic.flags = DB_LOGICF_COL_CMP;
        logic.logic = DB_LOGIC_EQ;
        cols[0] = (uchar*)name;
        cols[1] = (uchar*)&stat;
        colids[0] = db_get_colid_byname(ac->db, tid, "name");
        colids[1] = db_get_colid_byname(ac->db, tid, "stat");
        _deb_grp_add("colids[0](name): %u", colids[0]);
        _deb_grp_add("colids[1](stat): %u", colids[1]);
        ret = db_update(ac->db, tid, logic, 2, colids, cols, NULL, &st);
        _deb_grp_add("db_update ret: %i", ret);
        db_finalize(&st);
        _deb_grp_add("re-used old id");
        return ret;
        break;
    case E13_OK://new id, insert
        _deb_grp_add("need new id");
        break;

    default:
        _deb_grp_add("ret: %i", ret);
        return ret;
        break;

    }

    stat = ACC_GRP_STT_ACTIVE;
    cols[0] = (uchar*)&ac->regid;
    cols[1] = (uchar*)&gid;
    cols[2] = (uchar*)name;
    cols[3] = (uchar*)&stat;
    cols[4] = NULL;/*(uchar*)&flags;*/
    size[0] = sizeof(regid_t);
    size[1] = sizeof(gid13_t);
    size[2] = strlen(name) + 1;
    size[3] = sizeof(int);
//    size[4] = sizeof(int);
    if((ret = db_insert(ac->db, tid, cols, size, &st)) != E13_OK){
        db_finalize(&st);
//        m13_free(cols);
        _deb_grp_add("fails here");
        return ret;
    }

    _deb_grp_add("insert ok, fin;");

    db_finalize(&st);
//    m13_free(cols);
    _deb_grp_add("done");

    return E13_OK;

}

//remove all uid record
error13_t _acc_rm_user_membership(struct access13* ac, uid13_t uid){
    struct db_stmt st;
    struct db_logic_s logic;
    error13_t ret;
    db_table_id tid;
//    struct user13 usr;
//    size_t passlen;

    if(!_is_init(ac)){
        return e13_error(E13_MISUSE);
    }

//    if((ret = acc_user_chk(ac, uid, &usr)) != E13_OK){
//		return ret;
//    }

    tid = db_get_tid_byname(ac->db, ACC_TABLE_MEMBERSHIP);
    _deb_usr_rm("got tid %u", tid);

    logic.col = db_get_colid_byname(ac->db, tid, "uid");
    logic.comb = DB_LOGICOMB_NONE;
    logic.flags = DB_LOGICF_COL_CMP;
    logic.logic = DB_LOGIC_EQ;
    logic.ival = uid;

    _deb_usr_rm("collecting...");
    if((ret = db_delete(ac->db, tid, 1, &logic, &st)) != E13_OK){
        _deb_usr_rm("fails %i", ret);
        return ret;
    }
    _deb_usr_rm("collecting done");

    switch((ret = db_step(&st))){
        case E13_CONTINUE:
        db_finalize(&st);
        ret = E13_OK;
        break;
        case E13_OK:
        _deb_usr_rm("step OK");
        db_finalize(&st);
        return e13_error(E13_NOTFOUND);
        break;
        default:
        _deb_usr_rm("step %i", ret);
        return ret;
        break;
    }

    return ret;
}

error13_t _acc_rm_group_membership(struct access13* ac, gid13_t gid){
    struct db_stmt st;
    struct db_logic_s logic;
    error13_t ret;
    db_table_id tid;
//    struct group13 grp;
//    size_t passlen;

    if(!_is_init(ac)){
        return e13_error(E13_MISUSE);
    }

//    if((ret = acc_group_chk(ac, group, &grp)) != E13_OK){
//		return ret;
//    }

    tid = db_get_tid_byname(ac->db, ACC_TABLE_MEMBERSHIP);
    _deb_grp_rm("got tid %u", tid);

    logic.col = db_get_colid_byname(ac->db, tid, "gid");
    logic.comb = DB_LOGICOMB_NONE;
    logic.flags = DB_LOGICF_COL_CMP;
    logic.logic = DB_LOGIC_EQ;
    logic.ival = gid;

    _deb_grp_rm("collecting...");
    if((ret = db_delete(ac->db, tid, 1, &logic, &st)) != E13_OK){
        _deb_grp_rm("fails %i", ret);
        return ret;
    }
    _deb_grp_rm("collecting done");

    switch((ret = db_step(&st))){
        case E13_CONTINUE:
        db_finalize(&st);
        ret = E13_OK;
        break;
        case E13_OK:
        _deb_grp_rm("step OK");
        db_finalize(&st);
        return e13_error(E13_NOTFOUND);
        break;
        default:
        _deb_grp_rm("step %i", ret);
        return ret;
        break;
    }

    return ret;
}

error13_t acc_group_rm(struct access13 *ac, char *name, gid13_t gid){

    struct db_stmt st;
    db_table_id tid;
    error13_t ret;
    int stt;
    struct db_logic_s logic;
    db_colid_t colids[1];
    uchar* cols[1];
    struct group13 group;

    if(!_is_init(ac)){
        return e13_error(E13_MISUSE);
    }

    if(_acc_assert(name, gid, 0) != E13_OK) return e13_error(E13_NOTVALID);

    tid = db_get_tid_byname(ac->db, ACC_TABLE_GROUP);
    if(tid == DB_TID_INVAL) return e13_error(E13_CORRUPT);

    //1. check for existence
    if(acc_group_chk(ac, name, gid, &group) != E13_OK){
        return e13_error(E13_NOTFOUND);
    }

    if(group.stt == ACC_GRP_STT_REMOVED) return e13_error(E13_OK);

    if((ret = _acc_rm_group_membership(ac, group.gid)) != E13_OK) return ret;

    stt = ACC_GRP_STT_REMOVED;
    logic.col = db_get_colid_byname(ac->db, tid, "id");
    _deb_grp_rm("logic.colid(id): %u", logic.col);
    logic.comb = DB_LOGICOMB_NONE;
    logic.ival = group.gid;
    _deb_grp_rm("gid: %u", group.gid);
    logic.flags = DB_LOGICF_COL_CMP;
    logic.logic = DB_LOGIC_EQ;
    cols[0] = (uchar*)&stt;
    colids[0] = db_get_colid_byname(ac->db, tid, "stat");
    ret = db_update(ac->db, tid, logic, 1, colids, cols, NULL, &st);
    _deb_grp_rm("db_update ret: %i", ret);
    db_finalize(&st);
    return ret;

}

error13_t acc_group_chk(struct access13 *ac, char *name, gid13_t gid, struct group13* group){

    struct db_stmt st;
    struct db_logic_s logic;
    error13_t ret;
    db_table_id tid;

    if(!_is_init(ac)){
        return e13_error(E13_MISUSE);
    }

    tid = db_get_tid_byname(ac->db, ACC_TABLE_GROUP);
    _deb_grp_chk("got tid %u", tid);
    if(tid == DB_TID_INVAL) return e13_error(E13_CORRUPT);

    if(name){
		logic.col = db_get_colid_byname(ac->db, tid, "name");
		_deb_grp_chk("logic.col %u", logic.col);
		logic.comb = DB_LOGICOMB_NONE;
		logic.flags = 0;
		logic.logic = DB_LOGIC_LIKE;
		logic.sval = name;
    } else {
		logic.col = db_get_colid_byname(ac->db, tid, "id");
		_deb_grp_chk("logic.col %u", logic.col);
		logic.comb = DB_LOGICOMB_NONE;
		logic.flags = DB_LOGICF_COL_CMP;
		logic.logic = DB_LOGIC_EQ;
		logic.ival = gid;
    }

    _deb_grp_chk("collecting...");
    if((ret = db_collect(ac->db, tid, NULL, 1, &logic, NULL, DB_SO_DONT, 0, &st)) != E13_OK){
        _deb_grp_chk("fails %i", ret);
        return ret;
    }
    _deb_grp_chk("collecting done");

    switch((ret = db_step(&st))){
        case E13_CONTINUE:
        _deb_grp_chk("step CONTINUE");
        ret = db_column_int(&st, db_get_colid_byname(ac->db, tid, "id"), (int*)&group->gid);
        if(ret == E13_OK){
            _deb_grp_chk("id %u", group->gid);
            ret = db_column_int(&st, db_get_colid_byname(ac->db, tid, "stat"), &group->stt);
            _deb_grp_chk("stat %i (%s)", group->stt, ret == E13_OK?"OK":"NOK");
        }
        db_finalize(&st);
        break;
        case E13_OK:
        _deb_grp_chk("step OK");
        db_finalize(&st);
        return e13_error(E13_NOTFOUND);
        break;
        default:
        _deb_grp_chk("step %i", ret);
        return ret;
        break;
    }

    return ret;
}

error13_t acc_gid_chk(struct access13 *ac, gid13_t gid, struct group13* group){

    struct db_stmt st;
    struct db_logic_s logic;
    error13_t ret;
    db_table_id tid;

    if(!_is_init(ac)){
        return e13_error(E13_MISUSE);
    }

    tid = db_get_tid_byname(ac->db, ACC_TABLE_GROUP);
    _deb_grp_chk("got tid %u", tid);
    if(tid == DB_TID_INVAL) return e13_error(E13_CORRUPT);

    logic.col = db_get_colid_byname(ac->db, tid, "id");
    _deb_grp_chk("logic.col %u", logic.col);
    logic.comb = DB_LOGICOMB_NONE;
    logic.flags = DB_LOGICF_COL_CMP;
    logic.logic = DB_LOGIC_EQ;
    logic.ival = gid;

    _deb_grp_chk("collecting...");
    if((ret = db_collect(ac->db, tid, NULL, 1, &logic, NULL, DB_SO_DONT, 0, &st)) != E13_OK){
        _deb_grp_chk("fails %i", ret);
        return ret;
    }
    _deb_grp_chk("collecting done");

    switch((ret = db_step(&st))){
        case E13_CONTINUE:
        _deb_grp_chk("step CONTINUE");
        ret = db_column_text(&st, db_get_colid_byname(ac->db, tid, "name"), NULL, (uchar**)&group->name);
        if(ret == E13_OK){
            _deb_grp_chk("id %u", group->gid);
            ret = db_column_int(&st, db_get_colid_byname(ac->db, tid, "stat"), &group->stt);
            _deb_grp_chk("stat %i (%s)", group->stt, ret == E13_OK?"OK":"NOK");
        }
        db_finalize(&st);
        break;
        case E13_OK:
        _deb_grp_chk("step OK");
        db_finalize(&st);
        return e13_error(E13_NOTFOUND);
        break;
        default:
        _deb_grp_chk("step %i", ret);
        return ret;
        break;
    }

    return ret;
}

error13_t acc_group_list(struct access13 *ac, gid13_t *n, struct group13 **group){

    struct db_stmt st;
    struct db_logic_s logic;
    error13_t ret;
    db_table_id tid;
    int stt;
    gid13_t id;
    uchar* s;
    size_t slen;
    struct group13* next, *last = NULL;
    *n = GID13_NONE;

    if(!_is_init(ac)){
        return e13_error(E13_MISUSE);
    }

    tid = db_get_tid_byname(ac->db, ACC_TABLE_GROUP);
    _deb_grp_list("got tid %u", tid);

    if((logic.col = db_get_colid_byname(ac->db, tid, "stat")) ==DB_COLID_INVAL){
		return e13_error(E13_ABORT);
    }
    _deb_grp_list("logic.col %u", logic.col);
    logic.comb = DB_LOGICOMB_NONE;
    logic.flags = DB_LOGICF_COL_CMP;
    logic.ival = ACC_GRP_STT_REMOVED;
    logic.logic = DB_LOGIC_NE;

    _deb_grp_list("collecting...");
    if((ret = db_collect(ac->db, tid, NULL, 1, &logic, "name", DB_SO_INC, 0, &st)) != E13_OK){
        _deb_grp_list("fails@here");
        return ret;
    }
    _deb_grp_list("collecting done.");

    again:
    _deb_grp_list("step.");
    switch((ret = db_step(&st))){
        case E13_CONTINUE:
        _deb_grp_list("step CONTINUE");
        ret = db_column_int(&st, db_get_colid_byname(ac->db, tid, "id"), (int*)&id);
        if(ret == E13_OK){
            _deb_grp_list("id ok, %u", id);
            ret = db_column_int(&st, db_get_colid_byname(ac->db, tid, "stat"), &stt);
        }
        if(ret == E13_OK){
            _deb_grp_list("stat ok, %i", stt);
            ret = db_column_text(&st, db_get_colid_byname(ac->db, tid, "name"), &slen, &s);
        }
        if(ret == E13_OK){
            _deb_grp_list("name ok: %s", s);
            (*n)++;
            _deb_grp_list("n: %u", *n);
            next = (struct group13*)m13_malloc(sizeof(struct group13));
            next->name = (char*)s;
            next->gid = id;
            next->stt = stt;
            next->next = NULL;
            if(!last){
                last = next;
                *group = next;
                _deb_grp_list("added as first");
            } else {
                _deb_grp_list("added as last");
                last->next = next;
                last = next;
            }
        }
        if(ret != E13_OK){
            _deb_grp_list("ret = %i, break;", ret);
            break;
        }
        goto again;
        break;
        case E13_OK:
        _deb_grp_list("step OK");
        db_finalize(&st);
        return E13_OK;
        break;
        default:
        _deb_grp_list("step %i", ret);
        return ret;
        break;
    }

    _deb_grp_list("step %i", ret);
    return ret;

}

error13_t acc_group_list_free(struct group13* group){
    struct group13* first = group, *del;

    while(first){
        del = first;
        first = first->next;
        m13_free(del->name);
        m13_free(del);
    }

    return E13_OK;
}

/*********************************  USER FN  **********************************/

//return CONTINUE if an old uid reused
error13_t _acc_get_free_uid(struct access13* ac, uid13_t* id,
										uint8_t flags){

    struct db_stmt st;
    struct db_logic_s logic;
    error13_t ret;
    db_table_id tid;
    db_colid_t cid;

    if(!_is_init(ac)){
        return e13_error(E13_MISUSE);
    }

    if(flags & _ACC_FREEUIDF_ANY){
		*id = UID13_ANY;
		return E13_OK;
    }

    *id = 1;

    tid = db_get_tid_byname(ac->db, ACC_TABLE_USER);

    _deb_get_free_uid("tid: %u", tid);

    logic.col = db_get_colid_byname(ac->db, tid, "stat");
    _deb_get_free_uid("logic.colid(stat): %u", logic.col);
    logic.comb = DB_LOGICOMB_NONE;
    logic.flags = DB_LOGICF_COL_CMP;
    logic.logic = DB_LOGIC_EQ;
    logic.ival = ACC_USR_STT_REMOVED;

    if((ret = db_collect(ac->db, tid, NULL, 1, &logic, NULL, DB_SO_DONT, 0, &st)) != E13_OK){
        _deb_get_free_uid("collect fails %i", ret);
        return ret;
    }

    _deb_get_free_uid("collect rets %i", ret);

    cid = db_get_colid_byname(ac->db, tid, "id");

    _deb_get_free_uid("colid(id): %u", cid);

    switch((ret = db_step(&st))){
        case E13_CONTINUE:
        _deb_get_free_uid("step rets %i, CONTINUE", ret);
        ret = db_column_int(&st, cid, (int*)id);
        if(ret == E13_OK) ret = E13_CONTINUE;
        _deb_get_free_uid("col int ret %i, OK->CONTINUE", ret);
        break;
        case E13_OK:
        _deb_get_free_uid("step rets %i, OK", ret);
        break;
        default:
        _deb_get_free_uid("step rets %i, fin", ret);
        db_finalize(&st);
        return ret;
        break;
    }

    if(ret == E13_CONTINUE) return ret;

    db_finalize(&st);

    _deb_get_free_uid("collecting max id");

    //find max id
    if((ret = db_collect(ac->db, tid, NULL, 0, NULL, "id", DB_SO_DEC, 1, &st)) != E13_OK){
        _deb_get_free_uid("collecting max id fails %i", ret);
        return ret;
    }

    _deb_get_free_uid("collecting max id done");

    switch((ret = db_step(&st))){
        case E13_CONTINUE:
        _deb_get_free_uid("step CONTINUE");
        ret = db_column_int(&st, cid, (int*)id);
        (*id)++;
        if(*id == UID13_ANY) ret = e13_error(E13_FULL);
        _deb_get_free_uid("id %u, fin", *id);
        db_finalize(&st);
        break;
        case E13_OK:
        _deb_get_free_uid("step OK, fin");
        db_finalize(&st);
        break;
        default:
        _deb_get_free_uid("ret %i, fin", ret);
        db_finalize(&st);
        return ret;
        break;
    }

    return ret;
}

error13_t acc_user_set_stat(struct access13* ac, char* name, uid13_t id, int stt){

    struct db_stmt st;
    db_table_id tid;
    int stat[1];
    struct db_logic_s logic;
    db_colid_t colid;

    if(!_is_init(ac)){
        return e13_error(E13_MISUSE);
    }

    if(_acc_assert(name, id, 1) != E13_OK) return e13_error(E13_NOTVALID);

    tid = db_get_tid_byname(ac->db, ACC_TABLE_USER);

    if(name){
		logic.col = db_get_colid_byname(ac->db, tid, "name");
		logic.comb = DB_LOGICOMB_NONE;
		logic.flags = 0;
		logic.sval = name;
		logic.logic = DB_LOGIC_LIKE;
    } else {
		logic.col = db_get_colid_byname(ac->db, tid, "id");
		logic.comb = DB_LOGICOMB_NONE;
		logic.flags = DB_LOGICF_COL_CMP;
		logic.ival = id;
		logic.logic = DB_LOGIC_EQ;
    }

    colid = db_get_colid_byname(ac->db, tid, "stat");

    stat[0] = stt;

    return db_update(ac->db, tid, logic, 1, &colid, (uchar**)&stat, NULL,&st);

}

error13_t acc_user_add(struct access13 *ac, char *name, char* pass){

    struct db_stmt st;
    db_table_id tid;
    db_colid_t colids[2];
    uchar* cols[ACC_TABLE_USER_COLS];
    size_t size[ACC_TABLE_USER_COLS];
    uid13_t uid;
    error13_t ret;
    int stat;
    struct db_logic_s logic;
    uchar passhash[ac->hashlen];
    uint32_t lastdate = 0UL;
    uint32_t lasttime = 0UL;
    struct user13 user;
    uint8_t flags = _ACC_FREEUIDF_INIT;

    if(!_is_init(ac)){
        return e13_error(E13_MISUSE);
    }

    if(strcmp(name, ACC_USER_ALL)){
		if(_acc_assert(name, UID13_NONE, 0) != E13_OK)
			return e13_error(E13_NOTVALID);
    }

    tid = db_get_tid_byname(ac->db, ACC_TABLE_USER);
    _deb_usr_add("tid: %u", tid);
    if(tid == DB_TID_INVAL) return e13_error(E13_CORRUPT);

    //1. check for existence
    if(acc_user_chk(ac, name, UID13_NONE, &user) == E13_OK){
        _deb_usr_add("user exists");
        m13_free(user.passhash);
        return e13_error(E13_EXISTS);
    } else {
    	flags |= _ACC_FREEUIDF_ANY;
    }

    _deb_usr_add("user not exists");

    //2. add it

//    cols = (char**)m13_malloc(sizeof(char*)*ACC_TABLE_USER_COLS);
//    if(!cols) return e13_error(E13_NOMEM);

    ac->hash((uchar*)pass, s13_strlen(pass, ACC_MAX_PASSLEN), passhash);
    //passhash[0] = 0;

    switch((ret = _acc_get_free_uid(ac, &uid, flags))){
    case E13_CONTINUE://an old uid is re-used, update
        _deb_usr_add("reusing old id, id = %u", uid);
        stat = ACC_USR_STT_OUT;
        logic.col = db_get_colid_byname(ac->db, tid, "id");
        _deb_usr_add("logic.colid(id): %u", logic.col);
        logic.comb = DB_LOGICOMB_NONE;
        logic.ival = uid;
        logic.flags = DB_LOGICF_COL_CMP;
        logic.logic = DB_LOGIC_EQ;
        cols[0] = (uchar*)name;
        cols[1] = (uchar*)passhash;
        cols[2] = (uchar*)&lastdate;
        cols[3] = (uchar*)&lasttime;
        cols[4] = (uchar*)&stat;
        colids[0] = db_get_colid_byname(ac->db, tid, "name");
        colids[1] = db_get_colid_byname(ac->db, tid, "pass");
        colids[2] = db_get_colid_byname(ac->db, tid, "lastdate");
        colids[3] = db_get_colid_byname(ac->db, tid, "lasttime");
        colids[4] = db_get_colid_byname(ac->db, tid, "stat");
        size[0] = -1;
        size[1] = ac->hashlen;
        size[2] = 0;
        size[3] = 0;
        size[4] = 0;
        _deb_usr_add("colids[0](name): %u", colids[0]);
        _deb_usr_add("colids[2](pass): %u", colids[1]);
        _deb_usr_add("colids[3](lastdate): %u", colids[2]);
        _deb_usr_add("colids[4](lasttime): %u", colids[3]);
        _deb_usr_add("colids[1](stat): %u", colids[4]);
        ret = db_update(ac->db, tid, logic, 5, colids, cols, size, &st);
        _deb_usr_add("db_update ret: %i", ret);
        db_finalize(&st);
        _deb_usr_add("re-used old id");
        return ret;
        break;
    case E13_OK://new id, insert
        _deb_usr_add("need new id");
        break;

    default:
        _deb_usr_add("ret: %i", ret);
        return ret;
        break;

    }

    stat = ACC_USR_STT_OUT;
    cols[0] = (uchar*)&ac->regid;
    cols[1] = (uchar*)&uid;
    cols[2] = (uchar*)name;
    cols[3] = passhash;
    cols[4] = (uchar*)&lastdate;
    cols[5] = (uchar*)&lasttime;
    cols[6] = (uchar*)&stat;
    cols[7] = NULL;/*(uchar*)&flags;*/
    size[0] = sizeof(regid_t);
    size[1] = sizeof(uid13_t);
    size[2] = strlen(name) + 1;
    size[3] = ac->hashlen;
    size[4] = sizeof(uint32_t);
    size[5] = sizeof(uint32_t);
    size[6] = sizeof(int);
//    size[4] = sizeof(int);
	_deb_usr_add("inserting %s...", name);
    if((ret = db_insert(ac->db, tid, cols, size, &st)) != E13_OK){
        db_finalize(&st);
//        m13_free(cols);
        _deb_usr_add("fails here");
        return ret;
    }

    _deb_usr_add("insert ok, fin;");

    db_finalize(&st);
//    m13_free(cols);
    _deb_usr_add("done");

    return E13_OK;

}

error13_t acc_user_rm(struct access13 *ac, char *name, uid13_t id){

    struct db_stmt st;
    db_table_id tid;
    error13_t ret;
    int stt;
    struct db_logic_s logic;
    db_colid_t colids[1];
    uchar* cols[1];
    struct user13 user;

    if(!_is_init(ac)){
        return e13_error(E13_MISUSE);
    }

    if(_acc_assert(name, id, 1) != E13_OK) return e13_error(E13_NOTVALID);

    tid = db_get_tid_byname(ac->db, ACC_TABLE_USER);
    if(tid == DB_TID_INVAL) return e13_error(E13_CORRUPT);

    //1. check for existence
    if(acc_user_chk(ac, name, id, &user) != E13_OK){
        return e13_error(E13_NOTFOUND);
    }

    if(user.stt == ACC_USR_STT_REMOVED) return e13_error(E13_OK);

    if((ret = _acc_rm_user_membership(ac, user.uid)) != E13_OK) return ret;

    stt = ACC_USR_STT_REMOVED;
    logic.col = db_get_colid_byname(ac->db, tid, "id");
    _deb_usr_rm("logic.colid(id): %u", logic.col);
    logic.comb = DB_LOGICOMB_NONE;
    logic.ival = user.uid;
    _deb_usr_rm("uid: %u", user.uid);
    logic.flags = DB_LOGICF_COL_CMP;
    logic.logic = DB_LOGIC_EQ;
    cols[0] = (uchar*)&stt;
    colids[0] = db_get_colid_byname(ac->db, tid, "stat");
    ret = db_update(ac->db, tid, logic, 1, colids, cols, NULL, &st);
    _deb_usr_rm("db_update ret: %i", ret);
    db_finalize(&st);
    return ret;

}

error13_t acc_user_chpass(struct access13* ac, char* name, uid13_t id,
							char* oldpass,
							char* newpass){
	struct db_stmt st;
    db_table_id tid;
    uchar* col[1];
    struct db_logic_s logic;
    db_colid_t colid;
	uchar passhash[ac->hashlen];

    if(!_is_init(ac)){
        return e13_error(E13_MISUSE);
    }

    if(_acc_assert(name, id, 1) != E13_OK) return e13_error(E13_NOTVALID);

    tid = db_get_tid_byname(ac->db, ACC_TABLE_USER);

    if(name){
		logic.col = db_get_colid_byname(ac->db, tid, "name");
		logic.comb = DB_LOGICOMB_NONE;
		logic.flags = 0;
		logic.sval = name;
		logic.logic = DB_LOGIC_LIKE;
    } else {
		logic.col = db_get_colid_byname(ac->db, tid, "id");
		logic.comb = DB_LOGICOMB_NONE;
		logic.flags = DB_LOGICF_COL_CMP;
		logic.ival = id;
		logic.logic = DB_LOGIC_EQ;
    }

    colid = db_get_colid_byname(ac->db, tid, "pass");

    ac->hash(newpass, s13_strlen(newpass, ACC_MAX_PASSLEN), passhash);

	col[0] = passhash;

    return db_update(ac->db, tid, logic, 1, &colid, col, NULL, &st);

}

error13_t acc_user_chk(struct access13 *ac, char *name, uid13_t id,
					struct user13* user){

    struct db_stmt st;
    struct db_logic_s logic;
    error13_t ret;
    db_table_id tid;
    size_t passlen;

    if(!_is_init(ac)){
        return e13_error(E13_MISUSE);
    }

    tid = db_get_tid_byname(ac->db, ACC_TABLE_USER);
    _deb_usr_chk("got tid %u", tid);

    if(name){
		logic.col = db_get_colid_byname(ac->db, tid, "name");
		_deb_usr_chk("logic.col %u", logic.col);
		logic.comb = DB_LOGICOMB_NONE;
		logic.flags = 0;
		logic.logic = DB_LOGIC_LIKE;
		logic.sval = name;
    } else {
		logic.col = db_get_colid_byname(ac->db, tid, "id");
		_deb_usr_chk("logic.col %u", logic.col);
		logic.comb = DB_LOGICOMB_NONE;
		logic.flags = DB_LOGICF_COL_CMP;
		logic.logic = DB_LOGIC_EQ;
		logic.ival = id;
    }

    _deb_usr_chk("collecting...");
    if((ret = db_collect(ac->db, tid, NULL, 1, &logic, NULL, DB_SO_DONT, 0, &st)) != E13_OK){
        _deb_usr_chk("fails %i", ret);
        return ret;
    }
    _deb_usr_chk("collecting done");

    switch((ret = db_step(&st))){
        case E13_CONTINUE:
        _deb_usr_chk("step CONTINUE");
        ret = db_column_int(&st, db_get_colid_byname(ac->db, tid, "id"), (int*)&user->uid);
        if(ret == E13_OK){
            _deb_usr_chk("id %u", user->uid);
            ret = db_column_int(&st, db_get_colid_byname(ac->db, tid, "stat"), &user->stt);
            _deb_usr_chk("stat %i (%s)", user->stt, ret == E13_OK?"OK":"NOK");
        }
        if(ret == E13_OK){
			_deb_usr_chk("checking getting passhash, colid = %u", db_get_colid_byname(ac->db, tid, "pass"));
            ret = db_column_text(&st, db_get_colid_byname(ac->db, tid, "pass"), &passlen, &user->passhash);
        }
        if(ret == E13_OK){
			_deb_usr_chk("passhash (%s)", user->passhash);
            ret = db_column_date(&st, db_get_colid_byname(ac->db, tid, "lastdate"), user->lastdate);
            _deb_usr_chk("date(j) (%i/%i/%i)", user->lastdate[0], user->lastdate[1], user->lastdate[2]);
        }
        if(ret == E13_OK){
            ret = db_column_int(&st, db_get_colid_byname(ac->db, tid, "lasttime"), (int*)&user->lasttime);
            _deb_usr_chk("lasttime %i ", user->lasttime);
        }

        db_finalize(&st);
        ret = E13_OK;
        break;
        case E13_OK:
        _deb_usr_chk("step OK");
        db_finalize(&st);
        return e13_error(E13_NOTFOUND);
        break;
        default:
        _deb_usr_chk("step %i", ret);
        return ret;
        break;
    }

    return ret;
}

error13_t acc_uid_chk(struct access13 *ac, uid13_t uid, struct user13* user){

    struct db_stmt st;
    struct db_logic_s logic;
    error13_t ret;
    db_table_id tid;
    size_t passlen;

    if(!_is_init(ac)){
        return e13_error(E13_MISUSE);
    }

    tid = db_get_tid_byname(ac->db, ACC_TABLE_USER);
    _deb_usr_chk("got tid %u", tid);

    logic.col = db_get_colid_byname(ac->db, tid, "id");
    _deb_usr_chk("logic.col %u", logic.col);
    logic.comb = DB_LOGICOMB_NONE;
    logic.flags = DB_LOGICF_COL_CMP;
    logic.logic = DB_LOGIC_EQ;
    logic.ival = uid;

    _deb_usr_chk("collecting...");
    if((ret = db_collect(ac->db, tid, NULL, 1, &logic,NULL,DB_SO_DONT,0,&st)) !=
		E13_OK){
        _deb_usr_chk("fails %i", ret);
        return ret;
    }
    _deb_usr_chk("collecting done");

    switch((ret = db_step(&st))){
        case E13_CONTINUE:
        _deb_usr_chk("step CONTINUE");
        ret = db_column_int(&st, db_get_colid_byname(ac->db, tid, "id"),
							(int*)&user->uid);
        if(ret == E13_OK){
            _deb_usr_chk("id %u", user->uid);
            ret = db_column_int(&st, db_get_colid_byname(ac->db, tid, "stat"),
								&user->stt);
            _deb_usr_chk("stat %i (%s)", user->stt, ret == E13_OK?"OK":"NOK");
        }
        if(ret == E13_OK){
			_deb_usr_chk("checking getting passhash, colid = %u",
							db_get_colid_byname(ac->db, tid, "pass"));
            ret = db_column_text(&st, db_get_colid_byname(ac->db, tid, "pass"),
								&passlen, &user->passhash);
        }
        if(ret == E13_OK){
			_deb_usr_chk("passhash (%s)", user->passhash);
            ret = db_column_date(&st,db_get_colid_byname(ac->db,tid,"lastdate"),
								user->lastdate);
            _deb_usr_chk("date(j) (%i/%i/%i)", user->lastdate[0],
						user->lastdate[1], user->lastdate[2]);
        }
        if(ret == E13_OK){
            ret = db_column_int(&st, db_get_colid_byname(ac->db,tid,"lasttime"),
								(int*)&user->lasttime);
            _deb_usr_chk("lasttime %i ", user->lasttime);
        }

        db_finalize(&st);
        ret = E13_OK;
        break;
        case E13_OK:
        _deb_usr_chk("step OK");
        db_finalize(&st);
        return e13_error(E13_NOTFOUND);
        break;
        default:
        _deb_usr_chk("step %i", ret);
        return ret;
        break;
    }

    return ret;
}

error13_t acc_user_list(struct access13 *ac, uid13_t *n, struct user13 **user){

    struct db_stmt st;
    struct db_logic_s logic;
    error13_t ret;
    db_table_id tid;
    int stt;
    uid13_t id;
    uchar* s;
    size_t slen;
    struct user13* next, *last = NULL;
    *n = UID13_NONE;

    if(!_is_init(ac)){
        return e13_error(E13_MISUSE);
    }

    tid = db_get_tid_byname(ac->db, ACC_TABLE_USER);
    _deb_usr_list("got tid %u", tid);

    logic.col = db_get_colid_byname(ac->db, tid, "stat");
    _deb_usr_list("logic.col %u", logic.col);
    logic.comb = DB_LOGICOMB_NONE;
    logic.flags = DB_LOGICF_COL_CMP;
    logic.ival = ACC_USR_STT_REMOVED;
    logic.logic = DB_LOGIC_NE;

    _deb_usr_list("collecting...");
    if((ret = db_collect(ac->db, tid, NULL,1,&logic,"name",DB_SO_INC,0,&st)) !=
		E13_OK){
        _deb_usr_list("fails@here");
        return ret;
    }
    _deb_usr_list("collecting done.");

    again:
    _deb_usr_list("step.");
    switch((ret = db_step(&st))){
        case E13_CONTINUE:
        _deb_usr_list("step CONTINUE");
        ret = db_column_int(&st, db_get_colid_byname(ac->db, tid, "id"),
							(int*)&id);
        if(ret == E13_OK){
            _deb_usr_list("id ok, %u", id);
            ret = db_column_int(&st, db_get_colid_byname(ac->db, tid, "stat"),
								&stt);
        }
        if(ret == E13_OK){
            _deb_usr_list("stat ok, %i", stt);
            ret = db_column_text(&st, db_get_colid_byname(ac->db, tid, "name"),
								&slen, &s);
        }
        if(ret == E13_OK){
            _deb_usr_list("name ok: %s", s);
            (*n)++;
            _deb_usr_list("n: %u", *n);
            next = (struct user13*)m13_malloc(sizeof(struct user13));
            next->name = (char*)s;
            next->uid = id;
            next->stt = stt;
            next->next = NULL;
            if(!last){
                last = next;
                *user = last;
                _deb_usr_list("added as first");
            } else {
                _deb_usr_list("added as last");
                last->next = next;
                last = next;
            }
        }
        if(ret != E13_OK){
            _deb_usr_list("ret = %i, break;", ret);
            break;
        }
        goto again;
        break;
        case E13_OK:
        _deb_usr_list("step OK");
        db_finalize(&st);
        return E13_OK;
        break;
        default:
        _deb_usr_list("step %i", ret);
        return ret;
        break;
    }

    _deb_usr_list("step %i", ret);
    return ret;

}

error13_t acc_user_list_free(struct user13* user){
    struct user13* first = user, *del;

    while(first){
        del = first;
        first = first->next;
        m13_free(del->name);
        m13_free(del);
    }

    return E13_OK;
}

error13_t acc_user_login(struct access13* ac, char* username, char* password,
						uid13_t* uid){

    struct db_stmt st;
    db_table_id tid;
    int stt;
    uchar** val;
    struct db_logic_s logic;
    db_colid_t colid[3];
    error13_t ret;
    struct user13 user;
    int date_[3];
    int time_;
    char now[20];
    uchar passhash[ac->hashlen];

    if(!_is_init(ac)){
        return e13_error(E13_MISUSE);
    }

    if(_acc_assert(username, UID13_NONE, 1) != E13_OK)
		return e13_error(E13_NOTVALID);

    if((ret = acc_user_chk(ac, username, UID13_NONE, &user)) != E13_OK){
        return ret;
    }

    if(user.stt == ACC_USR_STT_IN) return e13_error(E13_EXISTS);

    ac->hash(password, s13_strlen(password, ACC_MAX_PASSLEN), passhash);

    if(memcmp(passhash, user.passhash, ac->hashlen)) return e13_error(E13_AUTH);

    _deb_acc_login("login request username %s, password %s", username,password);

    tid = db_get_tid_byname(ac->db, ACC_TABLE_USER);

    logic.col = db_get_colid_byname(ac->db, tid, "name");
    logic.comb = DB_LOGICOMB_NONE;
    logic.flags = 0;
    logic.sval = username;
    logic.logic = DB_LOGIC_LIKE;

    colid[0] = db_get_colid_byname(ac->db, tid, "stat");
    colid[1] = db_get_colid_byname(ac->db, tid, "lastdate");
    colid[2] = db_get_colid_byname(ac->db, tid, "lasttime");

    stt = ACC_USR_STT_IN;

    val = m13_malloc(3*sizeof(char*));

    val[0] = (char*)&stt;
    d13_today(date_);
    val[1]=(uchar*)date_;
    d13_clock(&time_);
    val[2]=(uchar*)&time_;

    ret = db_update(ac->db, tid, logic, 1, colid, val, NULL, &st);

    db_finalize(&st);

	m13_free(val);

    return ret==E13_CONTINUE?E13_OK:ret;

}

error13_t acc_user_logout(struct access13* ac, char* username, uid13_t uid){

    struct db_stmt st;
    db_table_id tid;
    int stt;
    uchar** val;
    struct db_logic_s logic;
    db_colid_t colid[3];
    error13_t ret;
    struct user13 user;

    if(!_is_init(ac)){
        return e13_error(E13_MISUSE);
    }

    if(_acc_assert(username, UID13_NONE, 1) != E13_OK)
		return e13_error(E13_NOTVALID);

    if((ret = acc_user_chk(ac, username, UID13_NONE, &user)) != E13_OK){
        return ret;
    }

    if(user.stt != ACC_USR_STT_IN) return e13_error(E13_NOTFOUND);

    _deb_acc_login("logout request username %s, uid %u", username, uid);

    tid = db_get_tid_byname(ac->db, ACC_TABLE_USER);

    if(username){
        logic.col = db_get_colid_byname(ac->db, tid, "name");
        logic.comb = DB_LOGICOMB_NONE;
        logic.flags = 0;
        logic.sval = username;
        logic.logic = DB_LOGIC_LIKE;
    } else {
        logic.col = db_get_colid_byname(ac->db, tid, "id");
        logic.comb = DB_LOGICOMB_NONE;
        logic.flags = DB_LOGICF_COL_CMP;
        logic.ival = uid;
        logic.logic = DB_LOGIC_EQ;
    }

    colid[0] = db_get_colid_byname(ac->db, tid, "stat");

    stt = ACC_USR_STT_OUT;

    val = m13_malloc(3*sizeof(char*));

    val[0] = (char*)&stt;
//    val[1] = ;
//    val[2] = ;

    ret = db_update(ac->db, tid, logic, 1, colid, val, NULL, &st);

    db_finalize(&st);

	m13_free(val);

    return ret==E13_CONTINUE?E13_OK:ret;

}

error13_t acc_destroy(struct access13* ac){
	//TODO
	if(!_is_init(ac)) return e13_error(E13_MISUSE);
//	return db_close(ac->db);
	return E13_OK;
}

error13_t acc_user_group_list(struct access13 *ac, char *username, uid13_t uid,
							struct group13** grouplist, int resolve_gid){

    struct db_stmt st;
    struct db_logic_s logic[2];
    error13_t ret;
    db_table_id tid;
    struct user13 usr;
    struct group13* grp, *gl_last, gtmp;
    size_t passlen;

    if(!_is_init(ac)){
        return e13_error(E13_MISUSE);
    }

    if((ret = acc_user_chk(ac, username, uid, &usr)) != E13_OK){
		return ret;
    }

    tid = db_get_tid_byname(ac->db, ACC_TABLE_MEMBERSHIP);
    _deb_usr_chk("got tid %u", tid);

    logic[0].col = db_get_colid_byname(ac->db, tid, "uid");
    logic[0].comb = DB_LOGICOMB_NONE;
    logic[0].flags = DB_LOGICF_COL_CMP;
    logic[0].logic = DB_LOGIC_EQ;
    logic[0].ival = usr.uid;

    _deb_usr_chk("collecting...");
    if((ret = db_collect(ac->db, tid, NULL, 2, logic, NULL, DB_SO_DONT, 0, &st)) != E13_OK){
        _deb_usr_chk("fails %i", ret);
        return ret;
    }
    _deb_usr_chk("collecting done");

    *grouplist = NULL;
    switch((ret = db_step(&st))){
        case E13_CONTINUE:
        grp = (struct group13*)m13_malloc(sizeof(struct group13));
        grp->next = NULL;

        if(db_column_int(&st, db_get_colid_byname(ac->db, tid, "gid"), &grp->gid) != E13_OK){
            grp->gid = GID13_INVAL;
        }

        if(resolve_gid){
            if(acc_gid_chk(ac, grp->gid, &gtmp) == E13_OK){
                grp->name = s13_malloc_strcpy(gtmp.name, 0);
            }
        } else {
            grp->name = NULL;
        }

        if(!(*grouplist)) {
                *grouplist = grp;
                gl_last = grp;
        } else {
            gl_last->next = grp;
            gl_last = grp;
        }
        break;
        case E13_OK:
        _deb_usr_chk("step OK");
        db_finalize(&st);
        return E13_OK;
        break;
        default:
        _deb_usr_chk("step %i", ret);
        return ret;
        break;
    }

    return ret;
}

error13_t acc_group_user_list(struct access13 *ac, char *groupname, gid13_t gid,
							struct user13** userlist, int resolve_uid){

    struct db_stmt st;
    struct db_logic_s logic[2];
    error13_t ret;
    db_table_id tid;
    struct group13 grp;
    struct user13* usr, *ul_last, utmp;

    if(!_is_init(ac)){
        return e13_error(E13_MISUSE);
    }

    if((ret = acc_group_chk(ac, groupname, gid, &grp)) != E13_OK){
		return ret;
    }

    tid = db_get_tid_byname(ac->db, ACC_TABLE_MEMBERSHIP);
    _deb_usr_chk("got tid %u", tid);

    logic[0].col = db_get_colid_byname(ac->db, tid, "gid");
    logic[0].comb = DB_LOGICOMB_NONE;
    logic[0].flags = DB_LOGICF_COL_CMP;
    logic[0].logic = DB_LOGIC_EQ;
    logic[0].ival = grp.gid;

    _deb_usr_chk("collecting...");
    if((ret = db_collect(ac->db, tid, NULL, 2, logic, NULL,DB_SO_DONT,0,&st)) !=
		E13_OK){
        _deb_usr_chk("fails %i", ret);
        return ret;
    }
    _deb_usr_chk("collecting done");

    *userlist = NULL;
    switch((ret = db_step(&st))){
        case E13_CONTINUE:
        usr = (struct user13*)m13_malloc(sizeof(struct user13));
        usr->next = NULL;

        if(db_column_int(&st,db_get_colid_byname(ac->db,tid,"uid"),&usr->uid) !=
			E13_OK){
            usr->uid = UID13_INVAL;
        }

        if(resolve_uid){
            if(acc_uid_chk(ac, usr->uid, &utmp) == E13_OK){
                usr->name = s13_malloc_strcpy(utmp.name, 0);
            }
        } else {
            usr->name = NULL;
        }

        if(!(*userlist)) {
                *userlist = usr;
                ul_last = usr;
        } else {
            ul_last->next = usr;
            ul_last = usr;
        }
        break;
        case E13_OK:
        _deb_usr_chk("step OK");
        db_finalize(&st);
        return E13_OK;
        break;
        default:
        _deb_usr_chk("step %i", ret);
        return ret;
        break;
    }

    return ret;
}

error13_t acc_user_group_check(struct access13 *ac, char *username, uid13_t uid,
							char* group, gid13_t gid){

    struct db_stmt st;
    struct db_logic_s logic[2];
    error13_t ret;
    db_table_id tid;
    struct user13 usr;
    struct group13 grp;
    size_t passlen;

    if(!_is_init(ac)){
        return e13_error(E13_MISUSE);
    }

    if((ret = acc_user_chk(ac, username, uid, &usr)) != E13_OK){
		return ret;
    }
    if((ret = acc_group_chk(ac, group, gid, &grp)) != E13_OK){
		return ret;
    }

    tid = db_get_tid_byname(ac->db, ACC_TABLE_MEMBERSHIP);
    _deb_usr_chk("got tid %u", tid);

    logic[0].col = db_get_colid_byname(ac->db, tid, "uid");
    logic[0].comb = DB_LOGICOMB_AND;
    logic[0].flags = DB_LOGICF_COL_CMP;
    logic[0].logic = DB_LOGIC_EQ;
    logic[0].ival = usr.uid;

    logic[1].col = db_get_colid_byname(ac->db, tid, "gid");
    logic[1].comb = DB_LOGICOMB_NONE;
    logic[1].flags = DB_LOGICF_COL_CMP;
    logic[1].logic = DB_LOGIC_EQ;
    logic[1].ival = grp.gid;

    _deb_usr_chk("collecting...");
    if((ret = db_collect(ac->db, tid, NULL, 2, logic,NULL,DB_SO_DONT,0,&st)) !=
		E13_OK){
        _deb_usr_chk("fails %i", ret);
        return ret;
    }
    _deb_usr_chk("collecting done");

    switch((ret = db_step(&st))){
        case E13_CONTINUE:
        db_finalize(&st);
        ret = E13_OK;
        break;
        case E13_OK:
        _deb_usr_chk("step OK");
        db_finalize(&st);
        return e13_error(E13_NOTFOUND);
        break;
        default:
        _deb_usr_chk("step %i", ret);
        return ret;
        break;
    }

    return ret;
}

error13_t acc_user_join_group(struct access13* ac, char* username, uid13_t uid,
							char* group, gid13_t gid){

    struct db_stmt st;
    struct db_logic_s logic[2];
    error13_t ret;
    db_table_id tid;
    struct user13 usr;
    struct group13 grp;
    size_t passlen;
    char stt;
    uchar* cols[6];
    size_t size[6];

    if(!_is_init(ac)){
        return e13_error(E13_MISUSE);
    }

    if(acc_user_group_check(ac, username, uid, group, gid) == E13_OK){
		return e13_error(E13_EXISTS);
    }

    if((ret = acc_user_chk(ac, username, uid, &usr)) != E13_OK){
		return ret;
    }
    if((ret = acc_group_chk(ac, group, gid, &grp)) != E13_OK){
		return ret;
    }

    tid = db_get_tid_byname(ac->db, ACC_TABLE_MEMBERSHIP);
    if(tid == DB_TID_INVAL) return e13_error(E13_CORRUPT);
    _deb_usr_chk("got tid %u", tid);

/*	COLUMNS
                                "RegID",
                                DB_T_INT,
                                "شماره ثبت",
                                "",
                                DB_COLF_HIDE|DB_COLF_AUTO,

                                "nrow",
                                DB_T_BIGINT,
                                "ردیف",
                                "",
                                DB_COLF_HIDE|DB_COLF_AUTO,

                                "gid",
                                DB_T_INT,
                                "گروه",
                                "@group:id>name",
                                DB_COLF_LIST|DB_COLF_TRANSL,

                                "uid",
                                DB_T_INT,
                                "کاربر",
                                "@user:id>name",
                                DB_COLF_LIST|DB_COLF_TRANSL,

                                "stat",
                                DB_T_INT,
                                "وضعیت",
                                "",
                                0,

                                "flags",
                                DB_T_INT,
                                "پرچم",
                                "",
                                DB_COLF_VIRTUAL|DB_COLF_HIDE

*/

    stt = ACC_MEMS_STT_ACTIVE;
    cols[0] = (uchar*)&ac->regid;
	cols[1] = NULL;/*(uchar*)&nrow;*/
    cols[2] = (uchar*)&grp.gid;
    cols[3] = (uchar*)&usr.uid;
    cols[4] = (uchar*)&stt;
    cols[5] = NULL;/*(uchar*)&flags;*/
    size[0] = sizeof(regid_t);
//    size[1] = sizeof(int);
    size[2] = sizeof(gid13_t);
    size[3] = sizeof(uid13_t);
    size[4] = sizeof(int);
//    size[5] = sizeof(int);
    if((ret = db_insert(ac->db, tid, cols, size, &st)) != E13_OK){
        db_finalize(&st);
//        m13_free(cols);
        return ret;
    }

    db_finalize(&st);
//    m13_free(cols);
    return E13_OK;
}

error13_t acc_user_leave_group(struct access13* ac, char* username, uid13_t uid,
							char* group, gid13_t gid){

    struct db_stmt st;
    struct db_logic_s logic[2];
    error13_t ret;
    db_table_id tid;
    struct user13 usr;
    struct group13 grp;
    size_t passlen;

    if(!_is_init(ac)){
        return e13_error(E13_MISUSE);
    }

    if((ret = acc_user_chk(ac, username, uid, &usr)) != E13_OK){
		return ret;
    }
    if((ret = acc_group_chk(ac, group, gid, &grp)) != E13_OK){
		return ret;
    }

    tid = db_get_tid_byname(ac->db, ACC_TABLE_MEMBERSHIP);
    _deb_usr_chk("got tid %u", tid);

    logic[0].col = db_get_colid_byname(ac->db, tid, "uid");
    logic[0].comb = DB_LOGICOMB_AND;
    logic[0].flags = DB_LOGICF_COL_CMP;
    logic[0].logic = DB_LOGIC_EQ;
    logic[0].ival = usr.uid;

    logic[1].col = db_get_colid_byname(ac->db, tid, "gid");
    logic[1].comb = DB_LOGICOMB_NONE;
    logic[1].flags = DB_LOGICF_COL_CMP;
    logic[1].logic = DB_LOGIC_EQ;
    logic[1].ival = grp.gid;

    _deb_usr_chk("collecting...");
    if((ret = db_delete(ac->db, tid, 2, logic, &st)) != E13_OK){
        _deb_usr_chk("fails %i", ret);
        return ret;
    }
    _deb_usr_chk("collecting done");

    switch((ret = db_step(&st))){
        case E13_CONTINUE:
        db_finalize(&st);
        ret = E13_OK;
        break;
        case E13_OK:
        _deb_usr_chk("step OK");
        db_finalize(&st);
        return e13_error(E13_NOTFOUND);
        break;
        default:
        _deb_usr_chk("step %i", ret);
        return ret;
        break;
    }

    return ret;

}

error13_t _acc_perm_chk(struct access13* ac, objid13_t objid, aclid13_t aclid,
						acc_perm_t* perm, int _idtype){

	//type 1 user, else group

    struct db_stmt st;
    struct db_logic_s logic[2];
    error13_t ret;
    db_table_id tid;
    struct user13 usr;
    struct group13 grp;
    size_t passlen;
    int dbperm;

    tid = db_get_tid_byname(ac->db, ACC_TABLE_ACL);
    _deb_usr_chk("got tid %u", tid);

    logic[0].col = db_get_colid_byname(ac->db, tid, "objid");
    logic[0].comb = DB_LOGICOMB_AND;
    logic[0].flags = DB_LOGICF_COL_CMP;
    logic[0].logic = DB_LOGIC_EQ;
    logic[0].ival = objid;

	logic[1].col = db_get_colid_byname(ac->db, tid, _idtype==1?"uid":"gid");
    logic[1].comb = DB_LOGICOMB_NONE;
    logic[1].flags = DB_LOGICF_COL_CMP;
    logic[1].logic = DB_LOGIC_EQ;
    logic[1].ival = aclid;

    _deb_usr_chk("collecting...");
    if((ret = db_collect(ac->db, tid, NULL, 2, logic,NULL,DB_SO_DONT,0,&st)) !=
		E13_OK){
        _deb_usr_chk("fails %i", ret);
        return ret;
    }
    _deb_usr_chk("collecting done");

    switch((ret = db_step(&st))){
        case E13_CONTINUE:
        ret = db_column_int(&st, db_get_colid_byname(ac->db,tid,"perm"), (int*)perm);
        db_finalize(&st);
        break;
        case E13_OK:
        _deb_usr_chk("step OK");
        db_finalize(&st);
        return e13_error(E13_NOTFOUND);
        break;
        default:
        _deb_usr_chk("step %i", ret);
        return ret;
        break;
    }

    return ret;

}

error13_t _acc_load_acl(struct access13* ac, objid13_t objid,
						acc_acl_entry** list){

	acc_acl_entry* first = NULL, *last, *entry;
	struct db_stmt st;
    struct db_logic_s logic;
    error13_t ret;
    db_table_id tid;
    db_colid_t colid_perm, colid_uid, colid_gid, colid_objid;
    acc_perm_t perm;

    tid = db_get_tid_byname(ac->db, ACC_TABLE_ACL);
    _deb_usr_chk("got tid %u", tid);

    logic.col = db_get_colid_byname(ac->db, tid, "objid");
    logic.comb = DB_LOGICOMB_AND;
    logic.flags = DB_LOGICF_COL_CMP;
    logic.logic = DB_LOGIC_EQ;
    logic.ival = objid;

   _deb_usr_chk("collecting...");
    if((ret = db_collect(ac->db, tid, NULL, 1, &logic,NULL,DB_SO_DONT,0,&st))!=
		E13_OK){
        _deb_usr_chk("fails %i", ret);
        return ret;
    }
    _deb_usr_chk("collecting done");

    colid_perm = db_get_colid_byname(ac->db,tid,"perm");
    colid_uid = db_get_colid_byname(ac->db,tid,"uid");
    colid_gid = db_get_colid_byname(ac->db,tid,"gid");
    colid_objid = db_get_colid_byname(ac->db,tid,"objid");

loop:
    switch((ret = db_step(&st))){
        case E13_CONTINUE:
        if((ret = db_column_int(&st, colid_perm, (int*)&perm)) != E13_OK) break;
        if((ret = db_column_int(&st, colid_uid, (int*)&uid)) != E13_OK) break;
        if((ret = db_column_int(&st, colid_gid, (int*)&gid)) != E13_OK) break;
        if((ret = db_column_int64(&st, colid_objid, (int*)&objid)) != E13_OK)
			break;
        if(ret = E13_OK){
				if(!(entry =(acc_acl_entry*)m13_malloc(sizeof(struct acc_acl_entry)))) break;
				entry->perm = perm;
				entry->uid = uid;
				entry->gid = gid;
				entry->objid = objid;
				entry->next = NULL;
			if(!first){
				first = entry;
                last = first;
			} else {
				last->next = entry;
				last = entry;
			}
        }
		goto loop;
        break;
        case E13_OK:
        _deb_usr_chk("step OK");
        db_finalize(&st);
        return e13_error(E13_NOTFOUND);
        break;
        default:
        _deb_usr_chk("step %i", ret);
        return ret;
        break;
    }

    return ret;
}

error13_t acc_perm_user_chk(struct access13* ac, objid13_t objid,
							char* username, uid13_t uid, acc_perm_t perm){

    struct db_logic_s logic[2];
    error13_t ret;
    struct user13 usr;
    struct group13 grp;

    if(!_is_init(ac)){
        return e13_error(E13_MISUSE);
    }

    if((ret = acc_user_chk(ac, username, uid, &usr)) != E13_OK){
		return ret;
    }

    //1. load all acl entries to objid
    //2. the rules are
    //	a. if the user access is denied, fail
	//	b. if the user access is permitted, success
	//	c. if one of the groups in the user membership list is allowed, success
	//	d. fail
	//3. special users/groups: allusers/allgroups

	return e13_error(E13_IMPLEMENT);
}

