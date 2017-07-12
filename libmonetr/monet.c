#include "monet_internal.h"
#include <unistd.h>

#define _deb1           _NullMsg
#define _deb_ins        _NullMsg
#define _deb_wait       _NullMsg
#define _deb_wait3      _NullMsg
#define _deb_connect    _NullMsg

struct monet_conf def_conf = {

    MN_CONF_FLAG_DEF,
    MONET_OPMODE_SERVER,//mode
    1000,//max session
    10,//session buck size
    512,//max session name length in bytes
    1024,//session thread read pipe buffer size
    "64999",//port
    "64998",//internal pipe port
    "64997",//console (external pipe) port
    "monetr",//path
    "monetr.conf",//config
    "monetr.acl",//acl
    "monetr.ctl",//control
    "root",//root user
    "root",//root password
    ACC_USER_GUEST,//guest user
    "guest",//guest password
    2,//acc_regid
    1000000,//max users
    1000, //max user on a session
    10, //user buck size
    MN_DATE_PERSIAN,
    MN_LANG_EN
};

struct monet_msg msg_en[] = {
	{MN_MSGID_EMPTY, ""},
	{MN_MSGID_SYSTEM_INSTALLED, "system has been installed"},
	{MN_MSGID_INIT_DONE, "init done"},
	{MN_MSGID_LOGIN_FAILED, "user %s has failed to login (%e)"},
	{MN_MSGID_LOGIN_SUCCESS, "user %s (uid = %u) has successfully logged in"},
	{MN_MSGID_ACCEPTING, "accepting connections"},
	{MN_MSGID_CONNECTING, "connecting to %s:%s.."},
	{MN_MSGID_CONNECTING_FAILED, "connection failed"},
	{MN_MSGID_CONNECTED, "connected successfully"},
	{MN_MSGID_INTERNAL, "internal error (%e)"},
	{MN_MSGID_CONSOLE_CONNECTED, "user %s has connected to the console"},
	{MN_MSGID_INVAL, NULL}
	};

struct _mn_tmp_accept_arg {
    struct monet* mn;
    struct infolink* acceptlink;
    error13_t *pret;
};

char* mn_msg(struct monet* mn, enum mn_msgid_t msgid){
	struct monet_msg* m;
	if(msgid < MN_MSGID_EMPTY || msgid > MN_MSGID_INVAL){
			return mn->msg[MN_MSGID_INVAL].msg;
	}

	return mn->msg[msgid].msg;
}

error13_t mn_reconf(struct monet* mn, struct monet_conf* conf){

	if(!_mn_is_init(mn)) return e13_error(E13_MISUSE);

    memcpy(&mn->conf, conf, sizeof(struct monet_conf));

    return E13_OK;

}

error13_t mn_def_conf(struct monet_conf *conf){

    memcpy(conf, &def_conf, sizeof(struct monet_conf));

    return E13_OK;

}

