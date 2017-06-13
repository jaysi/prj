#include "double.h"

#ifdef WIN32
#include <windows.h> //for _set_SSE2_enable
#endif

const char expdelimlist[] = "\"\'{};";
const char expescape = '\\';
const char exppacklist[] = "\"\'";
const char expblocklist[] = "{}";

int set_start_time(struct session* sess){

#ifdef _WIN32
    sess->PCFreq = 0.0;
    sess->CounterStart = 0;

    QueryPerformanceFrequency(&sess->li);

    //PCFreq = double(li.QuadPart)/1000000000.0;
    sess->PCFreq = (double)(sess->li.QuadPart);

    QueryPerformanceCounter(&sess->li);
    sess->CounterStart = sess->li.QuadPart;
#else
    struct timespec tS;
    clock_settime(CLOCK_PROCESS_CPUTIME_ID, &sess->tS);
#endif

    return true_;
}

int set_end_time(struct session* sess){
#ifdef _WIN32
    QueryPerformanceCounter(&sess->li);
    //pthread_mutex_lock(&sess->root->mx);
    sess->cursess->rt = (sess->li.QuadPart-sess->CounterStart)/sess->PCFreq;
    //pthread_mutex_unlock(&sess->root->mx);
#else
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &sess->tS);
    //pthread_mutex_lock(&sess->root->mx);
    sess->cursess->rt = sess->tS.tv_sec + sess->tS.tv_nsec/1000000000.0;
    //pthread_mutex_unlock(&sess->root->mx);
#endif
    return true_;
}

static inline int reset_end_time(struct session* sess){
    sess->cursess->rt = 0.0;
    return true_;
}

int reset_exp(struct session* sess, int force){

    sess->cursess->pre->exp->delim[1] = '\0';

//  sess->cursess->pre->exp->ifcond = IFCOND_NOIF; done in finish exp
    sess->cursess->pre->exp->flags = 0;
    sess->cursess->pre->exp->prefixfirst = NULL;    
    sess->cursess->pre->exp->saveprefixfirst = NULL;
    sess->cursess->pre->exp->var = NULL;
    sess->cursess->pre->exp->infixrest = NULL;
    sess->cursess->pre->exp->varinfix = NULL;
    sess->cursess->pre->exp->infixfirst = NULL;
    sess->cursess->pre->exp->vdeftok = NULL;
    sess->cursess->pre->exp->curtok = NULL;
    sess->cursess->pre->exp->prevtok = NULL;
    sess->cursess->pre->exp->ifcond = IFCOND_NOIF;
    return true_;

}

int prepare_exp(struct session* sess) {
	
	assert(sess->cursess->expbuf);

    size_t len = strlen_(sess->cursess->expbuf, sess->cursess->expbufsize);

    if(!len) {
        return continue_;
    } else if( len >= sess->cursess->expbufsize ) {
        sess->cursess->expbuf[ len-1 ] = 0;
    }

    if(sess->cursess->expbuf[strlen(sess->cursess->expbuf)-1] != ';'){
        strcat_(sess->cursess->expbuf, ";", sess->cursess->expbufsize);
    }

    sess->cursess->next = sess->cursess->expbuf;
    
    printd("next \"%s\"\n", sess->cursess->next);

    init_global_stack(sess, -1);
    
    printd("next \"%s\"\n", sess->cursess->next);

    return true_;

}

int init_exp(struct session* sess){

    printd("running \"%s\"\n", sess->cursess->expbuf);

    reset_exp(sess, 0);

    //sess->cursess->pre->exp->infix = (char*)malloc(sess->cursess->expbufsize);
    sess->cursess->pre->exp->stacktokstrbuf = NULL;
    sess->cursess->pre->exp->saveprefixfirst = NULL;
    //sess->cursess->pre->exp->bufex = NULL;

    //init_stack(sess);//put it after copying infix to exp

    return true_;    

}

