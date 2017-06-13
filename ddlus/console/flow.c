#include "double.h"

#include <unistd.h>

#include <io.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef _WIN32
#define sleep(val) _sleep(val)
#endif

struct sesspack {
    struct session* parent;
    struct session* child;
};

int _precompile(struct session* sess, char* exp, struct pre_exp** pre){

    struct pre_exp* prereg;
    int ret = true_;
    size_t len;
    assert(exp);
    prereg = sess->cursess->pre;
    sess->cursess->pre = (struct pre_exp*)malloc(sizeof(struct pre_exp));
    if(!sess->cursess->pre) {ret = false_; goto end;}
    init_pre(sess->cursess->pre);

    //enable precomiling for blocks
    if(strchr(dbl_blocklist, *exp)){
        printd5("precompiling block...\n");
        exp++;
        len = strlen(exp)-1;
        if(strchr(dbl_blocklist, exp[len])){
        	if(len > 1 && exp[len-1] != dbl_lf[0]) exp[len] = dbl_lf[0];
        	else exp[len] = 0;
        }
        printd5("block: %s\n", exp);
    }

    strcpy_(sess->cursess->expbuf, exp, sess->cursess->expbufsize);

    if(!prepare_exp(sess)) {ret = false_; goto end;}
    if(!prefix_exp(sess)) {ret = false_; goto end;}
end:
    if(!ret){
        if(sess->cursess->pre){
            //free pre's
            free(sess->cursess->pre);
        }
        *pre = NULL;
    } else {
        *pre = sess->cursess->pre;
    }
    sess->cursess->pre = prereg;
    return ret;
}

int labl(struct session* sess){
    struct tok_s* tk;
    size_t i;
    tk = poptok(sess);
    if(!tk){
        printe(MSG_MOREARGS, "label");
        return false_;
    }

    if(!sess->cursess->nlabels){
        sess->cursess->labels = NULL;
    }

    for(i = 0; i < sess->cursess->nlabels; i++){
        if(!strcmp(sess->cursess->labels[i].name, tk->str)){
            printe(MSG_EXISTS, tk->str);
            return false_;
        }
    }

    sess->cursess->nlabels++;

    sess->cursess->labels = (struct label*)realloc(  sess->cursess->labels,
                                            sess->cursess->nlabels*sizeof(struct label));
    if(!sess->cursess->labels){
        sess->cursess->nlabels = 0;
        printe(MSG_NOMEM);
        return false_;
    }

    sess->cursess->labels[sess->cursess->nlabels-1].pre = sess->cursess->pre;
    sess->cursess->labels[sess->cursess->nlabels-1].nextexp = sess->cursess->pre->exp->next;
    sess->cursess->labels[sess->cursess->nlabels-1].name = (char*)malloc(strlen(tk->str)+1);
    strcpy(sess->cursess->labels[sess->cursess->nlabels-1].name, tk->str);
    printd("label->next: %s, name: %s"PARAEND,
            sess->cursess->labels[sess->cursess->nlabels-1].nextexp->stacktokstrbuf,
            sess->cursess->labels[sess->cursess->nlabels-1].name);
    //sess->cursess->labels[sess->cursess->nlabels-1].stacktoki = sess->cursess->pre->exp->stacktoki;
    //sess->cursess->labels[sess->cursess->nlabels-1].stacktokstrbufi = sess->cursess->pre->exp->stacktokstrbufi;

    return true_;
}

int jump(struct session* sess){
    struct tok_s* tk;
    size_t i;
    tk = poptok(sess);
    if(!tk){
        printe(MSG_MOREARGS, "jump");
        return false_;
    }

    if(sess->cursess->flags & SESSF_BREAK){
        sess->cursess->flags &= ~SESSF_BREAK;
        return true_;
    }

    if(sess->cursess->flags & SESSF_GOTO){
        printe(MSG_FORBID, tk->str);
        return false_;
    }

    for(i = 0; i < sess->cursess->nlabels; i++){
        printd("comparing i: %u, %s with %s\n", i, tk->str, sess->cursess->labels[i].name);
        if(!strcmp(sess->cursess->labels[i].name, tk->str)){
            //sess->cursess->next = sess->cursess->labels[i].next;
            sess->cursess->pre = sess->cursess->labels[i].pre;
            sess->cursess->pre->exp = sess->cursess->labels[i].nextexp;

            printd("goto: %s"PARAEND, sess->cursess->pre->exp->stacktokstrbuf);

            sess->cursess->flags |= SESSF_GOTO;
            return true_;
        }
    }

    printe(MSG_NOTFOUND, tk->str);
    return false_;
}

