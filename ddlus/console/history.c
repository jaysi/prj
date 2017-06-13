#include "double.h"

int init_hist(struct session* sess){

    if(!sess->cursess->maxhist){
        sess->cursess->history = NULL;
        return true_;
    }

    sess->cursess->history = (char**)malloc(sess->cursess->maxhist*sizeof(char**));

    if(!sess->cursess->history){
        sess->cursess->maxhist = 0;
        return false_;
    }

    sess->cursess->curhist = sess->cursess->maxhist;

    do {
        sess->cursess->history[--sess->cursess->curhist] = NULL;
    } while(sess->cursess->curhist);

    return true_;
}

int resize_hist(struct session* sess, size_t newsize){

    size_t i;

    if(newsize == sess->cursess->maxhist) return true_;

    if(newsize > sess->cursess->maxhist){
        sess->cursess->history = (char**)realloc(sess->cursess->history, newsize*sizeof(char**));
        if(!sess->cursess->history){
            sess->cursess->maxhist = 0;
            return false_;
        }
        for(i = sess->cursess->maxhist; i < newsize; i++){
            sess->cursess->history[i] = NULL;
        }
        sess->cursess->maxhist = newsize;
    } else {
        for(i = newsize; i < sess->cursess->maxhist; i++){
            if(sess->cursess->history[i]){
                free(sess->cursess->history[i]);
            }
        }

        if(sess->cursess->curhist > newsize) sess->cursess->curhist = newsize?newsize-1:0;

        if(!newsize){
            sess->cursess->history = NULL;
        } else {
            sess->cursess->history = (char**)realloc(sess->cursess->history, newsize*sizeof(char**));
            if(!sess->cursess->history){
                sess->cursess->maxhist = 0;
                return false_;
            }
        }
        sess->cursess->maxhist = newsize;
    }

    return true_;
}

int save_history(struct session* sess){

    if(!sess->cursess->history) return true_;

    //avoid duplicates
    if(	sess->cursess->history[sess->cursess->curhist] &&
        !strcmp(sess->cursess->history[sess->cursess->curhist], sess->cursess->expbuf)) return true_;


    if(sess->cursess->curhist == sess->cursess->maxhist - 1){
        sess->cursess->curhist = 0;
    } else sess->cursess->curhist++;

    printd("saving at %i\n", sess->cursess->curhist);

    if(!sess->cursess->history[sess->cursess->curhist] ||
        strlen(sess->cursess->history[sess->cursess->curhist]) < strlen(sess->cursess->expbuf)){

        sess->cursess->history[sess->cursess->curhist] =
         (char*)realloc(sess->cursess->history[sess->cursess->curhist],
                strlen(sess->cursess->expbuf) + 1);
    }

    if(!sess->cursess->history[sess->cursess->curhist]){
        printe(MSG_NOMEM);
        return false_;
    }

    printd("copying %s\n", sess->cursess->expbuf);
    strcpy_(sess->cursess->history[sess->cursess->curhist], sess->cursess->expbuf, sess->cursess->expbufsize);

    return true_;
}

int get_history(struct session* sess, char* buf, size_t bufsize, int order){

    if(!sess->cursess->history){
        printe(MSG_FAIL, "history");
        return false_;
    }

    if(order >= sess->cursess->maxhist || order < 0){
        printe(MSG_RANGE, "history");
        return false_;
    }

    if(!sess->cursess->history[order]){
        printe(MSG_RANGE, "history");
        return false_;
    }

    strcpy_(buf, sess->cursess->history[order], bufsize);

    return true_;
}

char* last_hist_str(struct session* sess){
	
    if(!sess->cursess->history){        
        return NULL;
    }

    return sess->cursess->history[sess->cursess->curhist];    
}

int get_prev_hist(struct session* sess, char* buf, size_t bufsize){

//    char bud[10];
//    perror(itoa(sess->cursess->curhist, bud, 10));

    if(sess->cursess->curhist && sess->cursess->history[sess->cursess->curhist-1]) sess->cursess->curhist--;
    if(sess->cursess->history[sess->cursess->curhist]) strcpy_(buf, sess->cursess->history[sess->cursess->curhist], bufsize);
    else buf[0] = 0;
    return true_;
}

int get_next_hist(struct session* sess, char* buf, size_t bufsize){

//    char bud[10];
//    perror(itoa(sess->cursess->curhist, bud, 10));

    if(sess->cursess->curhist < sess->cursess->maxhist - 1 && sess->cursess->history[sess->cursess->curhist+1]) sess->cursess->curhist++;
    if(sess->cursess->history[sess->cursess->curhist]) strcpy_(buf, sess->cursess->history[sess->cursess->curhist], bufsize);
    else buf[0] = 0;
    return true_;

}

int list_history(struct session* sess){
    int i;
    char* buf = sess->cursess->outbuf;
    size_t bufsize = sess->cursess->outbufsize, pos = 0;

    print_listhead(sess, "history");
    if(!sess->cursess->history) {printw(MSG_EMPTY, ""); goto end;}
    for(i = 0; i < sess->cursess->maxhist; i++){

        if(sess->cursess->history[i]){
            snprintf(buf + pos, bufsize - pos, "  %i: %s %s"PARAEND, i, sess->cursess->history[i], i == sess->cursess->curhist?"<":"");
            pos += strlen(buf+pos);
            strcat(buf+pos, "\n");
            pos++;
//            printo("  %i: %s", i, sess->cursess->history[i]);
//            if(i == sess->cursess->curhist) printo(" <\n");
//            else printo("\n");
        }
    }

    printo(buf);
end:
    print_listfoot(sess, "history");
    return true_;
}