error13_t mn_install(struct monet *mn, struct monet_conf *conf) {

    char base[MAXPATHNAME];
    char tmp[MAXPATHNAME];
    int new_install;
    error13_t ret;
    int fd;
    struct db13 db, ctldb;
    struct access13 ac;

    if(!_mn_is_init(mn)) return e13_error(E13_MISUSE);

    //1. check for previous installation

    p13_merge_path(p13_get_home(), conf->path, base);

    _deb_ins("check access to %s", base);

    if(access(base, F_OK)){
        _deb_ins("ok");
//        if(access(base, X_OK)){
//            _deb_ins("perm");
//            return e13_error(E13_PERM);
//        }
        _deb_ins("new ins");
        new_install = 1;
    } else {
        _deb_ins("old ins");
        new_install = 0;
    }

    //2. create path

    if(new_install){

        if((ret = p13_create_dir_struct(base)) != E13_OK){
            _deb_ins("%s", e13_codemsg(ret));
            return ret;
        }

    }

    //3. create files

    p13_merge_path(base, conf->conf_file, tmp);

    if(new_install){

        if((fd = creat(tmp, S_IRUSR|S_IWUSR)) == -1){
            _deb_ins("%s", "create failed");
            return e13_error(E13_SYSE);

        }

    } else {

        if(access(tmp, W_OK|R_OK)){
            _deb_ins("%s", "perm");
            return e13_error(E13_PERM);

        }

    }

    p13_merge_path(base, conf->acl_file, tmp);

    if(new_install){

    	db_init(&db, DB_DEF_DRIVER);

        if((ret = db_open(&db, NULL, NULL, NULL, NULL, conf->acl_file))
                != E13_OK){
            _deb_ins("%s", e13_codemsg(ret));
            return ret;
        }

    } else {

        if(access(tmp, W_OK|R_OK)){
            _deb_ins("%s", "perm");
            return e13_error(E13_PERM);

        }

    }

    p13_merge_path(base, conf->ctl_file, tmp);

    if(new_install){

		db_init(&ctldb, DB_DEF_DRIVER);

        if((ret = db_open(&ctldb, NULL, NULL, NULL, NULL, conf->ctl_file))
                != E13_OK){
            _deb_ins("%s", e13_codemsg(ret));
            return ret;
        }

    } else {

        if(access(tmp, W_OK|R_OK)){
            _deb_ins("%s", "perm");
            return e13_error(E13_PERM);

        }

    }

    //4. setup defaults

    if((ret = acc_init(&ac, &db, conf->monet_acc_regid)) != E13_OK){
        _deb_ins("%s", e13_codemsg(ret));
        goto end;

    }

    if((ret = acc_user_add(&ac, conf->root_user, conf->root_pass)) != E13_OK){
        _deb_ins("%s", e13_codemsg(ret));
        if(ret != e13_error(E13_EXISTS)) goto end;
    }

    if((ret = acc_user_add(&ac, conf->guest_user, conf->guest_pass)) != E13_OK){
        _deb_ins("%s", e13_codemsg(ret));
        goto end;
    }

end:
    //5. free res

    acc_destroy(&ac);
    db_close(&db);

    _deb_ins("%s %i", "return ", ret);

    mn_clog(mn, MN_CLOGID_ALL, mn_msg(mn, MN_MSGID_SYSTEM_INSTALLED));

    return ret==e13_error(E13_EXISTS)?E13_OK:ret;

}

error13_t mn_uninstall(struct monet* mn){
	if(!_mn_is_init(mn)) return e13_error(E13_MISUSE);
    _deb_ins("%s", "not implemented yet!");
    return e13_error(E13_IMPLEMENT);
}

error13_t mn_init(struct monet* mn, struct monet_conf* conf){

#ifdef _DEBUG_MUTEX
    th13_mutexattr_t attr;
#endif

	if(_mn_is_init(mn)) return e13_error(E13_MISUSE);

    if(conf){
		memcpy(&mn->conf, conf, sizeof(struct monet_conf));
    } else {
    	mn_def_conf(&mn->conf);
    }

    mn->mod_list.first = NULL;
    mn->clog_list.first = NULL;
    mn->user_array.array = NULL;
    mn->user_array.obj_root = NULL;
    mn->sess_array.array = NULL;
    mn->poll.polist.first = NULL;

    if(mn->conf.lang_fmt == MN_LANG_EN){
	    mn->msg = msg_en;
    } else {
    	mn->msg = NULL;
    }

    mn_clog_init(mn);

    mn->magic = MAGIC13_MONET;

    mn_clog(mn, MN_CLOGID_ALL, mn_msg(mn, MN_MSGID_INIT_DONE));

    return E13_OK;

}

error13_t mn_destroy(struct monet* mn){
	if(!_mn_is_init(mn)) return e13_error(E13_MISUSE);
    _deb_ins("%s", "implement");
    return e13_error(E13_IMPLEMENT);
}

