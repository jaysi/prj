#include "double.h"

int alloc_global_toks(struct session* sess){

    sess->cursess->pre->tokpool = malloc(sess->cursess->pre->tokpoolsize*sizeof(struct tok_s));
    if(!sess->cursess->pre->tokpool) return false_;

    sess->cursess->pre->gsbufsize =   strlen(sess->cursess->expbuf) +
                        (sess->cursess->pre->tokpoolsize + 1) +
                        charcount(sess->cursess->expbuf, dbl_lf)*strlen(ANSVARNAME);
    sess->cursess->pre->gsbuf = (char*)malloc(sess->cursess->pre->gsbufsize);
    sess->cursess->pre->gsbufi = 0;

    printd("tokpoolsize: %u, gsbufsize: %u"PARAEND, sess->cursess->pre->tokpoolsize, sess->cursess->pre->gsbufsize);

    return true_;
}

int init_global_stack(struct session* sess, int ntoks){
    sess->cursess->pre->tokpool = NULL;
    sess->cursess->pre->tokpooli = 0;
    sess->cursess->pre->gsbuf = NULL;
    sess->cursess->pre->gsbufi = 0;
    if(ntoks <= 0){
        sess->cursess->pre->tokpoolsize = 2*charcount(sess->cursess->expbuf, dbl_delimlist);
        sess->cursess->pre->tokpoolsize += charcount(sess->cursess->expbuf, dbl_lf);//add for ans if needed, should be *2?
    } else {
        sess->cursess->pre->tokpoolsize = ntoks;
    }

    if(!alloc_global_toks(sess)
            ){
        printe(MSG_NOMEM);
        return false_;
    }

    return true_;
}

int alloc_stack_toks(struct session* sess, int ntok){

    /*
    if(sess->cursess->pre->exp->nstacktok){
    free(sess->cursess->pre->exp->stacktoks);
    sess->cursess->pre->exp->nstacktok = 0;
    }
    */

    sess->cursess->pre->exp->nstacktok = ntok + 1 + 1;	//the last 1 is for
//    if(sess->cursess->autorun == AUTORUN_DEFON){
//        sess->cursess->pre->exp->nstacktok += sess->cursess->autostack;
//    }
                        //last answer if needed
    sess->cursess->pre->exp->stacktoki = 0;
    //sess->cursess->pre->exp->stacktoks = malloc(sess->cursess->pre->exp->nstacktok*sizeof(struct tok_s));
    //if(!sess->cursess->pre->exp->stacktoks) return false_;
    sess->cursess->pre->exp->stacktoks = &sess->cursess->pre->tokpool[sess->cursess->pre->tokpooli];
    sess->cursess->pre->tokpooli += sess->cursess->pre->exp->nstacktok;

//    sess->cursess->pre->exp->bufex = NULL;
//    sess->cursess->pre->exp->bufexsize = 0;

    sess->cursess->pre->exp->stacktokstrbufi = 0;
    sess->cursess->pre->exp->stacktokstrbufsize =	strlen(sess->cursess->pre->exp->infix) + (sess->cursess->pre->exp->nstacktok+1) + strlen(ANSVARNAME);
    //sess->cursess->pre->exp->stacktokstrbuf = (char*)malloc(sess->cursess->pre->exp->stacktokstrbufsize);
    sess->cursess->pre->exp->stacktokstrbuf = sess->cursess->pre->gsbuf + sess->cursess->pre->gsbufi;
    sess->cursess->pre->gsbufi += sess->cursess->pre->exp->stacktokstrbufsize;


    printd("nstacktok: %u, tokpooli: %u, stacktokstrbufsize: %u, gsbufi: %u"PARAEND,
           sess->cursess->pre->exp->nstacktok,
           sess->cursess->pre->tokpooli,
           sess->cursess->pre->exp->stacktokstrbufsize,
           sess->cursess->pre->gsbufi
           );

    return true_;
}

int init_stack(struct session* sess){
	
    assert(sess->cursess->pre->exp);
    assert(sess->cursess->pre->exp->infix);
	
    sess->cursess->pre->exp->stacktop = NULL;
    sess->cursess->pre->exp->stacktoki = 0;
    sess->cursess->pre->exp->nstacktok = 0;
    sess->cursess->pre->exp->stacktokstrbufi = 0;
    sess->cursess->pre->exp->stacktokstrbuf = NULL;
    sess->cursess->pre->exp->stacktoks = NULL;

    //guess number of tokens
    printd10(	"counting tokens of \"%s\" to %i\n",
                sess->cursess->pre->exp->infix,
                2*charcount(sess->cursess->pre->exp->infix, dbl_delimlist));

    if(!alloc_stack_toks(sess, 2*charcount(sess->cursess->pre->exp->infix, dbl_delimlist))){
        printe(MSG_NOMEM);
        return false_;
    }

    return true_;
}

