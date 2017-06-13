#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "double.h"

#ifdef __WIN32
#define LOADFLAGS "rt"
#define SAVEFLAGS "at"
#else
#define LOADFLAGS "r"
#define SAVEFLAGS "a"
#endif

int do_load(struct session* sess){
    struct tok_s* tk;
    int ret;

    DEF_LOAD_RES_VARS();

    tk = poptok(sess);
    if(!tk){
        printe(MSG_MOREARGS, "load");
        return false_;
    } else {

        LOAD(1);

        ret = loadfile(sess, tk->str);

        RESTORE(1);

    }

    return ret;

}

int do_save(struct session* sess){
    struct tok_s* tk;
    tk = poptok(sess);
    if(!tk){
        printe(MSG_MOREARGS, "save");
        return false_;
    } else {

        if(!strcmp(tk->str, "stop")){
            return closesavefile(sess);
        } else {
            return savefile(sess, tk->str);
        }
    }

}

int do_view(struct session* sess){
    int fd;
    int red;
    int i;
    struct tok_s* tk;
    char* buf;
    size_t size;
    tk = poptok(sess);
    if(!tk){
        printe(MSG_MOREARGS, "view");
        return false_;
    } else {
        fd = open(tk->str, O_RDONLY);
        if(fd > -1){            
            red = read(fd, sess->cursess->outbuf, sess->cursess->outbufsize);
            if(red > 0){                
                tk = poptok(sess);
                if(tk && tk->id == TOK_HEX){
                    buf = (char*)malloc(red);
                    memcpy(buf, sess->cursess->outbuf, red);
                    size = sess->cursess->outbufsize;
                    for(i = 0; i < red; i++){
                        snprintf(sess->cursess->outbuf+(sess->cursess->outbufsize-size), size, "%x ", buf[i]);
                        size -= strlen(sess->cursess->outbuf);
                    }
                } else {
                    sess->cursess->outbuf[red] = 0;
                }
                textout_(sess->cursess->outbuf);
            }
            close(fd);
        } else return false_;
    }
    return true_;
}

int loadfile(struct session* sess, char* filename){

    FILE* file;
    size_t bufsize, explen;
    char* buf;
    int line = 1;
    struct pre_exp* pre, *old_pre;
    struct stat st;

    if(!filename){
        printe(MSG_MOREARGS, "load");
        return false_;
    }

    printi("loading \"%s\"..."PARAEND, filename);

    file = fopen(filename, LOADFLAGS);
    if(!file){
        //perror("fopen()");
        printe(MSG_NOOPEN, filename);
        return false_;
    }

    printd("file %s opened, bufsize: %u\n", filename, sess->cursess->expbufsize);

    if(sess->cursess->fileflags & SESS_FILEF_LINE){

        bufsize = sess->cursess->expbufsize;
        buf = (char*)malloc(bufsize);

        while(!feof(file)){

            if(!fgets(buf, bufsize, file)){
                if(ferror(file)){
                    //perror("fgets()");
                    printe(MSG_NOREAD, filename);
                    return false_;
                }
            } else if(	(explen = strlen_(buf, bufsize)) &&
                        buf[0] != '\n' &&
                        buf[0] != '\r'){

                if(buf[explen-1] == '\n'){
                    buf[explen-1] = '\0';
                }

                if(sess->cursess->uiflags & SESS_UIF_LOUD) printo("  %i: %s", line, buf);

                if(!memcmp(buf, COMMENT_SIGN, COMMENT_SIGN_SIZE)){
                    if(sess->cursess->uiflags & SESS_UIF_LOUD) printo(" <- commented out"PARAEND);
                } else {
#ifdef CONSOLE
                    if(sess->cursess->uiflags & SESS_UIF_LOUD) printo(PARAEND);
#endif
                    if(!_precompile(sess, buf, &pre)){
                        free(buf);
                        return false_;
                    }

                    old_pre = sess->cursess->pre;
                    sess->cursess->pre = pre;

                    if(!run_ci(sess)){
                        sess->cursess->pre = old_pre;
                        delete_pre(sess, pre);
                        free(buf);
                        return false_;
                    }

                    sess->cursess->pre = old_pre;
                    delete_pre(sess, pre);

                }
            }

            printd("read %s\n", buf);

            line++;
        }
        free(buf);
    } else {

        stat(filename, &st);
        bufsize = st.st_size;
        buf = (char*)malloc(bufsize);
        explen = 0;
        while(!feof(file)){
            explen += fread(buf+explen, sizeof(char), bufsize-explen, file);
        }

        //perror(buf);

        if(!_precompile(sess, buf, &pre)){
            free(buf);
            return false_;
        }

        old_pre = sess->cursess->pre;
        sess->cursess->pre = pre;

        if(!run_ci(sess)){
            sess->cursess->pre = old_pre;
            delete_pre(sess, pre);
            free(buf);
            return false_;
        }

        sess->cursess->pre = old_pre;
        delete_pre(sess, pre);
        free(buf);
    }

    fclose(file);

    return true_;
}

int savefile(struct session* sess, char* filename){

    if(!filename){
        printe(MSG_MOREARGS, "save");
        return false_;
    }

    if(!strcmp(filename, "stop")){
        if(sess->cursess->outfile){
            fclose(sess->cursess->outfile);
            sess->cursess->outfile = NULL;
        }
    } else {
        //printi("saving \"%s\"..."PARAEND, filename);
        if(!sess->cursess->outfile){
            sess->cursess->outfile = fopen(filename, SAVEFLAGS);
            if(!sess->cursess->outfile){
                printe(MSG_NOOPEN, filename);
                return false_;
            } else {
                //printi(MSG_OK);
                return true_;
            }
        } else {
            printw(MSG_EXISTS, filename);
            return true_;
        }
    }

    return true_;
}

int closesavefile(struct session* sess){
    flushf(sess);
    if(sess->cursess->outfile){
        fclose(sess->cursess->outfile);
        sess->cursess->outfile = NULL;
        //printi("output file closed."PARAEND);
    }
    return true_;
}

int flushf(struct session* sess){
    if(sess->cursess->outfile){
        if(!fflush(sess->cursess->outfile)){
            //printi(MSG_OK);
            return true_;
        } else {
            printw(MSG_FAIL, "flush");
        }
    } else {
        //printw("file not open.");
        return true_;
    }
    return false_;

}

int record(struct session* sess){
    printd3("recording \"%s\"\n", sess->cursess->expbuf);
    if(sess->cursess->outfile){
        switch(sess->cursess->mode1){
            case SESSM1_ACC:
                if(!memcmp(sess->cursess->outbuf, ANS_ICON, ICON_SIZE) || !memcmp(sess->cursess->outbuf, RESULT_ICON, ICON_SIZE)){
                    fputs(sess->cursess->outbuf + ICON_SIZE, sess->cursess->outfile);
                    fputs("\n#############\n", sess->cursess->outfile);
                }
                break;
            default:
                fputs(last_hist_str(sess->cursess), sess->cursess->outfile);
                fputs("\n", sess->cursess->outfile);
                break;
        }
        return true_;
    }
    return continue_;
}