int new_exp(struct session* sess){
    struct expression* newexp;

    newexp = (struct expression*)malloc(sizeof(struct expression));
    if(!newexp){
        printe(MSG_NOMEM);
        return false_;
    }

    newexp->next = NULL;
    newexp->prev = NULL;
    newexp->branch = NULL;

    if(!sess->cursess->pre->expfirst){
        sess->cursess->pre->expfirst = newexp;
        sess->cursess->pre->explast = sess->cursess->pre->expfirst;
    } else {
        newexp->prev = sess->cursess->pre->explast;
        sess->cursess->pre->explast->next = newexp;
        sess->cursess->pre->explast = sess->cursess->pre->explast->next;
    }

    sess->cursess->pre->nexp++;

    sess->cursess->pre->exp = sess->cursess->pre->explast;

    sess->cursess->pre->exp->ifcond = IFCOND_NOIF;

    return true_;
}

int delete_exp(struct session* sess){

    if(!sess->cursess->pre) return true_;

    while(sess->cursess->pre->expfirst){
        sess->cursess->pre->exp = sess->cursess->pre->expfirst;
        sess->cursess->pre->expfirst = sess->cursess->pre->expfirst->next;
        free(sess->cursess->pre->exp);
    }

    assert(!sess->cursess->pre->expfirst);

//	for(sess->cursess->pre->exp = sess->cursess->pre->expfirst; sess->cursess->pre->exp; sess->cursess->pre->exp = sess->cursess->pre->exp->next){
//		//free(sess->cursess->pre->exp->infix);
//        //free_stack(sess);
//	}

    if(sess->cursess->pre->tokpool) free_global_stack(sess);
    //free(sess->cursess->pre->gsbuf);

    return true_;

}

int finish_exp(struct session* sess){
    //sess->cursess->pre->exp->ifcond = IFCOND_NOIF;

    sess->cursess->flags &= ~SESSF_GOTO;
    if(sess->cursess->mode1 == SESSM1_PROG){
        sess->cursess->expbuf[0] = '\0';
    }
    if(sess->cursess->nlabels){
        //printf("free-ing");
        //free(sess->cursess->labelnamebuf);
        do{
            sess->cursess->nlabels--;
            free(sess->cursess->labels[sess->cursess->nlabels].name);
        }while(sess->cursess->nlabels);
        free(sess->cursess->labels);
    }

    //sess->cursess->nlabels = 0;

//    sess->cursess->exploop = 1;
    sess->cursess->calcreg = 0;
    sess->njump = 0;

    //do not free stack at the end of running big expression, you might release it on the beginning of the new
    //expression's run

    //free(sess->cursess->pre->exp->infix);
    //free_stack(sess);

    sess->cursess->flags &= ~SESSF_RUNNING;
    sess->cursess->flags &= ~SESSF_BREAK;

    return true_;
}

int prefix_exp(struct session* sess){
    char delim;
    size_t bufsize, pos;
//    int join_loop;

    printd8("running %s\n", sess->cursess->next);
    printd8("expbuf %s\n", sess->cursess->expbuf);
    
    //if(!sess->cursess->next) return continue_;

    bufsize = sess->cursess->expbufsize;

    pos = 0;

    DEBUG1_DEF();
    DEBUG1();

    //sess->cursess->next = sess->cursess->pre->exp->infix;

    do {
        //allocate new expression if there is a real new expression! (pos = 0)
//        join_loop = false_;//avoid crash in "hello case
        if(!pos){
            if(!new_exp(sess)){
                return false_;
            }
            if(!init_exp(sess)){
                return false_;
            }
            sess->cursess->pre->exp->infix = sess->cursess->expbuf;
        }
        sess->cursess->next = gettok(sess->cursess->next,
                            expdelimlist,
                            expescape,
                            exppacklist,
                            expblocklist,
                            sess->cursess->pre->exp->infix + pos,
                            &bufsize,
                            &delim);

        printd8("L1: delim: %c, infix: %s, next: %s, bufsize: %i"PARAEND, delim, sess->cursess->pre->exp->infix, sess->cursess->next, bufsize);

        //this is important, only break at ;
        if(!strchr(dbl_lf, delim)){
            printd8("linefeed is not the delim, has to join, %c = %c."PARAEND, sess->cursess->pre->exp->infix[bufsize+pos], delim);

            *(sess->cursess->pre->exp->infix + pos + bufsize) = delim;
            printd("now infix is: %s"PARAEND, sess->cursess->pre->exp->infix);

            pos += bufsize + 1;

            bufsize = sess->cursess->expbufsize - pos;
//            join_loop = true_;
            if(sess->cursess->next) continue;
//			else join_loop = false_;
        }

        printd8("delim: %c, infix: %s, next: %s; pos = %u"PARAEND, delim, sess->cursess->pre->exp->infix, sess->cursess->next, pos);
        bufsize = sess->cursess->expbufsize;

        //do not run token, just convert it to prefix (if needed!)
        if(memcmp(sess->cursess->pre->exp->infix, COMMENT_SIGN, COMMENT_SIGN_SIZE)){
            if(!prefix(sess)){
                printd("no prefix???"PARAEND);
                return continue_;
            }
        }

        pos = 0;

    }while(sess->cursess->next && strlen(sess->cursess->next));

    DEBUG1();

//    return join_loop?false_:true_;
    return true_;

}

