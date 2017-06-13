#include "double.h"

#include <errno.h>
#include <sys/types.h>

#define FMT "%%.%uLf"
#define FMT2 "%%.%uLf|" //array print format

/*	format functions	*/
char* clean_float(struct session* sess, fnum_t num, char* buf, size_t bufsize){
    int i;

    char fmt[10];

    if(sess->decim > 10){
        sess->decim = 10;
    }

    //"%.10Lf"

    snprintf(fmt, 10, FMT, sess->decim);

    snprintf(buf, bufsize, fmt, num);
    //snprintf(buf, bufsize, "%.10f", num);

    //printo("clean: %s\n", buf);

    if(!(sess->uiflags & SESS_UIF_DECIMNOTR) && sess->decim){

        i = strlen(buf)-1;

        while(buf[i] == '0') i--;

        if(buf[i] != '.') buf[i+1] = '\0';
        else buf[i] = '\0';

    }

    return buf;
}

char* sep_val(struct session* sess, fnum_t num, char* buf, size_t bufsize,
              char* tmp, size_t tmpsize){

    int ilen, iseplen, i;//, neg;
    char* end;

    //if(num < 0) neg = 1;
    //else neg = 0;       

    snprintf(tmp, tmpsize, "%s",
        num>=0?clean_float(sess, num, buf, bufsize):
        clean_float(sess, -1*num, buf, bufsize));

    if(!(sess->uiflags & SESS_UIF_NOSEP)){

        end = strstr(tmp, ".");

        if(!end){
            ilen = strlen(tmp);
            if(!ilen){
                //iseplen = 0;
                return buf;
            } else {
                iseplen = ilen + (ilen / 3) - (!(ilen%3)?1:0);
            }

        } else {
            ilen = end - tmp;
            iseplen = ilen + (ilen / 3) - (!(ilen%3)?1:0);
            memcpy(buf + iseplen, end, strlen(end)+1);
        }

        i = 0;

        while(iseplen){

            buf[iseplen--] = tmp[ilen--];

            if( i == 3 ){
                buf[iseplen--] = '\'';
                i = 0;
            }

            i++;

        }

        if(num<0){

            snprintf(tmp, tmpsize, "-%s", buf);
            strcpy_(buf, tmp, bufsize);

        }

    } else {
        strcpy_(buf, tmp, bufsize);
    }

    return buf;
}

char* sep_buf(char* tmp, char* buf, size_t bufsize){

    int ilen, iseplen, i, bufstart;//, neg;
    char* end, *start;
    //char deb[MAXNUMLEN];

    //if(num < 0) neg = 1;
    //else neg = 0;

    //clean trailing zeros
//    if(strchr(tmp, '.')){
//        i = strlen(tmp)-1;

//        while(tmp[i] == '0') i--;

//        if(tmp[i] != '.') tmp[i+1] = '\0';
//        else tmp[i] = '\0';
//    }
    //clean trailing zeros

    start = strpbrk(tmp, "1234567890");
    if(!start){
        bufstart = 0;
        start = tmp;
    } else {
        bufstart = start - tmp;
        memcpy(buf, tmp, bufstart);
    }

    end = start;
    while(strchr("1234567890", *end)){//TODO: check bounds
        end++;
    }

    if(end == start) {strcpy_(buf, tmp, bufsize); return buf;}

    if(!(*(end-1))){
        ilen = strlen(start);
        iseplen = ilen + (ilen / 3) - (!(ilen%3)?1:0);
    } else {
        ilen = end - start;
        iseplen = ilen + (ilen / 3) - (!(ilen%3)?1:0);
        memcpy(buf + iseplen + bufstart, end, strlen(end) + 1);
    }

    //protect buffer
    if(iseplen > bufsize || ilen > bufsize){
        buf[0] = 'E';
        buf[1] = 0;
        return buf;
    }

//    perror(itoa(ilen, deb, MAXNUMLEN));
//    perror(itoa(iseplen,deb,MAXNUMLEN));

    i = 0;

    while(ilen){

        //printf("buf[%i]=tmp[%i](%c),", iseplen, ilen, tmp[ilen-1]);

        buf[bufstart+(iseplen--)] = start[ilen--];

        if( i == 3 ){
            //printf("buf[%i]=',", iseplen);
            buf[bufstart+(iseplen--)] = '\'';
            i = 0;
        }

        i++;
    }

    buf[bufstart] = start[0];


//    if(num<0){

//        snprintf(tmp, tmpsize, "-%s", buf);
//        strcpy_(buf, tmp, bufsize);

//    }

    //printf("return %s\n", buf);

    return buf;
}