int free_global_stack(struct session* sess){

    if(sess->cursess->pre->gsbuf){
        free(sess->cursess->pre->gsbuf);
    }

    if(sess->cursess->pre->tokpool){
        free(sess->cursess->pre->tokpool);
    }

    sess->cursess->pre->gsbuf = NULL;
    sess->cursess->pre->tokpool = NULL;
    return true_;
}

int free_stack(struct session* sess){

//        if(sess->cursess->pre->exp->stacktokstrbuf){
//            printd("free'd stacktokstrbuf\n");
//            free(sess->cursess->pre->exp->stacktokstrbuf);
//        }

//        if(sess->cursess->pre->exp->stacktoks){
////	        for(i = 0; i < sess->cursess->pre->exp->nstacktok; i++)
////	            if(sess->cursess->pre->exp->stacktoks[i].flags & TOKF_DYN) free(sess->cursess->pre->exp->stacktoks[i].str);
//            printd("free'd stacktoks\n");
//            free(sess->cursess->pre->exp->stacktoks);
//        }

//        if(sess->cursess->pre->exp->bufex){
//            free(sess->cursess->pre->exp->bufex);
//        }

        return true_;
}

struct tok_s* get_stacktok_ptr(struct session* sess){

    struct tok_s* ret;

//	printd("getting stacktok pointer, stacktoki = %i, nstacktok = %i\n",
//		sess->cursess->pre->exp->stacktoki, sess->cursess->pre->exp->nstacktok);

    assert(sess->cursess->pre->exp->stacktoki < sess->cursess->pre->exp->nstacktok);

    //if(sess->cursess->pre->exp->stacktoki == sess->cursess->pre->exp->nstacktok) return NULL;

//	printd("trying to access pointer to see if crash: %i\n",
//		sess->cursess->pre->exp->stacktoks[sess->cursess->pre->exp->stacktoki].id);
//	printd("did it?\n");

    ret = &(sess->cursess->pre->exp->stacktoks[sess->cursess->pre->exp->stacktoki]);
//	printd(	"trying to access pointer again to see if crash: %i,"
//		"stacktoki = %i, nstacktok = %i\n", ret->id,
//		sess->cursess->pre->exp->stacktoki, sess->cursess->pre->exp->nstacktok);
//	printd("did it?\n");
    sess->cursess->pre->exp->stacktoki++;

    return ret;
}

struct tok_s* make_stacktok(struct session* sess, char* str){
    struct tok_s* tk;
    struct tok_s* base;

//    printd("make_stacktok() from %s\n", str);

    base = tokptr(sess, str);

//    printd("base is %u for str \'%s';\n", base->id, str);
    printd10(	"using stack slot %i for token %s, nstacktoks = %i,"
                " strsize: %i, bufi: %i\n",
                sess->cursess->pre->exp->stacktoki, str, sess->cursess->pre->exp->nstacktok,
                strlen(str) + 1, sess->cursess->pre->exp->stacktokstrbufi);

    tk = get_stacktok_ptr(sess);

    if(!tk){
//      printd("fails while getting tk\n");
        return NULL;
    }

//    printd("got tk ptr, trying to access it: %i, str = %s\n", tk->id, str);

    //memcpy(tk, base, sizeof(struct tok_s));
    //
    tk->id = base->id;
    tk->type = base->type;
    tk->pred = base->pred;
    tk->flags = 0;
    tk->i = -1; tk->j = -1;    
//    tk->resolved = 0;
//    tk->neg = 0;

//    printd("done %i, now getting tk's str address, strbufi = %i, "
//    "str = %s\n", tk->id, sess->cursess->pre->exp->stacktokstrbufi, str);

//	printd0("TODO: the vstr call scrambles this?");
    assert(sess->cursess->pre->exp->stacktokstrbufi + strlen(str) + 1 < sess->cursess->pre->exp->stacktokstrbufsize);
    tk->str = sess->cursess->pre->exp->stacktokstrbuf + sess->cursess->pre->exp->stacktokstrbufi;
    sess->cursess->pre->exp->stacktokstrbufi += strlen(str) + 1;

//    printd("got, now copying string %s to it\n", str);