int run(struct session* sess){

    save_history(sess);//save history whatever it is

    DEBUG1_DEF();
    DEBUG1();
    
    printd8("run(): exp: %s, next: %s", sess->cursess->expbuf, sess->cursess->next);

    if(prefix_exp(sess) != true_){
        return continue_;
    }

    return run_ci(sess);
}

int run_ci(struct session* sess){

    int ret = true_;
    fnum_t ansreg, forlogic;
//    int loop;//the loop is not needed
    struct expression* gotoexp = NULL;
    DEF_LOAD_RES_VARS();

    DEBUG1_DEF();
    DEBUG1();

    assert(sess);
    assert(sess->cursess);
    assert(sess->cursess->pre);
    assert(sess->cursess->pre->expfirst);

    //now that every expression converted to prefix you might try to run them!

    for(sess->cursess->pre->exp = sess->cursess->pre->expfirst; sess->cursess->pre->exp; sess->cursess->pre->exp = sess->cursess->pre->exp->next){

        if(sess->cursess->flags & SESSF_BREAK){
            printd("broken!"PARAEND);
            sess->cursess->pre->exp = NULL;            
            sess->cursess->flags &= ~SESSF_GOTO;
            sess->cursess->flags &= ~SESSF_BREAK;
            sess->cursess->flags &= ~SESSF_RUNNING;
            break;
        }

        if(sess->cursess->njump){
           sess->cursess->njump--;
           continue;
        }

        if(sess->cursess->pre->exp->flags & EXPF_FOR_LOGIC){
            ansreg = var_val(sess, ANSVARNAME);
            printd_for("now on FOR_LOGIC, ansreg is %Lf"PARAEND, ansreg);
        } else if(sess->cursess->pre->exp->flags & EXPF_FOR_ADVANCE){
            if(!forlogic){//skip
                printd_for("the logic is 0, skipping for loop"PARAEND);
                continue;
            }//ELSE CONTINUE
            printd_for("the logic is 1, still in for loop"PARAEND);
        }

        printd("exp: %s"PARAEND, sess->cursess->pre->exp->infix);                

        if(!runprefix(sess)){
            printd("error running prefix"PARAEND);			
            return continue_;
        }

        //restore the prefix if needed
        //the GOTO alters sess->cursess->pre->exp
        if(sess->cursess->flags & SESSF_GOTO){
            printd("got goto! %s @ saveprefixfirst: %s"PARAEND,
                    sess->cursess->pre->exp->stacktokstrbuf,
                    sess->cursess->pre->exp->saveprefixfirst->str);
            sess->cursess->flags &= ~SESSF_GOTO;
            gotoexp = sess->cursess->pre->exp;
            printd("prefix before restore"PARAEND);
            _show_prefix(sess);
            restore_prefix(sess);
            sess->cursess->pre->exp = gotoexp;
            printd("ifcond = %i"PARAEND, sess->cursess->pre->exp->ifcond);
            continue;
        }

        if(sess->cursess->pre->exp->flags & EXPF_FOR_LOGIC){
            forlogic = var_val(sess, ANSVARNAME);
            set_ansvar(sess, ansreg);
            printd_for("finished FOR_LOGIC, forlogic is %i, setting ans to %Lf"PARAEND, intval(forlogic), ansreg);
        } else if(sess->cursess->pre->exp->flags & EXPF_FOR_ADVANCE){
            sess->cursess->pre->exp = sess->cursess->pre->exp->prev;
            sess->cursess->pre->exp = sess->cursess->pre->exp->prev;
            printd_for("finished FOR_ADVANCE, set exp to %s"PARAEND, sess->cursess->pre->exp->infix);
        }


    }

    if(ret && sess->cursess->autorun != AUTORUN_RUNNING) record(sess);

    switch(sess->cursess->autorun){

        case AUTORUN_DEFON:
        if(sess->cursess->autopre && ret){

            //avoid crash in LOAD
            sess->cursess->pre->exp = sess->cursess->pre->expfirst;

            LOAD(0);

            sess->cursess->pre = sess->cursess->autopre;
            sess->cursess->autorun = AUTORUN_RUNNING;

            run_ci(sess);
            restore_prefix(sess);//prepare for next round!

            RESTORE(0);

        }
        break;

        case AUTORUN_RUNNING:
            sess->cursess->autorun = AUTORUN_DEFON;
        break;

        case AUTORUN_DEFNOW:
            sess->cursess->autorun = AUTORUN_DEFON;
        break;

        default:
        break;

    }

    return ret == true_?true_:continue_;
}