void printval(struct session* sess, fnum_t val){
    char buf[MAXNUMLEN];
     printo("%s",	sep_val(sess->cursess, val,
                    sess->cursess->tokbuf,
                    sess->cursess->tokbufsize,
                    buf,
                    MAXNUMLEN));
}

int append_nerd_info(struct session* sess){
    snprintf(sess->cursess->outbuf + strlen(sess->cursess->outbuf),
             sess->cursess->outbufsize - strlen(sess->cursess->outbuf),
             "\ntime: %.10f seconds.\n"
             "ntokens: %i"PARAEND,
             sess->cursess->rt,
             sess->cursess->calc);
    return true_;
}

int show_ans_nr(struct session* sess){
    char buf[MAXNUMLEN];
    size_t len;
    printd("TODO: this buffer is temporary\n");

    if(sess->cursess->uiflags & SESS_UIF_DET){
        snprintf(sess->cursess->outbuf, sess->cursess->outbufsize, "%s@%s: %s",
                ANS_ICON,
                sess->cursess->name,
                sep_val(sess->cursess, var_val(sess, ANSVARNAME),
                        sess->cursess->tokbuf,
                        sess->cursess->tokbufsize,
                        buf,
                        MAXNUMLEN));
    } else {
        snprintf(sess->cursess->outbuf, sess->cursess->outbufsize, "%s%s",
                ANS_ICON,
                sep_val(sess->cursess, var_val(sess, ANSVARNAME),
                        sess->cursess->tokbuf,
                        sess->cursess->tokbufsize,
                        buf,
                        MAXNUMLEN));
    }    

    len = strlen(sess->cursess->outbuf);

    if(sess->cursess->uiflags & SESS_UIF_DET){
        snprintf(   sess->cursess->outbuf + len,
                    sess->cursess->outbufsize - len,
                    "\t[ %s ", last_hist_str(sess));

        len = strlen(sess->cursess->outbuf);

        if(sess->cursess->autorun == AUTORUN_DEFON){
            snprintf(sess->cursess->outbuf + len, sess->cursess->outbufsize - len,
                     "( %s ) ]"PARAEND, "A");
        } else {
            snprintf(sess->cursess->outbuf + len, sess->cursess->outbufsize - len,
                    "]"PARAEND);
        }

    } else {
#ifndef QT
        snprintf(sess->cursess->outbuf + len, sess->cursess->outbufsize - len, "%s", PARAEND);
#endif
    }

    if(sess->cursess->uiflags & SESS_UIF_NERD)
        append_nerd_info(sess);

    textout_(sess->cursess->outbuf);

    return true_;
}

int show_ans_acc(struct session* sess){
    char brk[]="+-*/^%;";
    char* start, *end, *last;
    size_t len, pos;
    char buf[MAXNUMLEN];
    printd("TODO: this buffer is temporary\n");

    //pos = 0;

    if(sess->cursess->uiflags & SESS_UIF_DET){
        snprintf(sess->cursess->outbuf, sess->cursess->outbufsize, "%s@%s:\n", ANS_ICON, sess->cursess->name);
    } else {
        snprintf(sess->cursess->outbuf, sess->cursess->outbufsize, "%s\n", ANS_ICON);
    }
    pos = ICON_SIZE;
    //pos = strlen(sess->cursess->outbuf);

    //start = sess->cursess->expbuf;
    last = last_hist_str(sess);
    start = last;
    end = start;

    while(end){

        end = strpbrk(start, brk);
        if(!end) break;
        if(end){
            len = end - start;
            if(len) memcpy(sess->cursess->tokbuf, start, len);
            sess->cursess->tokbuf[len] = '\0';
            start = end + 1;
        } else {
            //len = start - sess->cursess->expbuf;
            len = start - last;
            memcpy(sess->cursess->tokbuf, start, len);
            sess->cursess->tokbuf[len] = '\0';
        }

        if(len){
            sep_buf(sess->cursess->tokbuf, buf, MAXNUMLEN);
        } else {
            strcpy(buf, ANSVARNAME);
        }

        snprintf(   sess->cursess->outbuf + pos,
                    sess->cursess->outbufsize - pos,
                    "%s\n%c ",
                    buf,
                    *end);
        //printf("buf: %s, strlen(buf): %i, @pos = %i\n", buf, strlen(buf), pos);

        pos += strlen(buf) + (3);

    }

    pos-=2;//to remove unwanted ;

    //printf("tokbuf: %s, pos: %i, outbuf:\n%s"PARAEND, sess->cursess->tokbuf, pos, sess->cursess->outbuf);


    sep_val(sess->cursess, var_val(sess, ANSVARNAME),
            sess->cursess->tokbuf,
            sess->cursess->tokbufsize,
            buf,
            MAXNUMLEN);

    len = strlen(buf) + 4;

    while(len){
        *(sess->cursess->outbuf + pos) = '-';
        len--;
        pos++;
    }

    snprintf(   sess->cursess->outbuf + pos,
                sess->cursess->outbufsize - pos,
                "\n= %s"PARAEND,
                sep_val(sess->cursess, var_val(sess, ANSVARNAME),
                        sess->cursess->tokbuf,
                        sess->cursess->tokbufsize,
                        buf,
                        MAXNUMLEN));

    textout_(sess->cursess->outbuf);

    return true_;
}