error13_t _mn_login_recv(struct monet *mn, struct infolink* link){

    //TODO: TEMPORARY
    static uid13_t uid = UID13_NONE;
    size_t i;
    struct monet_user* mn_user;
    struct ilink_io ios;
    error13_t ret;
    ilink_data_t* data;
    ilink_datalen_t dlen, unamelen = MN_MAX_USERNAME, passlen = MN_MAX_PASSWORD;
    size_t pos;
    char buf[MN_MAX_LOGIN_PAYLOAD];
    char username[MN_MAX_USERNAME], password[MN_MAX_PASSWORD];
    ilink_pkt_type_t type;

    _deb_ins("%s", "TODO: TEMP");

    memset(&ios, 0, sizeof(struct ilink_io));

    ios.ctl.recv_time = MN_USER_LOGIN_TO;
    ios.maxpayload = MN_MAX_LOGIN_PAYLOAD;

    _deb_connect("recieving login...");
    _deb1("sym_crypt_MAGIC: %x", link->sym_crypt.magic);
    if((ret = ilink_recv(link, NULL, NULL, &ios)) != E13_OK){
		_deb_connect("recv failed code: %i, msg: %s", ret, e13_codemsg(ret));
		return ret;
    }
    _deb1("sym_crypt_MAGIC: %x", link->sym_crypt.magic);

    if((ret = ilink_explode_pkt(link, &data, &dlen, &type, 0)) != E13_OK){
		_deb_connect("explode_pkt failed code: %i, msg: %s", ret,
					e13_codemsg(ret));
		return ret;
    }
    _deb1("sym_crypt_MAGIC: %x", link->sym_crypt.magic);

    if(unpack13(data, dlen, "bb",
				&unamelen, username,
				&passlen, password) == ((size_t)-1)){

		_deb_connect("unpack failed, check bounds.");
		return e13_error(E13_CONSTRAINT);

    }
    _deb1("sym_crypt_MAGIC: %x", link->sym_crypt.magic);

    //truncate
    username[MN_MAX_USERNAME-1] = '\0';
    password[MN_MAX_PASSWORD-1] = '\0';

    _

//    _deb_connect("logging in %s", username);
//    if((ret = acc_user_login(&mn->ac, username, password, &uid)) != E13_OK){
//		_deb_connect("login_user failed code: %i, msg: %s", ret,
//					e13_codemsg(ret));
//		mn_clog(mn, MN_CLOGID_ALL, mn_msg(mn, MN_MSGID_LOGIN_FAILED), username, ret);
//		return ret;
//    }
//
//    //alloc + init user struct
//    mn_user = (struct monet_user*)m13_malloc(sizeof(struct monet_user));
//
//    mn_user->next = NULL;
//
//    mn_user->link = link;
//    mn_user->name = s13_malloc_strcpy(username, MN_MAX_USERNAME);
//    mn_user->uid = uid;
////    mn_user->gid_array = NULL;
////    mn_user->name = mn->conf.guest_user;//TODO: TEMP
////    mn_user->id == ++uid;//TODO: TEMP
//
//
//    th13_mutex_lock(&mn->user_array.mx);
//
//    if(!mn->user_array.array){
//        mn->user_array.array = (struct monet_user_array_entry*)m13_malloc(
//                    sizeof(struct monet_user_array_entry)*
//                    mn->conf.user_bulk_size);
//        mn->user_array.nalloc = mn->conf.user_bulk_size;
//        mn->user_array.nactive = 0UL;
//        mn->user_array.npause = 0UL;
//        for( i = 0; i < mn->conf.user_bulk_size; i++ ) {
//            mn->user_array.array[i].user = NULL;
//        }
//        i = 0;
//    } else {
//        if(mn->user_array.nalloc == mn->user_array.nactive + mn->user_array.npause){
//            mn->user_array.array = (struct monet_user_array_entry*)m13_realloc(
//                        mn->user_array.array,
//                        sizeof(struct monet_user_array_entry)*
//                        (mn->conf.user_bulk_size + mn->user_array.nalloc));
//            for(i = 0; i < mn->conf.user_bulk_size; i++){
//                mn->user_array.array[i + mn->user_array.nalloc].user = NULL;
//            }
//            i = mn->user_array.nalloc;
//            mn->user_array.nalloc += mn->conf.user_bulk_size;
//        } else {
//            for(i = 0; i < mn->user_array.nalloc; i++){
//                if(!mn->user_array.array[i].user) break;
//            }
//        }
//    }
//
//    link->ext_ctx = mn_user;
//
//	mn_user->i = i;
//    mn->user_array.array[i].user = mn_user;
//    mn->user_array.nactive++;

    mn_clog(mn, MN_CLOGID_ALL, mn_msg(mn, MN_MSGID_LOGIN_SUCCESS), username,
			uid);

    _deb1("sym_crypt_MAGIC: %x", link->sym_crypt.magic);

    _deb_connect("logged in %s", username);

    if((ret = ilink_prepare_pkt(link, NULL, 0, MN_PKT_LOGIN, 0)) != E13_OK){
		_deb_connect("prepare_pkt failed code: %i, msg: %s", ret,
					e13_codemsg(ret));
		return ret;
    }
    _deb1("sym_crypt_MAGIC: %x", link->sym_crypt.magic);

    _deb_connect("sending OK");

    if((ret = ilink_send(link, NULL, NULL, &ios)) != E13_OK){
		_deb_connect("send failed code: %i, msg: %s", ret,
					e13_codemsg(ret));
		return ret;
    }

    //do not unlock

    return E13_OK;

}