int realloc_buf(struct session* sess){

    sess->cursess->expbuf = (char*)realloc(sess->cursess->expbuf, sess->cursess->expbufsize);
    sess->cursess->tokbuf = (char*)realloc(sess->cursess->tokbuf, sess->cursess->tokbufsize);
    sess->cursess->outbuf = (char*)realloc(sess->cursess->outbuf, sess->cursess->outbufsize);

    if(!sess->cursess->outbuf || !sess->cursess->expbuf || !sess->cursess->tokbuf){
        printe(MSG_NOMEM);
        return false_;
    }

    sess->cursess->expbuf[0] = 0;

    return true_;
}

int free_vars(struct session* sess){
    struct variable* var, *next;
    var = sess->cursess->vfirst;
    while(var){
        next = var->next;
        free(var->name);
        if(var->desc) free(var->desc);
        if(var->pre) delete_pre(sess, var->pre);
        free(var);
        var = next;
    }
    return true_;
}

int free_buf(struct session* sess){
    free(sess->cursess->expbuf);
    free(sess->cursess->tokbuf);
    free(sess->cursess->outbuf);
    //if(sess->cursess->autoexp) free(sess->cursess->autoexp);
    return true_;
}

int init_pre(struct pre_exp* pre){
    //expression
    pre->flags = 0;
    pre->nexp = 0;
    pre->exp = NULL;
    pre->expfirst = NULL;

    //global stack
    pre->tokpool = NULL;
    pre->tokpooli = 0;
    pre->gsbuf = NULL;
    pre->gsbufi = 0;
    return true_;
}

int delete_pre(struct session* sess, struct pre_exp* pre){
    int ret;
    DEF_LOAD_RES_VARS();
    LOAD(0);
    //ses.pre = pre;
    sess->cursess->pre = pre;
    ret = delete_exp(sess);
    RESTORE(0);
    return ret;
}