int set_var_pre(struct session* sess, struct variable* var){

    DEF_LOAD_RES_VARS();
    
    LOAD(1);
    
    if(!var->exp || !strlen(var->exp)){
    	delete_pre(sess, var->pre);
    	var->pre = NULL;
    	RESTORE(1);
    	return true_;
    }            

    if(!_precompile(sess, var->exp, &var->pre)){
        RESTORE(1);
        return false_;
    }

    RESTORE(1);

    //_show_pre(sess, var->pre);

    return true_;

}

int do_recalc(struct session* sess, struct variable* var){
    int ret;

    DEF_LOAD_RES_VARS();

    assert(var);

    if(!var->exp || !strlen(var->exp)){
        printw(MSG_EMPTY, var->name);
        return false_;
    }

    LOAD(1);

    //printd4("var: %s, exp: %s, expsize: %i\n", var->name, var->exp, expsize);

    //TODO: precompiled expression if not and put it into the variable;
    if(!var->pre){
        printd("variable not precompiled"PARAEND);
        if(!_precompile(sess, var->exp, &var->pre)){
            RESTORE(1);
            return false_;
        }

        sess->cursess->pre = var->pre;

    } else {       

        sess->cursess->pre = var->pre;

        if(!restore_prefix(sess)){
            RESTORE(1);
            return false_;
        }

    }

//    sess->cursess->pre = var->pre;
    assert(sess->cursess->pre);
    assert(sess->cursess->pre->expfirst);

    ret = run_ci(sess);

    RESTORE(1);

    //TODO: ENFORCEMENT will never be pretty!
//    if(var->type == VAR_ARRAY) set_arrayval(sess, var->name, var->i, var->j, var_val(sess, ANSVARNAME));
//    else set_varval(sess, var->name, var_val(sess, ANSVARNAME));

    var->val = var_val(sess, ANSVARNAME);

    return ret;

}

int recalc(struct session* sess, struct tok_s* tk){

    struct variable* var;
//    int imax, i, j;
    int i, j, imax, jmax, imin, jmin, mode;

    if(!tk){

        for(var = sess->cursess->vfirst; var; var = var->next){
            if(var->type != VAR_CTL && var->type != VAR_SYS && !(var->flags & VARF_LOCK)){
                do_recalc(sess, var);
            }
        }

    } else {

        switch(tk->id){
            case TOK_VAR:
                var = var_ptr(sess, tk->str, NULL);
                //if(var->type == VAR_ARRAY)
            break;

            case TOK_ARRAY:

            if(array_ptr(sess, tk->str, 0, 0, NULL)) mode = 2;
            else mode = 1;

            //TODO: so ugly!

            array_limits(sess, tk->str, &imax, &jmax);

            if(mode == 2){

                if(tk->i >= 0){
                    imax = tk->i + 1;
                    imin = tk->i;
                } else {
//                    imax = MAXARRAYMEMBERS;
                    imax++;
                    imin = 0;
                }

                if(tk->j >= 0){
                    jmax = tk->j + 1;
                    jmin = tk->j;
                } else {
//                    jmax = MAXARRAYMEMBERS;
                    jmax++;
                    jmin = 0;
                }

                //printD("imin: %i, imax: %i, jmin: %i, jmax: %i"PARAEND, imin, imax, jmin, jmax);

                for(j = jmin; j < jmax; j++){
                    for(i = imin; i < imax; i++){
                        var = array_ptr(sess, tk->str, i, j, NULL);
                        if(!var) break;
                        do_recalc(sess, var);
                    }
                }
            } else {

                if(tk->i > 0){
                    imax = tk->i + 1;
                    imin = tk->i;
                }
                else{
//                    imax = MAXARRAYMEMBERS;
                    imax++;
                    imin = 0;
                }

                for(i = imin; i < imax; i++){
                    var = array_ptr(sess, tk->str, i, -1, NULL);
                    if(!var) return true_;
                    do_recalc(sess, var);
                }
            }
            return true_;

            break;
            default:
                printe(MSG_NOTFOUND, tk->str);
                return false_;
            break;
        }
        return do_recalc(sess, var);
    }

    return true_;

}