    strcpy(tk->str, str);

//    printd("done, results is: %s\n", tk->str);

    return tk;
}

//struct tok_s* make_dyn_stacktok(struct session* sess, char* str){
//    struct tok_s* tk;
//    struct tok_s* base;

////    printd("make_stacktok() from %s\n", str);

//    base = tokptr(sess, str);

////    printd("base is %u for str \'%s';\n", base->id, str);
//    printd10(	"using stack slot %i for token %s, nstacktoks = %i,"
//                " strsize: %i, bufi: %i\n",
//                sess->cursess->pre->exp->stacktoki, str, sess->cursess->pre->exp->nstacktok,
//                strlen(str) + 1, sess->cursess->pre->exp->stacktokstrbufi);

//    tk = get_stacktok_ptr(sess);

//    if(!tk){
////        printd("fails while getting tk\n");
//        return NULL;
//    }

////    printd("got tk ptr, trying to access it: %i, str = %s\n", tk->id, str);

//    //memcpy(tk, base, sizeof(struct tok_s));
//    //
//    //if(tk->flags & TOKF_DYN && tk->str) free(tk->str);
//    tk->id = base->id;
//    tk->type = base->type;
//    tk->pred = base->pred;
//    tk->flags = 0;
////    tk->resolved = 0;
////    tk->neg = 0;

////    printd("done %i, now getting tk's str address, strbufi = %i, "
////    "str = %s\n", tk->id, sess->cursess->pre->exp->stacktokstrbufi, str);

////	printd0("TODO: the vstr call scrambles this?");
//    tk->str = (char*)malloc(strlen(str)+1);
//    if(!tk->str){
//        printe("not enough memmory."PARAEND);
//        return NULL;
//    }

////    printd("got, now copying string %s to it\n", str);

//    strcpy(tk->str, str);
//    //tk->flags |= TOKF_DYN;

////    printd("done, results is: %s\n", tk->str);

//    return tk;
//}

void insert_pre(struct session* sess, struct tok_s* tk){

    printd("inserting %s to prefix\n", tk->str);

    //must be cut from pop,
    tk->next = NULL;
    if(!sess->cursess->pre->exp->prefixfirst){
    printd("as first\n");
        sess->cursess->pre->exp->prefixfirst = tk;
        //tk->prev = NULL;
    } else {
        sess->cursess->pre->exp->prefixlast->next = tk;
        //sess->cursess->pre->exp->prefixlast->prev = sess->cursess->pre->exp->prefixlast;
    }
    sess->cursess->pre->exp->prefixlast = tk;

}

void remove_prehead(struct session* sess){

    printd("removing %s from prefix\n", sess->cursess->pre->exp->prefixfirst->str);

    if(sess->cursess->pre->exp->prefixfirst){
        sess->cursess->pre->exp->prefixfirst = sess->cursess->pre->exp->prefixfirst->next;
    }

}


//used for calc stack
struct tok_s* poptok(struct session* sess){

    struct tok_s* tk = NULL;

//    printd("poping\n");

    if(sess->cursess->pre->exp->stacktop){
    printd("stacktop = %s\n", sess->cursess->pre->exp->stacktop->str);
        if(sess->cursess->pre->exp->stacktop->id != TOK_BR_OPEN){
        printd("sess->cursess->pre->exp->stacktop->id != BR_OPEN (\n");
        tk = sess->cursess->pre->exp->stacktop;
            sess->cursess->pre->exp->stacktop = sess->cursess->pre->exp->stacktop->next;
            tk->next = NULL;
        } else {
            sess->cursess->pre->exp->stacktop = sess->cursess->pre->exp->stacktop->next;
        }
    printd("stacktop is now %s\n",
        sess->cursess->pre->exp->stacktop?sess->cursess->pre->exp->stacktop->str:"NULL");
    }

    return tk;
}