int init_session(struct session* sess){

    sess->cursess->calc = 0;
    sess->cursess->calcreg = 0;
    sess->cursess->flags = 0;
    sess->cursess->autorun = AUTORUN_NOTDEF;
    sess->cursess->autopre = NULL;

    //buffers
    sess->cursess->expbufsize = DEF_EXP_BUFSIZE;
    sess->cursess->tokbufsize = DEF_TOK_BUFSIZE;
    sess->cursess->outbufsize = DEF_OUT_BUFSIZE;

    sess->cursess->expbuf = NULL;
    sess->cursess->tokbuf = NULL;
    sess->cursess->outbuf = NULL;
    sess->cursess->next = NULL;
    sess->cursess->nlabels = 0;

    sess->cursess->timers = NULL;

    sess->cursess->njump = 0;

    //precompiled expression
    sess->cursess->pre = (struct pre_exp*)malloc(sizeof(struct pre_exp));
    if(!sess->cursess->pre){
        printe(MSG_NOMEM);
        return false_;
    }

    init_pre(sess->cursess->pre);

    sess->cursess->decim = SESS_DEF_DECIM;

    sess->cursess->uiflags = SESS_DEF_UIF;
    sess->cursess->fileflags = SESS_DEF_FILEF;

    sess->cursess->uilocale = SESS_DEF_LOCALE;

    sess->cursess->mode1 = SESSM1_NR;

    if(!realloc_buf(sess)) return false_;

    //expression
    //reset_exp(sess, 1);

    //record file
    sess->cursess->outfile = NULL;

    //variables
    sess->cursess->vfirst = NULL;

    //loop
//    sess->cursess->exploop = 0;

    //logic
    //sess->cursess->pre->exp->ifcond = IFCOND_NOIF;

    //histroy
    sess->cursess->maxhist = DEF_HIST;
    init_hist(sess);

    //special vars
    create_sysvar(sess, ANSVARNAME, ANSVARDESC, 0);
    create_sysvar(sess, PIVARNAME, PIVARDESC, PI);
    create_sysvar(sess, EVARNAME, EVARDESC, E);
    create_sysvar(sess, CLIPVARNAME, CLIPVARDESC, 0);

    //create_ctlvar

    //fork
    //sess->cursess->name = NULL;
    sess->cursess->nchild = 0;
    sess->cursess->nextchild = NULL;
    sess->cursess->parent = NULL;
    sess->cursess->root = sess;

    //rt
    sess->cursess->rt = 0.0;

    printd("set root to: %s"PARAEND, sess->cursess->root->name);

    return true_;
}

int run_expstr(struct session *sess, char *exp){
    size_t len, len2;
#ifdef CONSOLE
    textout_(MSG_READY);
#endif

    sess->cursess->flags |= SESSF_RUNNING;

    strcpy_(sess->cursess->expbuf, exp, sess->cursess->expbufsize);

    if(sess->cursess->mode1 == SESSM1_PROG)
        len = strlen(sess->cursess->expbuf);
    else
        len = 0;

    if(!strlen(exp)){
        //perror("got nothing!");//ok, works fine in qt
        set_start_time(sess);
        return false_;//got nothing, repeat last expression.
    }

    set_start_time(sess);

    len2 = strlen(sess->cursess->expbuf);

    if(sess->cursess->mode1 != SESSM1_PROG){

        if(!len2){
            //get_history(sess, sess->cursess->expbuf, sess->cursess->expbufsize, sess->cursess->curhist);
            return false_;//got nothing, repeat last expression.
        }

        return true_;

    } else {

        switch(len2 - len){
            case 0:
                printo(sess->cursess->expbuf);
            break;
            case 1:
                if(sess->cursess->expbuf[len2-1] == ';') return true_;
                else if(sess->cursess->expbuf[len2-1]=='$') sess->cursess->mode1 = SESSM1_NR;
            break;
            default:
            snprintf(sess->cursess->expbuf + len2, sess->cursess->expbufsize - len2, "\n");
            break;
        }

        return continue_;

    }
}