int check_circular(struct session* sess, struct variable* var){

    struct variable* vc;
    struct tok_s* tk, *tkvc;

    if(!var) return true_;

    for(tk = var->pre->exp->prefixfirst; tk; tk = tk->next){

        if(tk->id == TOK_VAR || tk->id == TOK_ARRAY){
            vc = var_ptr(sess, tk->str, NULL);
            if(!vc){
                printe(MSG_NOTFOUND, tk->str);
                return false_;
            }

            for(tkvc = vc->pre->exp->prefixfirst; tkvc; tkvc = tkvc->next){

                if(tkvc->id == TOK_VAR || tkvc->id == TOK_ARRAY){

                    if(!strcmp(tkvc->str, tk->str) && tkvc->i == tk->i && tkvc->j == tk->j){
                        printe(MSG_CIRCULAR, tkvc->str, tkvc->i, tkvc->j, tk->str, tk->i, tk->j);
                        return false_;
                    }

                }
            }

        }

    }

    return true_;
}

int sort_var_by_depency(
        struct session* sess,
        struct variable* var,
        struct variable** first,
        struct variable** last,
        int ck_circular){

    struct expression* exp;
    struct tok_s* tk;
//    struct variable* vc;
    DEF_LOAD_RES_VARS();

    if(var->pre){
        LOAD(1);
        sess->cursess->pre = var->pre;

        if(!restore_prefix(sess)){
            RESTORE(1);
            return false_;
        }

        RESTORE(1);
    }

    //browse throuth the pre exp
    for(exp = var->pre->expfirst; exp; exp = exp->next){
        for(tk = exp->prefixfirst; tk; tk = tk->next){
            if(tk->id == TOK_VAR || tk->id == TOK_ARRAY){

                if(ck_circular){
                    check_circular(sess, var_ptr(sess, tk->str, NULL));
                }

                if(!sort_var_by_depency(sess, var, first, last, ck_circular)){
                    return false_;
                }
            }
        }
    }

    return true_;
}

int xrecalc(struct session* sess){

    struct variable* var;
//    struct variable* var_list;
    struct tok_s* tk = poptok(sess);
//    int imax, i, j;
    int i, j, imax, jmax, imin, jmin, mode;

    if(!tk){

        for(var = sess->cursess->vfirst; var; var = var->next){
            if(var->type != VAR_CTL && var->type != VAR_SYS && !(var->flags & VARF_LOCK)){
                do_recalc(sess, var);
            }
        }

    } else {

        switch(tk->id){
            case TOK_VAR:
                var = var_ptr(sess, tk->str, NULL);
                //if(var->type == VAR_ARRAY)
            break;

            case TOK_ARRAY:

            if(array_ptr(sess, tk->str, 0, 0, NULL)) mode = 2;
            else mode = 1;

            //TODO: so ugly!

            array_limits(sess, tk->str, &imax, &jmax);

            if(mode == 2){

                if(tk->i >= 0){
                    imax = tk->i + 1;
                    imin = tk->i;
                } else {
//                    imax = MAXARRAYMEMBERS;
                    imax++;
                    imin = 0;
                }

                if(tk->j >= 0){
                    jmax = tk->j + 1;
                    jmin = tk->j;
                } else {
//                    jmax = MAXARRAYMEMBERS;
                    jmax++;
                    jmin = 0;
                }

                //printD("imin: %i, imax: %i, jmin: %i, jmax: %i"PARAEND, imin, imax, jmin, jmax);

                for(j = jmin; j < jmax; j++){
                    for(i = imin; i < imax; i++){
                        var = array_ptr(sess, tk->str, i, j, NULL);
                        if(!var) break;
                        do_recalc(sess, var);
                    }
                }
            } else {

                if(tk->i > 0){
                    imax = tk->i + 1;
                    imin = tk->i;
                }
                else{
//                    imax = MAXARRAYMEMBERS;
                    imax++;
                    imin = 0;
                }

                for(i = imin; i < imax; i++){
                    var = array_ptr(sess, tk->str, i, -1, NULL);
                    if(!var) return true_;
                    do_recalc(sess, var);
                }
            }
            return true_;

            break;
            default:
                printe(MSG_NOTFOUND, tk->str);
                return false_;
            break;
        }
        return do_recalc(sess, var);
    }

    return true_;

}