int pop_to_br(struct session* sess){

    int ret = false_;//check for matching brackets

    printd("this was called, popping till BR_OPEN\n");

    while(sess->cursess->pre->exp->stacktop){
        if(sess->cursess->pre->exp->stacktop->id == TOK_BR_OPEN){
            ret = true_;
            break;
        }
        /*
        tk = sess->cursess->pre->exp->stacktop;
        sess->cursess->pre->exp->stacktop = sess->cursess->pre->exp->stacktop->next;
        insert_pre(sess, tk);
        */
        pop_to_pre(sess);

    }

    printd(	"stacktop is now %s\n",
        sess->cursess->pre->exp->stacktop?sess->cursess->pre->exp->stacktop->str:"NULL");

    if(sess->cursess->pre->exp->stacktop && ret){//bypass BR_OPEN
        sess->cursess->pre->exp->stacktop = sess->cursess->pre->exp->stacktop->next;
    }

    printd(	"and then, stacktop is now %s\n",
        sess->cursess->pre->exp->stacktop?sess->cursess->pre->exp->stacktop->str:"NULL");

    return ret;

}

int pop_to_br_for_col(struct session* sess){

    int ret = false_;//check for matching brackets

    printd("this was called, popping till BR_OPEN, ABR_OPEN or COL\n");

    while(sess->cursess->pre->exp->stacktop){
        if(sess->cursess->pre->exp->stacktop->id == TOK_BR_OPEN  || sess->cursess->pre->exp->stacktop->id == TOK_ABR_OPEN || sess->cursess->pre->exp->stacktop->id == TOK_COL){
            ret = true_;
            break;
        }
        /*
        tk = sess->cursess->pre->exp->stacktop;
        sess->cursess->pre->exp->stacktop = sess->cursess->pre->exp->stacktop->next;
        insert_pre(sess, tk);
        */
        pop_to_pre(sess);

    }

    printd(	"stacktop is now %s\n",
        sess->cursess->pre->exp->stacktop?sess->cursess->pre->exp->stacktop->str:"NULL");

    if(sess->cursess->pre->exp->stacktop && sess->cursess->pre->exp->stacktop->id == TOK_COL && ret){//bypass ,
        sess->cursess->pre->exp->stacktop = sess->cursess->pre->exp->stacktop->next;
    }

    printd(	"and then, stacktop is now %s\n",
        sess->cursess->pre->exp->stacktop?sess->cursess->pre->exp->stacktop->str:"NULL");

    return ret;

}

int pop_to_abr(struct session* sess){

    int ret = false_;//check for matching brackets

    printd("this was called, popping till ABR_OPEN\n");

    while(sess->cursess->pre->exp->stacktop){
        if(sess->cursess->pre->exp->stacktop->id == TOK_ABR_OPEN){
            ret = true_;
            break;
        }
        /*
        tk = sess->cursess->pre->exp->stacktop;
        sess->cursess->pre->exp->stacktop = sess->cursess->pre->exp->stacktop->next;
        insert_pre(sess, tk);
        */
        pop_to_pre(sess);

    }

    printd(	"stacktop is now %s\n",
        sess->cursess->pre->exp->stacktop?sess->cursess->pre->exp->stacktop->str:"NULL");

    if(sess->cursess->pre->exp->stacktop && ret){//bypass BR_OPEN
        //sess->cursess->pre->exp->stacktop = sess->cursess->pre->exp->stacktop->next;
        pop_to_pre(sess);
    }

    printd(	"and then, stacktop is now %s\n",
		sess->cursess->pre->exp->stacktop?sess->cursess->pre->exp->stacktop->str:"NULL");

    return ret;

}


int pop_all(struct session* sess){
     while(sess->cursess->pre->exp->stacktop){
        pop_to_pre(sess);
    }
    return true_;
}

void push_calc(struct session* sess, struct tok_s* tk){

    printd("pushing \'%s\'...\n", tk->str);

    //tk->prev = sess->cursess->pre->exp->stacktop;
    //tk->next = NULL;
    tk->next = sess->cursess->pre->exp->stacktop;
    sess->cursess->pre->exp->stacktop = tk;
/*
    if(sess->cursess->pre->exp->stacktop){
        printd("to stack top\n");
        sess->cursess->pre->exp->stacktop->next = tk;
    }

    sess->cursess->pre->exp->stacktop = tk;
*/
}

