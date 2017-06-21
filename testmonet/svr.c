#include "../libmonetr/monetR.h"
#include "main.h"

//enum ccmd_code {
//    CCMD_EMPTY,
//    CCMD_CONNECT,
//    CCMD_CLOSE,
//    CCMD_HELP,
//    CCMD_INVAL
//};
//
//struct con_cmd_s{
//    enum ccmd_code;
//    char* longcmd;
//    char* shortcmd;
//    char* desc;
//} ccmd[] = {
//    {
//        CCMD_EMPTY, "", "", ""
//    },
//    {
//        CCMD_CONNECT, "cconnet", "cco", "connect to a console"
//    },
//    {
//        CCMD_CLOSE, "cclose", "ccl", "close the current connection"
//    },
//    {
//        CCMD_HELP, "chelp", "?", "help on the console platform"
//    },
//    {
//        CCMD_INVAL, NULL, NULL, NULL
//    }
//
//};

void* wait_thread(void* arg){
        error13_t ret;

        if((ret=mn_wait((struct monet*)arg)) != E13_OK){
            printo("\nmn_wait(): %s\n", e13_codemsg(ret));
        }
        return NULL;
}

//int cprompt(struct monet* mn){
//    printo("\nCONSOLE >");
//    return 0;
//}
//
//int ccmd_help(){
//    struct con_cmd_s* s;
//    for(s = ccmd, s->longcmd, s++){
//            printo("%s (%s) - %s", s->longcmd, s->shortcmd, s->desc);
//    }
//    return 0;
//}
//
//enum ccmd_code ccmd_find(char* ccmd){
//    struct con_cmd_s* s;
//    for(s = ccmd, s->longcmd, s++){
//        if(!strcmp(s->longcmd) || !strcmp(s->shortcmd)){
//            return s->ccmd_code;
//        }
//    }
//    return CCMD_INVAL;
//}
//
//int ccmd_do_connect(struct monet* mn, struct monet** mnc, ){
//    char uname[256];
//    char pass[1024];
//    char host[1024];
//    struct monet_user* admin;
//
//    /*
//    printf("username: ");
//    scanf("%s");
//    printf("password: ");
//    scanf("%s");
//    */
//
//    printo("connecting to the console...");
//
//	//if(!strcmp(username, "*")){
//			strcpy(username, "admin");
//			strcpy(password, "admin");
//			strcpy(host, "localhost");
//			//goto do_connect;
//	//}
//
//    mn_init(&mnc, NULL);
//
//    printo("trying to connect to %s:%s\n", host, mnc.conf.console_port);
//
//    if( ( mn_connect(&mnc,
//                     username,
//                     password,
//                     host,
//                     mnc.conf.console_port,
//                     &admin ) ) != E13_OK ){
//
//        printo("failed.\n");
//        return -1;
//
//    }
//
//    printo("done.\n");
//
//    return 0;
//}

int manage_svr(struct monet* mn){
    char cmd[256];
    while(1){
            printo("\nREADY >");
            scanf("%s", cmd);

//        cprompt(mn);
//        scanf("%s", cmd);
//        switch(ccmd_find(cmd)){
//        case CCMD_CONNECT:
//            ccmd_do_connect(mn);
//            break;
//        case CCMD_CLOSE:
//            break;
//        case CCMD_HELP:
//            ccmd_help();
//            break;
//        default:
//
//			if( ( ret = ilink_prepare_pkt(
//					  &mn.acceptlink,
//					  msg,
//					  strlen(msg)+1,
//					  type,
//					  0) ) != E13_OK ) {
//				printo("prepare failed (%i): %s\n", ret, e13_codemsg(ret));
//			} else {
//				ret = ilink_send(&mn.acceptlink, NULL, NULL, NULL);
//				if(ret != E13_OK) {
//					printo("sending failed (%i): %s\n", ret, e13_codemsg(ret));
//				}
//			}
//
//        }
        printo(";>");
    }
    return 0;
}

int svr(){
    struct monet mn;
    struct monet_conf conf;
    char choice[16];
    error13_t ret;
    th13_t th_svr;
    mn_def_conf(&conf);
choice_label:
    printo("[c]onfig / [i]nstall / [r]un / [b]ack:");
    scani("%s", choice);
    if(!strcasecmp(choice, "config") || !strcasecmp(choice, "c")){

    } else if(!strcasecmp(choice, "install") || !strcasecmp(choice, "i")){
        if((ret = mn_install(&mn, &conf)) != E13_OK){
            printo("\nmn_install(): %s\n", e13_codemsg(ret));
        }
    } else if(!strcasecmp(choice, "run") || !strcasecmp(choice, "r")){
        mn_init(&mn, &conf);

        th13_create(&th_svr, NULL, &wait_thread, (void*)&mn);

        manage_svr(&mn);

        printo("Waiting for the MoNet thread...");
        th13_join(th_svr, NULL);

    } else if(!strcasecmp(choice, "back") || !strcasecmp(choice, "b")){
        return 0;
    } else {
        printo("use one of the choices provided.\n");
    }

    goto choice_label;

    return 0;
}