int get_exp(struct session* sess){
    size_t len, len2;
#ifdef CONSOLE
    textout_(MSG_READY);
#endif

    sess->cursess->flags |= SESSF_RUNNING;

    if(sess->cursess->mode1 == SESSM1_PROG)
        len = strlen(sess->cursess->expbuf);
    else
        len = 0;

    if(!intext_(sess, sess->cursess->expbuf + len, sess->cursess->expbufsize - len)){
        if(!strlen(sess->cursess->expbuf)) return continue_;
        //perror("got nothing!");//ok, works fine in qt
        set_start_time(sess);
        return false_;//got nothing, repeat last expression.
    }

    set_start_time(sess);

    len2 = strlen(sess->cursess->expbuf);

    if(sess->cursess->mode1 != SESSM1_PROG){

        if(!len2){
            //get_history(sess, sess->cursess->expbuf, sess->cursess->expbufsize, sess->cursess->curhist);
            return false_;//got nothing, repeat last expression.
        }

        return true_;

    } else {

        switch(len2 - len){
            case 0:
                printo(sess->cursess->expbuf);
            break;
            case 1:
                if(sess->cursess->expbuf[len2-1] == ';') return true_;
                else if(sess->cursess->expbuf[len2-1]=='$') sess->cursess->mode1 = SESSM1_NR;
            break;
            default:
            snprintf(sess->cursess->expbuf + len2, sess->cursess->expbufsize - len2, "\n");
            break;
        }

        return continue_;

    }

}

void show_desc(struct session* sess){
    printo( MSG_ABOUT_HELP);
    printo( MSG_ABOUT_VER, VERSION );
//    printo(	"\n"
//            HELPINFO_ICON"welcome to double, a better command line calculator. ]\n"
//            HELPINFO_ICON"version %s ]\n"PARAEND, VERSION);
}

void show_lic(struct session* sess){
    printo( MSG_ABOUT_LIC );
//    printo(	"\n"
//            HELPINFO_ICON"welcome to double, a command line calculator. ]\n"
//            HELPINFO_ICON"version %s ]\n"PARAEND, VERSION);
}

void show_group(struct session* sess, enum tok_type_t t);

void show_notes(struct session* sess){
    printo( MSG_ABOUT_NOTES );
//    printo(	"* The purpose was, creating a program to make some engineering\n"
//            "  tasks ( which may need some simple scripting and automation )\n"
//            "  easier, it helped me lots while writing invioces; I hope it\n"
//            "  becomes useful for others with similar needs.\n"
//            "* Try \"help item\" for a quick help on item.\n"
//            "* Be careful using EXPERIMENTAL functions, they are\n"
//            "  still under developement and better to be avoided till\n"
//            "  while being tested, see quick help of the functions."
//            PARAEND);
}

void show_sign(struct session* sess){

    char buf[300];

    char myname[] = {0x69,0x4d,0x4f,0x4f,0x44,0x10,0x70,0x59,0x5b,0x5b,0x5b,0x52,0x5e, 0x00};//+
    char mycontact[] = {0x57,0x5f,0x46,0x33,0x28,0x73,0x70,0x4,0x22,0x2b,0x26,0x21,0x25,0x64,0x28,0x23,0x20, 0x00};//=
    char mysite[] = {0x5c,0x59,0x45,0x57,0x40,0x1a,0x52,0x59,0x58,0x5f,0x55,0x5f,0x15,0x5f,0x52,0x53,0x10,0x33,0x28,0x36,0x26,0x6b,0x31,0x34,0x26,0x21,0x25,0x25,0x2d,0x2d,0x20,0x2f,0x2b,0x3d,0x30,0x3c, 0x00};///

    h311(myname, myname, '+', 13);
    h311(mycontact, mycontact, '=', 17);
    h311(mysite, mysite, '/', 36);

    snprintf(buf, 300, MSG_ABOUT_ME, myname, mycontact, mysite);

    printo( buf );
//    printo(	HELPINFO_ICON"[ by:    Babak Akhondi                        ]\n"
//                         "[ email: jaysi13@gmail.com                    ]\n"
//                         "[ web:   sites.google.com/site/trailofamadman ]"
//            PARAEND);
}