void pop_to_pre(struct session* sess){

//    printd("poping\n");
    struct tok_s* tk;

    if(sess->cursess->pre->exp->stacktop){
        printd("stacktop = %s\n", sess->cursess->pre->exp->stacktop->str);
        if(sess->cursess->pre->exp->stacktop->id != TOK_BR_OPEN){
            printd("sess->cursess->pre->exp->stacktop->id != BR_OPEN (\n");

            tk = sess->cursess->pre->exp->stacktop;
            sess->cursess->pre->exp->stacktop = sess->cursess->pre->exp->stacktop->next;

            //disconnect, this is done in insert pre
            //tk->next = NULL;

            //put to prefix
            insert_pre(sess, tk);
        } else {//bypass (
            sess->cursess->pre->exp->stacktop = sess->cursess->pre->exp->stacktop->next;
        }
    printd("stacktop is now %s\n",
        sess->cursess->pre->exp->stacktop?sess->cursess->pre->exp->stacktop->str:"NULL");
    }
}

void push(struct session* sess){

    /*if there is items in stack and token is not ( and [*/
    printd("pushing %s...\n", sess->cursess->pre->exp->curtok->str);
    if(sess->cursess->pre->exp->curtok->id != TOK_BR_OPEN && sess->cursess->pre->exp->curtok->id != TOK_ABR_OPEN){
    printd("is not BR_OPEN \"%s\" pred = %i, poping \"%s\" pred = %i...\n",
        sess->cursess->pre->exp->curtok->str, sess->cursess->pre->exp->curtok->pred,
        sess->cursess->pre->exp->stacktop?sess->cursess->pre->exp->stacktop->str:NULL,
        sess->cursess->pre->exp->stacktop?sess->cursess->pre->exp->stacktop->pred:-1);
        while(  (sess->cursess->pre->exp->stacktop) &&
                //bigger pred number means lower prediction
        (sess->cursess->pre->exp->curtok->pred >= sess->cursess->pre->exp->stacktop->pred) &&
                (sess->cursess->pre->exp->stacktop->id != TOK_BR_OPEN) &&
                (sess->cursess->pre->exp->stacktop->id != TOK_ABR_OPEN)
            ){

            pop_to_pre(sess);
        }
    }

    sess->cursess->pre->exp->curtok->next = sess->cursess->pre->exp->stacktop;
    sess->cursess->pre->exp->stacktop = sess->cursess->pre->exp->curtok;

    printd("stacktop is now %s\n",
        sess->cursess->pre->exp->stacktop?sess->cursess->pre->exp->stacktop->str:"NULL");


}

void pushtok(struct session* sess, struct tok_s* tk){

    /*if there is items in stack and token is not (*/
    printd("pushing %s...\n", tk->str);
    if(tk->id != TOK_BR_OPEN){
    printd("is not BR_OPEN \"%s\", poping \"%s\"...\n",
        tk->str, sess->cursess->pre->exp->stacktop?sess->cursess->pre->exp->stacktop->str:NULL);
        while(  (sess->cursess->pre->exp->stacktop) &&
                //bigger pred number means lower prediction
                (tk->pred >= sess->cursess->pre->exp->stacktop->pred) &&
                (sess->cursess->pre->exp->stacktop->id != TOK_BR_OPEN)
            ){

            pop_to_pre(sess);
        }
    }

    tk->next = sess->cursess->pre->exp->stacktop;
    sess->cursess->pre->exp->stacktop = tk;
}

void _show_pre(struct session* sess, struct pre_exp* pre){
    struct tok_s* ent;
    struct expression* exp;
    for(exp = pre->expfirst; exp; exp = exp->next){
        for(ent = exp->prefixfirst; ent; ent = ent->prenext){
            printo("(%i%s)%s(%Lf)", ent->id, ent->flags & TOKF_NEG?"-":"+", ent->str, tokval(sess, ent));
        }
        printo(" #\n");
    }
}

#ifndef NDEBUG

void _show_stack(struct session* sess){
    struct tok_s* ent;
    ent = sess->cursess->pre->exp->stacktop;
    printo("stack [top -> down]: ");
    while(ent){
        printo("(%i%s)%s(%Lf)", ent->id, ent->flags & TOKF_NEG?"-":"+", ent->str, tokval(sess, ent));
        ent = ent->next;
        if(ent) printo(" ");
    }
    printo(" #\n");
}

void _show_prefix(struct session* sess){
    struct tok_s* ent;
    ent = sess->cursess->pre->exp->prefixfirst;
    printo("prefix [first -> last]: ");
    while(ent){
        printo("(%i%s)%s(%Lf)", ent->id, ent->flags & TOKF_NEG?"-":"+", ent->str, tokval(sess, ent));
        ent = ent->next;
        if(ent) printo(" ");
    }
    printo(" #\n");
}

#endif
