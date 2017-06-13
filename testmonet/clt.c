#include "main.h"
#include "../libmonetr/monetR.h"
#include "../libmonetr/monet_pkt.h"

#define CMD_START   '`'

struct cmd_code {

    char* cmd;
    char* desc;
    ilink_pkt_type_t code;

} cc[] = {

	{"ss", "start session", MN_PKT_SESS_START},
	{"sp", "pause session", MN_PKT_SESS_PAUSE},
	{"se", "end session", MN_PKT_SESS_END},
	{"sj", "join session", MN_PKT_SESS_JOIN},
	{"sd", "debug session", MN_PKT_SESS_DEBUG},
	{"sl", "list session", MN_PKT_SESS_LIST},
	{"ts", "start transaction", MN_PKT_SESS_TRANS_START},
	{"ta", "acknowledge transaction", MN_PKT_SESS_TRANS_ACK},
	{"te", "end transaction", MN_PKT_SESS_TRANS_END},
	{NULL, NULL, ILINK_PKT_TYPE_INVAL}

};

ilink_pkt_type_t find_cmd(char* cmd){

    struct cmd_code* c;
    for(c = cc; c->cmd; c++){
        if(!strcmp(cmd, c->cmd)){
            return c->code;
        }
    }
    return ILINK_PKT_TYPE_INVAL;
}

error13_t proc_pkt(struct monet* mn, ilink_data_t* data, ilink_datalen_t datalen, ilink_data_t type){

    switch(type){

    case ILINK_PKT_TYPE_TEST:
        printo("\nTEST MSG [%u bytes]: %s\n", datalen, data);
        break;

    default:
        printo("not implemented yet.\n");
        break;

    }

    printo( "READY> " );

    if(datalen) m13_free(data);

    return E13_OK;
}

void* recv_thread(void* arg) {

    struct monet* mn = (struct monet*)arg;
    ilink_data_t* data;
    ilink_datalen_t datalen;
    ilink_pkt_type_t type;
    error13_t ret;

    printf("recieve thread starting...\n");

    while( 1 ) {
        ret = ilink_recv(&mn->acceptlink, NULL, NULL, NULL);
        if(ret == E13_OK) {
            if((ret = ilink_explode_pkt(&mn->acceptlink,
                                        &data,
                                        &datalen,
                                        &type,
                                        0)) == E13_OK) {

                proc_pkt(mn, data, datalen, type);

            } else {
                printo("explode error (%i): %s\n", ret, e13_codemsg(ret));
            }
        } else {
            printo("recieve error (%i): %s\n", ret, e13_codemsg(ret));
            if(ret == e13_error(E13_DC)) {
                printo("DISCONNECTED.\n");
                break;
            }
        }
    }

    return NULL;

}

error13_t proc_cmd(struct monet* mn, struct monet_user* user, char* cmd, ilink_data_t** msg, ilink_datalen_t* len){

}

int clt(){
    struct monet mn;
    struct monet_user* user;
    char cmd[1024];
    char* msg;
    error13_t ret;
    th13_t th;
    char username[MN_MAX_USERNAME], password[MN_MAX_PASSWORD], host[MN_MAX_HOSTNAME];
    ilink_pkt_type_t type;

    printo("username(* = auto guest-local): ");
    scani("%s", username);
	if(!strcmp(username, "*")){
			strcpy(username, "guest");
			strcpy(password, "guest");
			strcpy(host, "localhost");
			goto do_connect;
	}
    printo("password: ");
    scani("%s", password);
    printo("host: ");
    scani("%s", host);

//    if(!strcmp(username, "*")) strcpy(username, "guest");
//    if(!strcmp(password, "*")) strcpy(password, "guest");
//    if(!strcmp(host, "*")) strcpy(host, "localhost");

do_connect:
    mn_init(&mn, NULL);

    printo("trying to connect to %s:%s\n", host, mn.conf.port);

    if( ( mn_connect(&mn,
                     username,
                     password,
                     host,
                     NULL,
                     &user ) ) != E13_OK ){

        return -1;

    }

    th13_create( &th, NULL, &recv_thread, &mn );

    printo( "START CMD USING %c CHARACTER or \n", CMD_START );
    printo( "TYPE SOMETHING TO TEST SYSTEM.\n");

    while( 1 ) {

        printo( "READY> " );
        scani( "%s", cmd );

        if(cmd[0] == CMD_START){

            switch((type = find_cmd(cmd+1))){
                case MN_PKT_SESS_START:
                	msg = cmd;
					printo("name: ");
					scani("%s", msg);
					if((ret = mn_open_sess(&mn, msg, MN_SESSFLAG_DEF)) !=
							E13_OK){
						printo("open session failed (%i): %s\n", ret,
									e13_codemsg(ret));
						break;
					}
                break;
            default:
                printo("not implemented\n");
                continue;
                break;//!!
            }

        } else {

            type = ILINK_PKT_TYPE_TEST;
            msg = cmd;

			if( ( ret = ilink_prepare_pkt(
					  &mn.acceptlink,
					  msg,
					  strlen(msg)+1,
					  type,
					  0) ) != E13_OK ) {
				printo("prepare failed (%i): %s\n", ret, e13_codemsg(ret));
			} else {
				ret = ilink_send(&mn.acceptlink, NULL, NULL, NULL);
				if(ret != E13_OK) {
					printo("sending failed (%i): %s\n", ret, e13_codemsg(ret));
				}
			}

        }

    }

    return 0;
}