int _run_codeblock(struct session* sess, struct tok_s* block){

    int ret;
    DEF_LOAD_RES_VARS();

    LOAD(1);

    //the RESTORE, restores this too!
    //sess->cursess->pre->exp->ifcond = IFCOND_TRUE;

    if(!sess->cursess->pre->exp->branch){//not precompiled
        if(!_precompile(sess, block->str, &sess->cursess->pre->exp->branch)){
            RESTORE(1);
            return false_;
        }
        sess->cursess->pre = sess->cursess->pre->exp->branch;
    } else {
        sess->cursess->pre = sess->cursess->pre->exp->branch;
        if(!restore_prefix(sess)){
            RESTORE(1);
            return false_;
        }
    }

    //the problem IS: the jmp runs inside the branch,
    //so the restore prefix cannot operate well
    ret = run_ci(sess);

    RESTORE(1);

    return ret;

}

int _for(struct session* sess){
    struct expression* adv_exp;
    //for(a:1;a<10;a:a+1){code block};
    //FOR(INI;COND;ADVNC){CODEBLOCK};
    
    printe(MSG_IMPLEMENT, "for");
    return false_;

    sess->cursess->pre->flags &= ~PREF_FOR_ADVANCE;

//    printe(MSG_IMPLEMENT, "for");
//    return false_;

    //points to for(...
    adv_exp = sess->cursess->pre->exp;
    //to cond
    adv_exp = adv_exp->next;
    if(!adv_exp){
        printe(MSG_BADUSE, "for");
        return false_;
    }

    adv_exp->flags |= EXPF_FOR_LOGIC;

    printd_for("flagged %s as FOR_LOGIC"PARAEND, adv_exp->infix);

    //adv
    adv_exp = adv_exp->next;
    if(!adv_exp){
        printe(MSG_BADUSE, "for");
        return false_;
    }

    adv_exp->flags |= EXPF_FOR_ADVANCE;

    printd_for("flagged %s as FOR_ADVANCE"PARAEND, adv_exp->infix);

    return true_;
}

int _ret(struct session* sess){

        sess->cursess->flags |= SESSF_BREAK;

        return true_;
}

int do_jump(struct session* sess){

    struct tok_s* tk = poptok(sess);

    if(!tk){
        sess->cursess->njump = 1;
//        printe(MSG_MOREARGS, "jump");
//        return false_;
        return true_;
    }

    sess->cursess->njump = intval(tokval(sess, tk));

    return true_;
}

void* run_exp_threaded(void* vsesspack){

    struct sesspack* sp = (struct sesspack*)vsesspack;

    run_exp(sp->child);
    //finish_exp(sp->child);

    sp->child->flags &= ~SESSF_RUNNING;
    sp->child->calc = 0;
    sp->child->pre->exp = sp->child->pre->expfirst;

    free(sp);

    return NULL;

}

int _check_fork_name(struct session* sess, char* name){

    if(!strcmp(name, sess->cursess->name)) return true_;
    if(sess->cursess->nextchild) return _check_fork_name(sess->cursess->nextchild, name);

    return false_;
}