int show_ans_prog(struct session* sess){
    char buf[MAXNUMLEN];
    size_t len;
    char* last;
    printd("TODO: this buffer is temporary\n");

    if(sess->cursess->uiflags & SESS_UIF_DET){

		last = last_hist_str(sess);

        len = 0;

//        while(sess->cursess->expbuf[len]){
//            if(sess->cursess->expbuf[len] == '\n') sess->cursess->expbuf[len] = ' ';
//            len++;
//        }

        while(last[len]){
            if(last[len] == '\n') last[len] = ' ';
            len++;
        }

        snprintf(   sess->cursess->outbuf,
                    sess->cursess->outbufsize,
                    "%s@%s: {\n  %s", ANS_ICON, sess->cursess->name, sess->cursess->expbuf);

        len = strlen(sess->cursess->outbuf);

        if(sess->cursess->autorun == AUTORUN_DEFON){
            snprintf(sess->cursess->outbuf + len, sess->cursess->outbufsize - len,
                     "\n  ( %s ) }\n", "A");
        } else {
            snprintf(sess->cursess->outbuf + len, sess->cursess->outbufsize - len,
                    "\n}\n");
        }

    } else {
        snprintf(sess->cursess->outbuf, sess->cursess->outbufsize, "%s", ANS_ICON);
    }

    len = realstrlen(sess->cursess->outbuf);

    snprintf(sess->cursess->outbuf + len, sess->cursess->outbufsize - len, "%s"PARAEND,
            sep_val(sess->cursess, var_val(sess, ANSVARNAME),
                    sess->cursess->tokbuf,
                    sess->cursess->tokbufsize,
                    buf,
                    MAXNUMLEN));

    textout_(sess->cursess->outbuf);

    return true_;
}


int show_ans(struct session* sess){

    if(!(sess->cursess->uiflags & SESS_UIF_ANS)) return true_;

    switch(sess->cursess->mode1){
        case SESSM1_ACC:
            return show_ans_acc(sess);
        break;
        case SESSM1_PROG:
            return show_ans_prog(sess);
        break;
        default:
            return show_ans_nr(sess);
        break;
    }
    return true_;
}

int print_array(struct session* sess, char* varname){

    struct variable* var;
    size_t len = realstrlen(sess->cursess->outbuf);
    int i, j, imax, jmax;

    char fmt[10];

    array_limits(sess, varname, &imax, &jmax);
    imax++;
    jmax++;

    snprintf(sess->cursess->outbuf + len,
             sess->cursess->outbufsize - len,
             "\n");
    len++;

    for(j = 0; j < jmax; j++){
        snprintf(sess->cursess->outbuf + len,
                 sess->cursess->outbufsize - len, "    %i|", j);
        len = realstrlen(sess->cursess->outbuf);
    }

    snprintf(sess->cursess->outbuf + len,
             sess->cursess->outbufsize - len,
             "\n");
    len++;

    for(i = 0; i < imax; i++){

        snprintf(sess->cursess->outbuf + len,
                 sess->cursess->outbufsize - len, "%i: ", i);
        len = realstrlen(sess->cursess->outbuf);

        for(j = 0; j < jmax; j++){
            var = array_ptr(sess, varname, i, j, NULL);
            if(var){
                snprintf(fmt, 10, FMT2, sess->decim);
                snprintf(sess->cursess->outbuf + len,
                         sess->cursess->outbufsize - len,
                         fmt,
                         var->val);
                len = realstrlen(sess->cursess->outbuf);
            }
        }

        snprintf(sess->cursess->outbuf + len,
                 sess->cursess->outbufsize - len,
                 "\n");
        len++;

    }

    return true_;

}