void* _mn_tmp_console(void* arg){

#ifdef _DEBUG_MUTEX
    th13_mutexattr_t attr;
#endif // _DEBUG_MUTEX
    struct _mn_tmp_accept_arg* args = (struct _mn_tmp_accept_arg*)arg;
    struct monet* mn = (struct monet*)args->mn;
    error13_t ret;

//    th13_sem_post(args->mn->wait_sem);

    if((ret = ilink_accept(args->acceptlink, &mn->console_intl)) != E13_OK){
        goto end;
    }

    //TODO: authorize user

    if((ret = ilink_poll_add(&mn->poll.polist, mn->console_intl, ILINK_POLL_RD)) != E13_OK){
        goto end;
    }

    mn_clog(mn, MN_CLOGID_ALL, mn_msg(mn, MN_MSGID_CONSOLE_CONNECTED), mn_link_username(mn, mn->console_intl));

end:
    th13_exit(&ret);
    return NULL;
}

void* _mn_tmp_accept(void* arg){

#ifdef _DEBUG_MUTEX
    th13_mutexattr_t attr;
#endif // _DEBUG_MUTEX
    struct _mn_tmp_accept_arg* args = (struct _mn_tmp_accept_arg*)arg;
    struct monet* mn = (struct monet*)args->mn;
    error13_t ret;

    th13_sem_post(args->mn->wait_sem);

    if((ret = ilink_accept(args->acceptlink, &mn->pipelink_recv)) != E13_OK){
        goto end;
    }

    //3. init poll list, add pipe link to the list, TODO
#ifdef _DEBUG_MUTEX
    _deb1("DEBUG_MUTEX ON");
    TH13_MUTEXATTR_INIT_ERRORCHECK(&attr);
    th13_mutex_init(&mn->user_array.mx, &attr);
#else
    th13_mutex_init(&mn->user_array.mx, NULL);
#endif
    if((ret = ilink_poll_init(&mn->poll.polist, 0, 0, &mn->user_array.mx)) != E13_OK){
        _deb_wait("%s", e13_codemsg(ret));
//        db_close(&acdb);
//        ilink_destroy(&pipeaccept);
        goto end;
    }

    if((ret = ilink_poll_add(&mn->poll.polist, mn->pipelink_recv, ILINK_POLL_RD)) != E13_OK){
        _deb_wait("%s", e13_codemsg(ret));
//        db_close(&acdb);
//        ilink_destroy(&pipeaccept);
        goto end;
    }

end:
    th13_exit(&ret);
    return NULL;
}