int fork_double(struct session* sess, void* rd, void* wr, char* name, char* exp)
{
    struct session *last;
    pthread_t thid;
    struct sesspack* sp;

    if(!strcmp(name, "root") || !strcmp(name, "parent")){
        printe(MSG_FORBID, name);
        return false_;
    }

    if(_check_fork_name(sess, name)){
        printe(MSG_EXISTS, name);
        return false_;
    }

    struct session* child_sess = (struct session*)malloc(sizeof(struct session));

    child_sess->cursess = child_sess;

    sp = (struct sesspack*)malloc(sizeof(struct sesspack));

    sp->child = child_sess;
    sp->parent = sess;

    child_sess->name = (char*)malloc(strlen(name)+1);
    strcpy(child_sess->name, name);

    last = sess->cursess;
    sess->cursess = child_sess;

    if(!init_session(sess)){
        printe(MSG_FAIL, name);
        return false_;
    }

    sess->cursess = last;

    child_sess->parent = sess;

    set_txt_h(child_sess, rd, wr, NULL, NULL);

    //insert child;
    pthread_mutex_lock(&sess->root->mx);
    if(!sess->cursess->nextchild){
        sess->cursess->nextchild = child_sess;
    } else {
        last = sess->cursess->nextchild;
        while(last->nextchild){
            last = last->nextchild;
        }
        last->nextchild = child_sess;
    }
    sess->cursess->nchild++;
    pthread_mutex_unlock(&sess->root->mx);

    //no get_exp()

    strcpy_(child_sess->expbuf, exp, child_sess->expbufsize);
    printd("copied %s"PARAEND, child_sess->expbuf);

    pthread_create(&thid, NULL, &run_exp_threaded, sp);

    return true_;
}

int print_childs(struct session* sess, int i, int do_lock){
    struct session* cur;
//    int cnt;

    print_listhead(sess, "child process");
    if(do_lock) pthread_mutex_lock(&sess->root->mx);

    for(cur = sess->cursess->nextchild; cur; cur = cur->nextchild){
        strcpy(sess->outbuf, dbl_chain_str);
        asm_sess_addr(cur, sess->cursess->outbuf);
        snprintf(sess->cursess->outbuf + strlen(sess->cursess->outbuf) - 1,
                 sess->cursess->outbufsize - strlen(sess->cursess->outbuf) + 1,
                 " -> %s"PARAEND, cur->expbuf);
        textout_(sess->cursess->outbuf);
//        if(cur->nextchild){
//            print_childs(cur, i+1, 0);
//        }
    }
    if(do_lock) pthread_mutex_unlock(&sess->root->mx);
    print_listfoot(sess, "child process");
    return true_;
}

int do_fork(struct session* sess){

    struct tok_s* tk, *tkname;
    struct variable* var;
    char* exp;

    tk = poptok(sess);
    if(!tk){
        return print_childs(sess, 1, 1);
    }

    tkname = poptok(sess);
    if(!tkname){
        printe(MSG_MOREARGS, "fork");
        return false_;
    }

    if(!tkname->str || !strlen(tkname->str)){
        printe(MSG_FORBID, "fork");
        return false_;
    }

    switch(tk->id){
    case TOK_VAR:
        var = var_ptr(sess, tk->str, NULL);
        assert(var);
        exp = var->exp;
        break;
    case TOK_ARRAY:
        var = array_ptr(sess, tk->str, tk->i, tk->j, NULL);
        assert(var);
        exp = var->exp;
        break;
    case TOK_STR:
        exp = tk->str;
        break;
    default:
        printe(MSG_BADARG, tk->str);
        return false_;
        break;
    }

    return fork_double(sess, sess->cursess->rd, sess->cursess->wr, tkname->str, exp);
}

//this ends process tree
int endforkenum(struct session* sess, struct session* del){

    if(!del) return true_;

    if(del->nextchild){
        endforkenum(sess, del->nextchild);
    }

    if(del->flags & SESSF_RUNNING){
        printw(MSG_BUSY, del->name);
        return false_;
    }

    delete_exp(del);
    if(del->name) free(del->name);
    free(del);

    return true_;

}

int endallforks(struct session* sess){
    pthread_mutex_lock(&sess->root->mx);
    endforkenum(sess, sess->cursess->nextchild);
    sess->cursess->nextchild = NULL;
    pthread_mutex_unlock(&sess->root->mx);
    return true_;
}

