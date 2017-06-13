#include "../libmonetr/monetR.h"
#include "main.h"

void* wait_thread(void* arg){

        if((ret = mn_wait(&mn)) != E13_OK){
            printo("\nmn_wait(): %s\n", e13_codemsg(ret));
        }
        return NULL;
}

int manage_svr(struct monet* mn){
    printo("\n:)\n");
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

        th13_join(th_svr, NULL);

    } else if(!strcasecmp(choice, "back") || !strcasecmp(choice, "b")){
        return 0;
    } else {
        printo("use one of the choices provided.\n");
    }

    goto choice_label;

    return 0;
}