error13_t mn_wait(struct monet *mn){

    struct infolink *link, pipeaccept, conaccept;
    error13_t ret;
    struct ilink_conf conf;
    char *path;
    struct db13 acdb;
    th13_t th;
    struct _monet_poll_thread_arg poll_arg;
    struct _mn_tmp_accept_arg arg;
#ifdef _DEBUG_MUTEX
    th13_mutexattr_t attr;
#endif

	if(!_mn_is_init(mn)) return e13_error(E13_MISUSE);

    ilink_def_conf(&conf);

    conf.type = ILINK_TYPE_LISTEN;
    conf.port = mn->conf.port;

    //0. init access db

    path = m13_malloc(MAXPATHNAME);
    if(!path) return e13_error(E13_NOMEM);

    p13_join_path(p13_get_home(), mn->conf.acl_file, path);

    if((ret = db_init(&acdb, DB_DRV_SQLITE)) != E13_OK){
        _deb_wait("%s", e13_codemsg(ret));
        m13_free(path);
        return ret;
    }

    if((ret = db_open(&acdb, NULL, NULL, NULL, NULL, path)) != E13_OK){
        _deb_wait("%s", e13_codemsg(ret));
        m13_free(path);
        return ret;
    }

    m13_free(path);

    if((ret = acc_init(&mn->ac, &acdb, mn->conf.monet_acc_regid)) != E13_OK){
        _deb_wait("%s", e13_codemsg(ret));
        db_close(&acdb);
        return ret;
    }

    //1. init accept link

    if((ret = ilink_init(&mn->acceptlink, &conf)) != E13_OK){
        _deb_wait("%s", e13_codemsg(ret));
        db_close(&acdb);
        return ret;
    }

    //2. init pipe link

    conf.type = ILINK_TYPE_SERVER;
    conf.port = mn->conf.pipe_port;

    if((ret = ilink_init(&pipeaccept, &conf)) != E13_OK){
        _deb_wait("%s", e13_codemsg(ret));
        db_close(&acdb);
        return ret;
    }

//    _deb_wait2("pipeaccept->sock = %i", pipeaccept.sock);

    conf.type = ILINK_TYPE_CLIENT;

    if((ret = ilink_init(&mn->pipelink_send, &conf)) != E13_OK){
        _deb_wait("%s", e13_codemsg(ret));
        db_close(&acdb);
        ilink_destroy(&pipeaccept);
        return ret;
    }

//    _deb_wait2("pipelink_send->sock = %i", mn->pipelink_send.sock);

    //2a. connect pipe ends using a temp accept thread

    arg.mn = mn;
    arg.acceptlink = &pipeaccept;
    arg.pret = &ret;

    _deb_wait("%s", "create tmp accept");
    mn->wait_sem = m13_malloc(sizeof(th13_sem_t));
    th13_sem_init(mn->wait_sem, 0, 0);
    th13_create(&th, NULL, &_mn_tmp_accept, &arg);
    _deb_wait("%s", "waiting for semaphore...");
    th13_sem_wait(mn->wait_sem);

//    //TODO: FIX THIS, MUST WAIT FOR THREAD to accept, done via SEMs
//    #ifdef WIN32
//    _sleep(1);
//    #else
//    sleep(1);
//    #endif // WIN32

    if((ret = ilink_connect(&mn->pipelink_send, &conf)) != E13_OK){
        _deb_wait("%s", e13_codemsg(ret));
        db_close(&acdb);
        ilink_destroy(&pipeaccept);
        return ret;
    }

    //WAIT for THREAD to FINISH
    th13_join(&th, &arg.pret);

    ilink_destroy(&pipeaccept);

    //check tmp_accept thread's return value
    if(*arg.pret != E13_OK){
        _deb_wait("fails here, %s", e13_codemsg(*arg.pret));
        db_close(&acdb);
//        mn_clog(mn, MN_CLOGID_ALL, mn_msg(mn, MN_MSGID_INIT_FAILE));
        return *arg.pret;
    }

    mn->pipelink_recv->flags |= ILINK_FLAG_PIPE;
    mn->pipelink_send.flags |= ILINK_FLAG_PIPE;

    //TODO: remove this line to make it corrupt!!!
//    _deb_wait3("pipe %i send to pipe %i recv", mn->pipelink_send.sock, mn->pipelink_recv->sock);

//    //3. init poll list, add pipe link to the list, TODO
//#ifdef _DEBUG_MUTEX
//    _deb1("DEBUG_MUTEX ON");
//    TH13_MUTEXATTR_INIT_ERRORCHECK(&attr);
//    th13_mutex_init(&mn->user_array.mx, &attr);
//#else
//    th13_mutex_init(&mn->user_array.mx, NULL);
//#endif
//    if((ret = ilink_poll_init(&mn->poll.polist, 0, 0, &mn->user_array.mx)) != E13_OK){
//        _deb_wait("%s", e13_codemsg(ret));
//        db_close(&acdb);
//        ilink_destroy(&pipeaccept);
//        return ret;
//    }
//
//    if((ret = ilink_poll_add(&mn->poll.polist, mn->pipelink_recv, ILINK_POLL_RD)) != E13_OK){
//        _deb_wait("%s", e13_codemsg(ret));
//        db_close(&acdb);
//        ilink_destroy(&pipeaccept);
//        return ret;
//    }

    //2. init console link

    conf.type = ILINK_TYPE_SERVER;
    conf.port = mn->conf.console_port;

    if((ret = ilink_init(&conaccept, &conf)) != E13_OK){
        _deb_wait("%s", e13_codemsg(ret));
        db_close(&acdb);
        return ret;
    }

//    _deb_wait2("pipeaccept->sock = %i", pipeaccept.sock);

    conf.type = ILINK_TYPE_CLIENT;

    if((ret = ilink_init(&mn->console, &conf)) != E13_OK){
        _deb_wait("%s", e13_codemsg(ret));
        db_close(&acdb);
        ilink_destroy(&conaccept);
        return ret;
    }

//    _deb_wait2("pipelink_send->sock = %i", mn->pipelink_send.sock);

    //2a. connect pipe ends using a temp accept thread

    arg.mn = mn;
    arg.acceptlink = &conaccept;
    arg.pret = &ret;

    _deb_wait("%s", "create tmp accept");
    mn->wait_sem = m13_malloc(sizeof(th13_sem_t));
    th13_sem_init(mn->wait_sem, 0, 0);
    th13_create(&th, NULL, &_mn_tmp_console, &arg);
    _deb_wait("%s", "waiting for semaphore...");
    th13_sem_wait(mn->wait_sem);

//    //TODO: FIX THIS, MUST WAIT FOR THREAD to accept, done via SEMs
//    #ifdef WIN32
//    _sleep(1);
//    #else
//    sleep(1);
//    #endif // WIN32

    if((ret = ilink_connect(&mn->console, &conf)) != E13_OK){
        _deb_wait("%s", e13_codemsg(ret));
//        db_close(&acdb);
//        ilink_destroy(&pipeaccept);
//        return ret;
    }

    //WAIT for THREAD to FINISH
    th13_join(&th, &arg.pret);

    ilink_destroy(&conaccept);

    //check tmp_accept thread's return value
//    if(*arg.pret != E13_OK){
//        _deb_wait("fails here, %s", e13_codemsg(*arg.pret));
//        db_close(&acdb);
////        mn_clog(mn, MN_CLOGID_ALL, mn_msg(mn, MN_MSGID_INIT_FAILE));
//        return *arg.pret;
//    }

    mn->console_intl->flags |= ILINK_FLAG_PIPE;
    mn->console.flags |= ILINK_FLAG_PIPE;

    //4. start poll thread

    poll_arg.mn = mn;
    poll_arg.index = 0;

    _deb_wait("%s", "create poll thread");

    th13_create(&mn->poll.th, NULL, &_monet_poll_thread, &poll_arg);
	th13_sem_wait(mn->wait_sem);
    //th13_sem_destroy(mn->wait_sem); gonna need it in the monet_proc thread
    //m13_free(mn->wait_sem);

    mn_clog(mn, MN_CLOGID_ALL, mn_msg(mn, MN_MSGID_ACCEPTING));

    poll_arg.mn = mn;
    poll_arg.index = 0;

    _deb_wait("%s", "create poll thread");

    mn->req_fifo.first = NULL;
    mn->req_fifo.n = 0UL;
    th13_mutex_init(&mn->req_fifo.mx);
    mn->req_fifo.flags = MN_FIFO_FLAG_INIT;
    th13_create(&mn->req_fifo.th, NULL, &_monet_proc_thread, &poll_arg);

    while( 1 ) {

        //accept user

        _deb_wait("%s", "accept");
        if((ret = ilink_accept(&mn->acceptlink, &link)) != E13_OK){
            _deb_wait("%s", e13_codemsg(ret));
            return ret;
        }

        //login user

        //th13_mutex_lock(&mn->user_array.mx); login_recv locks user mx

        _deb_wait("%s", "accepted, recv login");

        if((ret = _mn_login_recv(mn, link)) != E13_OK){
            _deb_wait("%s", e13_codemsg(ret));
            //do nothing, just continue
            ilink_disconnect(link, ILINK_DC_ALL);
            ilink_destroy(link);
            //TODO: stop on serious conditions
            th13_mutex_unlock(&mn->user_array.mx);
            continue;
        }

        _deb_wait("%s", "add poll link");
        if((ret=_monet_add_poll_link(mn, link, ILINK_POLL_RD, 0))!= E13_OK){
			mn_clog(mn, MN_CLOGID_ALL, mn_msg(mn, MN_MSGID_INTERNAL), ret);
        }

        th13_mutex_unlock(&mn->user_array.mx);

        //loop

    }

    return E13_OK;

}