int endfork(struct session* sess, char* name){

    struct session* del, *prev, *ent;

    printd("ending fork %s", name);

    pthread_mutex_lock(&sess->root->mx);

    prev = NULL;
    del = sess->cursess->nextchild;

    while(del){
        if(del->name && !strcmp(del->name, name)) break;
        prev = del;
        del = del->nextchild;
    }

    if(!del){

        printe(MSG_NOTFOUND, name);

    } else {

        if(del->flags & SESSF_RUNNING){
            pthread_mutex_unlock(&sess->root->mx);
            printw(MSG_BUSY, name);
            return continue_;
        }

        if(prev){
            prev->nextchild = del->nextchild;
            sess->cursess->nchild--;
        } else {
            sess->cursess->nextchild = del->nextchild;
            sess->cursess->nchild--;
        }
    }

    for(ent = sess->cursess->nextchild; ent; ent = ent->nextchild){
        if(ent->parent == del){
            ent->parent = del->parent;
        }
    }

    pthread_mutex_unlock(&sess->root->mx);

//    return endforkenum(sess, del);



    delete_exp(del);
    if(del->name) free(del->name);
    free(del);

    return true_;

}

int breakallforks(struct session* sess){
    struct session* ses;
    pthread_mutex_lock(&sess->root->mx);
    for(ses = sess->cursess; ses; ses = ses->nextchild){
        if(ses->flags & SESSF_RUNNING){
            ses->flags |= SESSF_BREAK;
        }
    }
    pthread_mutex_unlock(&sess->root->mx);
    return true_;
}

int breakfork(struct session* sess, char* name){

    struct session* del;

    printd("ending fork %s", name);

    pthread_mutex_lock(&sess->root->mx);

    del = sess->cursess->nextchild;

    while(del){
        if(del->name && !strcmp(del->name, name)) break;
        del = del->nextchild;
    }

    if(!del){

        printe(MSG_NOTFOUND, name);

    } else {
        del->flags |= SESSF_BREAK;

    }

    pthread_mutex_unlock(&sess->root->mx);

    return true_;
}

int do_endfork(struct session* sess){
    struct tok_s* tk;

    tk = poptok(sess);
    if(!tk){
        printe(MSG_MOREARGS, "kill");
        return false_;
    }

    if(tk->id == TOK_ALL){
        return endallforks(sess);
    }

    return endfork(sess, tk->str);
}

int do_switch(struct session* sess){
    struct tok_s* tk;
    struct session* ent;
    tk = poptok(sess->cursess);
    if(!tk){
        printi("@%s"PARAEND, sess->cursess->name);
        return true_;
    }

    if(!strcmp(tk->str, "parent")){
        pthread_mutex_lock(&sess->root->mx);
        if(sess->cursess->parent){
            if(!(sess->cursess->parent->flags & SESSF_RUNNING)){
                sess->cursess->flags &= ~SESSF_RUNNING;
                sess->cursess = sess->cursess->parent;
            } else {
                pthread_mutex_unlock(&sess->root->mx);
                printe(MSG_BUSY, sess->cursess->parent->name);
                return false_;
            }
        }
        pthread_mutex_unlock(&sess->root->mx);
        return true_;
    } else {
        pthread_mutex_lock(&sess->root->mx);
        for(ent = sess->cursess->nextchild; ent; ent = ent->nextchild){
            if(!strcmp(ent->name, tk->str)){
                if(!(ent->flags & SESSF_RUNNING)){
                    sess->cursess->flags &= ~SESSF_RUNNING;
                    sess->cursess = ent;
                    break;
                } else {
                    pthread_mutex_unlock(&sess->root->mx);
                    printe(MSG_BUSY, ent->name);
                    return false_;
                }
            }
        }
        pthread_mutex_unlock(&sess->root->mx);
        return true_;
    }

    printw(MSG_NOTFOUND, tk->str);
    return false_;
}