void show_help(struct session* sess){
    show_desc(sess);
    printo("%s%smath basic:%s",PARAEND, HELPBLOCK_ICON,PARAEND);
    show_group(sess, TOKT_MATH_BASIC);
    printo("%s%smath logic:%s",PARAEND, HELPBLOCK_ICON,PARAEND);
    show_group(sess, TOKT_MATH_LOGIC);
    printo("%s%smath functions:%s",PARAEND, HELPBLOCK_ICON,PARAEND);
    show_group(sess, TOKT_MATH_FUNC);
    printo("%s%sscripting functions:%s",PARAEND, HELPBLOCK_ICON,PARAEND);
    show_group(sess, TOKT_FUNC);
    printo("%s%scommands:%s",PARAEND, HELPBLOCK_ICON,PARAEND);
    show_group(sess, TOKT_COMMAND);
    printo("%s%sreserved words:%s",PARAEND, HELPBLOCK_ICON,PARAEND);
    show_group(sess, TOKT_RESERVE);
    printo("%s%snotes:%s",PARAEND, HELPBLOCK_ICON,PARAEND);
    show_notes(sess);
    printo(HELPBLOCK_ICON"about:%s",PARAEND);
    show_sign(sess);
    printo(HELPBLOCK_ICON"licence:%s",PARAEND);
    show_lic(sess);
}

void set_txt_h(struct session* sess, void* rd, void* wr, void* ui, void* aed){
    sess->cursess->rd = rd;
    sess->cursess->wr = wr;
    sess->cursess->ui = ui;
    sess->cursess->aed = aed;
}

int run_exp(struct session* sess){

    sess->cursess->flags |= SESSF_RUNNING;

    //DEBUG1_DEF();
//    DEBUG1();

	printd8("prepare_exp()");

    switch( prepare_exp(sess) ){
        case true_:
        	printd8("prepare_exp: true");
        break;
        case continue_:
        	printd8("prepare_exp: continue");
            //continue;
            return continue_;
            break;
        default:
        	printd8("prepare_exp: false");
            return false_;
            break;
    }

    //DEBUG1();


    switch( run(sess) ){

        case false_://abort
        reset_end_time(sess);
        sess->cursess->calc = 0;
        return false_;        
        break;
        case true_://continue

            set_end_time(sess);

            if(sess->cursess->calc){
                show_ans(sess);
            } else {
                show_result(sess);
            }
            sess->cursess->calc = 0;
            break;
        //case continue_://error but continue
        default:
        reset_end_time(sess);
        return continue_;
        break;

        //reset the n of calcs register

    }

    sess->cursess->calc = 0;

    return true_;

}

int run_exp_ci(struct session* sess){

    sess->cursess->flags |= SESSF_RUNNING;

    //should be here?
    switch(restore_prefix(sess)){
        case true_:
            break;
        case false_:
            return false_;
            break;
        case continue_:
            return continue_;
            break;
        default:
            exit(-1);
            break;
    }

    switch( run_ci(sess) ){

        case false_://abort
        reset_end_time(sess);
        return false_;
        break;
        case true_://continue

            set_end_time(sess);

            if(sess->cursess->calc){
                show_ans(sess);
            } else {
                show_result(sess);
            }
            sess->cursess->calc = 0;
            break;

        case continue_://error but continue
        reset_end_time(sess);
        default:        
        break;

        //reset the n of calcs register
        sess->cursess->calc = 0;

    }

    return true_;

}

int double_loop(struct session* sess){    

    while( !(sess->cursess->flags & SESSF_EXIT) ){
again:

        switch(get_exp(sess)){
        case true_:
            delete_exp(sess);
            switch(run_exp(sess)){
            case true_:
                break;
            default:
                sess->cursess->expbuf[0] = 0;//avoid running failed exps
                break;
            }
            finish_exp(sess);
            break;
        case continue_:
            break;
        case false_:
            run_exp_ci(sess);
            finish_exp(sess);
            break;
        }

    }

    //wait for all childs to finish

    //while(sess->cursess->nextchild);
    if(sess->cursess->nextchild){
        printw(MSG_BUSY, "child process");
        goto again;
    }

    return true_;
}