int show_result(struct session* sess){
    char buf[MAXNUMLEN];
    struct tok_s* tk;
    struct variable* var;
    int jdate[3];
    int gdate[3];
    inum_t inum;
    
    sess->cursess->pre->exp = sess->cursess->pre->explast;
    
    tk = poptok(sess);

    if(!tk){
//		printw("no results."PARAEND);
		sess->cursess->pre->exp = NULL;
        return false_;
    }

    switch(tk->id){
        case TOK_VAR:
            var = var_ptr(sess, tk->str, NULL);
            if(var){
                if(sess->cursess->uiflags & SESS_UIF_DET){
                    printo("%s@%s: variable '%s' [ %s ]: -> %s%s",
                           RESULT_ICON,
                           sess->cursess->name,
                            tk->str,
                            var->exp?var->exp:"",
                            sep_val(sess->cursess, var_val(sess, tk->str),
                                sess->cursess->tokbuf,
                                sess->cursess->tokbufsize,
                                buf,
                                MAXNUMLEN), PARAEND
                    );
                } else {
                    printo("%s%s: %s"PARAEND,
                           RESULT_ICON,
                            tk->str,
                            sep_val(sess->cursess, var_val(sess, tk->str),
                                sess->cursess->tokbuf,
                                sess->cursess->tokbufsize,
                                buf,
                                MAXNUMLEN)
                    );
                }
            }
            break;

        case TOK_ARRAY:

        if(tk->i < 0 && tk->j < 0) {
            if(sess->cursess->uiflags & SESS_UIF_DET){
                snprintf(sess->cursess->outbuf, sess->cursess->outbufsize,
                         "%s@%s: array %s: ", RESULT_ICON, sess->cursess->name,
                         tk->str);
            } else {
                snprintf(sess->cursess->outbuf, sess->cursess->outbufsize,
                         "%sarray %s: ", RESULT_ICON,
                         tk->str);
            }
            print_array(sess, tk->str);
            textout_(sess->cursess->outbuf);
        } else {            
            var = array_ptr(sess, tk->str, tk->i, tk->j, NULL);
            if(var){
                if(sess->cursess->uiflags & SESS_UIF_DET){
                    printo("%s@%s: array '%s[%i][%i]' [ %s ]: -> %s"PARAEND, RESULT_ICON,
                            sess->cursess->name,
                            tk->str,
                            tk->i, tk->j,
                            var->exp?var->exp:"",
                            sep_val(sess->cursess, array_val(sess, tk->str, tk->i, tk->j),
                                sess->cursess->tokbuf,
                                sess->cursess->tokbufsize,
                                buf,
                                MAXNUMLEN)
                        );
                } else {
                    printo("%s%s[%i][%i]: %s"PARAEND,
                           RESULT_ICON,
                            tk->str,
                            tk->i, tk->j,
                            sep_val(sess->cursess, array_val(sess, tk->str, tk->i, tk->j),
                                sess->cursess->tokbuf,
                                sess->cursess->tokbufsize,
                                buf,
                                MAXNUMLEN)
                        );
                }
            }
        }
            break;
        case TOK_DATE:
            inum = intval(tokval(sess, tk));
            jdayno2jdate(inum, jdate);
            //jdayno2gdate(i, gdate);

            if(sess->cursess->uilocale == SESS_LOCALE_FA) {
                d13_j2g(jdate[0], jdate[1], jdate[2], gdate);
                printo( "%i/%i/%i"PARAEND,
                        jdate[0], jdate[1], jdate[2]
                        );
            } if(sess->cursess->uilocale == SESS_LOCALE_EN) {

                d13_j2g(jdate[0], jdate[1], jdate[2], gdate);
                printo( "%i/%i/%i"PARAEND,
                        gdate[0], gdate[1], gdate[2]
                        );

            }
//old code
//            d13_j2g(jdate[0], jdate[1], jdate[2], gdate);
//            printo( "jalali[ %i/%i/%i ]\ngregorian[ %i/%i/%i ]"PARAEND,
//                    jdate[0], jdate[1], jdate[2],
//                    gdate[0], gdate[1], gdate[2]
//                    );


            break;
        default:
        if(sess->cursess->uiflags & SESS_UIF_DET){
            if(!strcmp(tk->str, "system")) strcpy(tk->str, "down!");
            printo("%s@%s: %s%s", RESULT_ICON, sess->cursess->name, tk->str, PARAEND);
        } else {
            if(!strcmp(tk->str, "system")) strcpy(tk->str, "down!");
            printo("%s%s%s", RESULT_ICON, tk->str, PARAEND);
        }
            break;        
	}

	sess->cursess->pre->exp = NULL;

    return true_;

}