/***                        client side                         ***/

error13_t _mn_login_send(struct monet* mn,
                         char* username,
                         char* password){

	error13_t ret;

	uint16_t unamelen = s13_strlen(username, MN_MAX_USERNAME);
	uint16_t passlen = s13_strlen(password, MN_MAX_PASSWORD);
    ilink_datalen_t datalen = 	2*sizeof(unsigned int) +
								unamelen +
								passlen + 2;


    ilink_data_t data[datalen], *recvdata;

    struct ilink_io ios;
//    struct ilink_packet pkt;
    ilink_pkt_type_t type;

    memset(&ios, 0, sizeof(struct ilink_io));

    ios.ctl.send_time = MN_USER_LOGIN_TO;
    ios.maxpayload = MN_MAX_LOGIN_PAYLOAD;

    _deb_connect("username %s, password %s, datalen %u",
				username,
				password,
				datalen);

    if(pack13(	data, datalen, "bb",
			unamelen+1, username,
			passlen+1, password ) == ((size_t)-1)){
		_deb_connect("packing failed");
		return e13_error(E13_INTERNAL);
	}

    if((ret =
			ilink_prepare_pkt(&mn->acceptlink, data, datalen, MN_PKT_LOGIN, 0))
		!= E13_OK){

		_deb_connect("prepare_pkt failed code: %i, msg: %s", ret,
					e13_codemsg(ret));
		return ret;

    }

    _deb_connect("sending login info...");
    if((ret = ilink_send(&mn->acceptlink, NULL, NULL, &ios)) != E13_OK){
		_deb_connect("send failed code: %i, msg: %s", ret, e13_codemsg(ret));
		return ret;
    }

    _deb_connect("waiting reply...");

    if((ret = ilink_recv(&mn->acceptlink, NULL, NULL, &ios)) != E13_OK){
		_deb_connect("recv failed code: %i, msg: %s", ret, e13_codemsg(ret));
		return ret;
    }
    _deb_connect("got reply.");

    if((ret = ilink_explode_pkt(&mn->acceptlink, &recvdata, &datalen, &type, 0)) !=
		E13_OK){
		_deb_connect("explode_pkt failed code: %i, msg: %s", ret,
					e13_codemsg(ret));
		return ret;
    }

    _deb_connect("checking type...");

	if( type != MN_PKT_LOGIN ) {
		_deb_connect("bad packet type %i", type);
		return e13_error( E13_AUTH );
	}

	_deb_connect("done, logged in.");

    return E13_OK;

}