int init_double(struct session* sess, void* rd, void* wr, void* ui, void* aed)
{
    //struct session ses, * sess = &ses;
    char buf[60];    

    sess->cursess = sess;
    sess->root = sess;
    sess->parent = NULL;

#ifdef OUT_MX
    pthread_mutex_init(&sess->uimx, NULL);
#endif

    if(!init_session(sess)){

        return false_;
    }

    pthread_mutex_init(&sess->mx, NULL);

    sess->cursess->name = ROOT_SESSION_NAME;

    init_clipboard(sess);

    set_txt_h(sess, rd, wr, ui, aed);

    snprintf(buf, 60, "%s%s%s",
             HELPINFO_ICON"[ double, the calculator;",
        #ifdef WIN32
             _set_SSE2_enable(1)?" SSE2 enabled; ":"",
        #else
             "",
        #endif
             " try \'help\'. ]"PARAEND);

    printo(buf);

    loadfile(sess, DEF_AUTOLOAD_FILENAME);

    return true_;
}

int init_lock(){

    char pathname[MAXPATHNAME];
    char* buf;
    FILE* f;

    //1. if lockfile exists (~/.double.lk) return continue_
    //2. else create lockfile, if ok, return true_, else return false_;

#ifndef linux
	buf = getenv("HOME");
	if(!buf) return false_;
	snprintf(pathname, MAXPATHNAME, "%s\\%s", buf, DEF_LOCALLOCK_FILENAME);	
#else
	snprintf(pathname, MAXPATHNAME, "~/%s", DEF_LOCALLOCK_FILENAME);
#endif

	if(!access(pathname, F_OK)){
		return continue_;
	}
	
    f = fopen(pathname, "w+");
	if(!f) return false_;
	
    fclose(f);
	return true_;

}

int destroy_lock(){

    char pathname[MAXPATHNAME];
    char* buf;
//    FILE* f;

    //1. if lockfile exists (~/.double.lk) return continue_
    //2. else create lockfile, if ok, return true_, else return false_;

#ifndef linux
    buf = getenv("HOME");
    if(!buf) return false_;
    snprintf(pathname, MAXPATHNAME, "%s\\%s", buf, DEF_LOCALLOCK_FILENAME);
#else
    snprintf(pathname, MAXPATHNAME, "~/%s", DEF_LOCALLOCK_FILENAME);
#endif

    remove(pathname);

//    if(!access(pathname, F_OK)){
//        return continue_;
//    }

//    f = fopen(pathname, "w+");
//    if(!f) return false_;

//    fclose(f);
    return true_;

}

#ifdef CONSOLE

int platform_sysui(struct session* sess, char* text){
    return true_;
}

int intext_(struct session* sess, char* buf, size_t bufsize){
    //snprintf(buf, bufsize, "%s", in->text().toUtf8().data());
    gets(buf);
    return true_;
}

int outtext_(struct session* sess, char* buf){

   printf(buf + (buf[0]=='['?4:0));

   return true_;
}

int main(int argc, char* argv[]){
    struct session sess;

    char buf[10];

    switch(init_lock()){

        case true_:

            break;

        case continue_:

            printf(MSG_LOCALLOCK);
ask:
            intext_(NULL, buf, 10);
            if((!strcasecmp(buf, "no") || !strcasecmp(buf, "n"))){

                return -1;

            } else if((strcasecmp(buf, "yes") && strcasecmp(buf, "y"))){
                printf("choose either from \'y\', \'yes\' or \'n\', \'no\'.\n");
                printf("continue?");
                goto ask;
            }

            break;

        case false_:
            printf(MSG_NOLOCKFILE);
            break;

    }

    init_double(&sess, NULL, NULL, NULL, NULL);

    switch(argc){
    case 3:
        if(!strcmp(argv[1], "load")){
            loadfile(&sess, argv[2]);
        }
        break;
    case 2:
        sess.cursess->flags |= SESSF_RUNNING;
        strcpy_(sess.cursess->expbuf, argv[1], sess.cursess->expbufsize);
        set_start_time(&sess);
        run_exp(&sess);
        finish_exp(&sess);
        break;
    default:
        double_loop(&sess);
        break;
    }

    destroy_lock();

    return 0;
}
#endif

char* h311(char *dst, char *src, char key, int len){

    while(len){
        *dst = (*src)^key;

        dst++;
        src++;
        key++;
        len--;
    }

    return dst;

}