int do_break(struct session* sess){
    struct tok_s* tk;

    tk = poptok(sess);
    if(!tk){
        printe(MSG_MOREARGS, "break");
        return false_;
    }

    if(tk->id == TOK_ALL){
        return breakallforks(sess);
    }

    return breakfork(sess, tk->str);
}

int do_sleep(struct session* sess){
    struct tok_s* tk;
//    inum_t val;
//    int i;
    //1000000    

    tk = poptok(sess);
    if(!tk){
        printe(MSG_MOREARGS, "sleep");
        return false_;
    }

//    val = intval(tokval(sess, tk))*1000000;

//    if(val > 1000000){
//        i = val/1000000;
//        val = val%1000000;
//    } else {
//        i = 0;
//    }

//    while(i){
//        usleep(1000000);
//        i--;
//    }

//    usleep(val);

#ifdef QT
    //printe(MSG_IMPLEMENT, "sleep");
    //Sleep(intval(tokval(sess, tk)));
    //qsleep(intval(tokval(sess, tk)));
#else
    sleep(intval(tokval(sess, tk)));
#endif

    return true_;

}

int do_wait(struct session* sess){
    struct tok_s* tk;
    struct session* s;

    tk = poptok(sess);
    if(!tk){
        printe(MSG_MOREARGS, "wait");
        return false_;
    }

    pthread_mutex_lock(&sess->root->mx);

    for(s = sess->root; s; s = s->nextchild){
        if(!strcmp(s->name, tk->str)){
            break;
        }
    }

    pthread_mutex_unlock(&sess->root->mx);

    if(!s){
        printe(MSG_NOTFOUND, tk->str);
        return false_;
    }

    while(s->flags & SESSF_RUNNING){
        if(sess->cursess->flags & SESSF_BREAK) break;
    }

    return true_;

}

int do_exec(struct session* sess){
    struct tok_s* tk, *first = NULL, *last=NULL;
    char* cmd;
    size_t size;
    int red;

//    char* arg[10] = {{"> OUT"},NULL};

    int fd;

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
        printe(MSG_MOREARGS, "exec");
        return false_;
    }

    cmd = (char*)malloc(sess->cursess->expbufsize);
    if(!cmd){
        printe(MSG_NOMEM);
        return false_;
    }

    size = sess->cursess->expbufsize;
    *cmd = 0;

    strcpy(cmd, first->str);
    size -= strlen(cmd);

    for(tk = first->next; tk; tk = tk->next){
        if(tk->flags & TOKF_RES){
            snprintf(cmd+(sess->cursess->expbufsize-size), size, " %Lf", tokval(sess, tk));
        } else {
            snprintf(cmd+(sess->cursess->expbufsize-size), size, " %s", tk->str);
        }
        size -= strlen(cmd);
    }

    strcat(cmd, " > OUT");

    system(cmd);

    fd = open("OUT", O_RDONLY);

    if(fd > -1){
        red = read(fd, sess->cursess->outbuf, sess->cursess->outbufsize);
        if(red > 0){
            sess->cursess->outbuf[red] = 0;
            textout_(sess->cursess->outbuf);
        }
        close(fd);
    } else {
        return false_;
    }

    return true_;
}

//name, time(seconds), expression or variable
int do_timer(struct session* sess){
    struct tok_s* exp, *t, *name;
    struct timerinfo* ti;
    struct tm* tmnow, *tmthen;
    time_t now;//, then;

    time(&now);

    exp = poptok(sess);
    t = poptok(sess);
    name = poptok(sess);

    if(!exp || !t || !name){
        printe(MSG_MOREARGS, "timer");
        return false_;
    }

    tmnow = localtime(&now);
    tmnow->tm_sec += tokval(sess, t);

    for(ti = sess->cursess->timers; ti; ti = ti->next){
        if(now + tokval(sess, t) > ti->sched + ti->len){
            tmthen = localtime(&ti->sched);
            tmthen->tm_sec += ti->len;
        }
    }
	
	return false_;
}

int do_killtimer(struct session* sess){
	return false_;
}

int do_pausetimer(struct session* sess){
	return false_;
}

int do_resumetimer(struct session* sess){
	
	return false_;

}