error13_t mn_connect(struct monet* mn,
                     char* username,
                     char* password,
                     char* address,
                     char* port,
                     struct monet_user** user){

    error13_t ret;
    struct ilink_conf conf;

    if(!_mn_is_init(mn)) return e13_error(E13_MISUSE);

    *user = (struct monet_user*)m13_malloc(sizeof(struct monet_user));
    if(!(*user)) return e13_error(E13_NOMEM);

    (*user)->link = &mn->acceptlink;
    mn->acceptlink.ext_ctx = *user;

    ilink_def_conf(&conf);

    conf.type = ILINK_TYPE_CLIENT;

    conf.port = s13_malloc_strcpy(port?port:mn->conf.port, strlen(port?port:mn->conf.port)+1);
    conf.addr = s13_malloc_strcpy(address?address:MONET_LOCALHOST, strlen(address?address:MONET_LOCALHOST)+1);
    (*user)->name = s13_malloc_strcpy(username?username:mn->conf.guest_user, strlen(username?username:mn->conf.guest_user)+1);

	_deb_connect("port: %s, pipe: %s", mn->conf.port, mn->conf.pipe_port);

    mn_clog(mn, MN_CLOGID_ALL, mn_msg(mn, MN_MSGID_CONNECTING), conf.addr, conf.port);
    ret = ilink_connect(&mn->acceptlink, &conf);

    m13_free(conf.port);
    m13_free(conf.addr);

    if(ret != E13_OK){
    		_deb_connect("failed, code = %i, %s", ret);
#ifndef NDEBUG
	perror("connect");
#endif // NDEBUG
    		goto end;
    }

    _deb_connect("sending login...");
    ret = _mn_login_send(mn, username, password);

end:

    if(ret != E13_OK){
		mn_clog(mn, MN_CLOGID_ALL, mn_msg(mn, MN_MSGID_CONNECTING_FAILED));
        m13_free((*user)->name);
        m13_free(*user);
    } else {
    	mn_clog(mn, MN_CLOGID_ALL, mn_msg(mn, MN_MSGID_CONNECTED));
    }

    return ret;

}