void asm_sess_addr(struct session* sess, char* buf){
	if(sess->parent){
		asm_sess_addr(sess->parent, buf);
	}
	strcat(buf, sess->name);
	strcat(buf, dbl_chain_str);
}

void print_listhead(struct session* sess, char* name){
	
	size_t len;

	//strcpy(sess->cursess->outbuf, dbl_chain_str);
	snprintf(sess->cursess->outbuf, sess->cursess->outbufsize, "%s%s", INFOBLOCK_ICON, dbl_chain_str);
	asm_sess_addr(sess->cursess, sess->cursess->outbuf);
	len = strlen(sess->cursess->outbuf);
	len--;//remove the later @
	snprintf(sess->cursess->outbuf + len, sess->cursess->outbufsize - len,
			": %s {%s", name, PARAEND);
	
	textout_(sess->cursess->outbuf);
	
    //printo(INFOBLOCK_ICON"@%s: %s {%s", sess->cursess->name, name, PARAEND);
    
}

void print_listfoot(struct session* sess, char* name){
    printo(INFOBLOCK_ICON"} %s;\n"PARAEND, name);
}

void print_as_bin(char* buf, inum_t val){
    size_t i;
    buf[0] = '0';
    buf[1] = 'b';
    buf[66] = 0;
    for(i = 2; i < 66; i++){
        if(val & 1LL << (i-2)){
            buf[i] = '1';
        } else {
            buf[i] = '0';
        }
    }

    //trim
    for(i = 65; i > 2; i--){
        if(buf[i] == '1') break;
    }
    buf[i+1] = 0;
    //strcpy(buf + 3, buf + i);
}

int print(struct session* sess){

#define PRINTF_HEX (0x01<<0)
#define PRINTF_BIN (0x01<<1)
#define PRINTF_DET (0x01<<2)

    struct tok_s* tk, *first = NULL, *last=NULL;
    size_t len = 0;
    char flags = 0;
    char* buf;
    char ibuf[MAXNUMLEN];
    char bitbuf[67];

    //reverse stack
    while((tk = poptok(sess))){
        if(!last){
            last = tk;
            last->next = NULL;
        } else {
            tk->next = first;
        }
        first = tk;
    }

    if(!first){
        printe(MSG_MOREARGS, "print");
        return false_;
    }

    for(tk = first; tk; tk = tk->next){

        switch(tk->id){
        case TOK_TRIM:
            snprintf(sess->cursess->outbuf + len, sess->cursess->outbufsize - len, "\n");
            len++;
            break;
        case TOK_STR:
            snprintf(sess->cursess->outbuf + len, sess->cursess->outbufsize - len, "%s", tk->str);
            len += realstrlen(tk->str);
            break;
        case TOK_HEX:
            if(flags & PRINTF_BIN) flags &= ~PRINTF_BIN;
            flags |= PRINTF_HEX;
            break;
        case TOK_BIN:
            if(flags & PRINTF_HEX) flags &= ~PRINTF_HEX;
            flags |= PRINTF_BIN;
            break;
        case TOK_ALL:
            flags |= PRINTF_DET;
            break;
        default:
            if(flags & PRINTF_BIN){
                buf = bitbuf;
                print_as_bin(buf, intval(tokval(sess, tk)));
                flags &= ~PRINTF_BIN;
            } else if(flags & PRINTF_HEX){
                buf = ibuf;
                snprintf(buf, MAXNUMLEN, "0x%Lx", intval(tokval(sess, tk)));
                flags &= ~PRINTF_HEX;
            } else {
                if(flags & PRINTF_DET){
                    if(tk->id == TOK_ARRAY){
                        print_array(sess, tk->str);
                        len += realstrlen(sess->cursess->outbuf);
                    }
                    flags &= ~PRINTF_DET;
                    buf = ibuf;
                    *buf = 0;
                } else {
                    buf = ibuf;
                    clean_float(sess->cursess, tokval(sess, tk), buf, MAXNUMLEN);
                }
            }

            snprintf(sess->cursess->outbuf + len, sess->cursess->outbufsize - len, "%s", buf);
            len += realstrlen(buf);
            break;
        }

    }

    //snprintf(sess->cursess->outbuf + len, sess->cursess->outbufsize - len, PARAEND);

    textout_(sess->cursess->outbuf);

    return true_;

}

#ifdef CONSOLE
int plotxy(struct session* sess){
    printe(MSG_IMPLEMENT, "plot");
    return false_;
}
int editarray(struct session* sess){
    printe(MSG_IMPLEMENT, "aedit");
    return false_;
}
#endif
