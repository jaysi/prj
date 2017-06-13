#include "double.h"

struct variable* get_clipvar(struct session* sess){
    struct variable* var;
    char* end;
    size_t len, nsize = strlen(CLIPVARNAME) + 1;
    char* str;
    for(var = sess->cursess->vfirst; var; var = var->next){
        if(!strcmp(var->name, CLIPVARNAME)){

#if (defined(WIN32)&&(!defined(QT)))
    HGLOBAL hglb;
//    LPSTR lpstr;
    if (!OpenClipboard(GetClipboardOwner())) return NULL;
    hglb = GetClipboardData(CF_TEXT);
    str = GlobalLock(hglb);
    if(!str) goto end;

    len = strlen(str);
    if(len > sess->cursess->expbufsize) len = sess->cursess->expbufsize - 1;

    if(!strcmp(str, var->exp)) goto end;

    free(var->name);
    var->name = (char*)malloc(nsize + len + 1);

    var->exp = var->name + nsize;

    strcpy(var->name, CLIPVARNAME);
    strcpy_(var->exp, str, len+1);
    var->val = strtold(var->exp, &end);
end:
    GlobalUnlock(hglb);
    CloseClipboard();

#elif (defined(QT))                

    if((str = qclipstr(sess))){
        var->val = strtold(str, &end);
    } else {
        var->val = 0.0;
    }

            /*
    if(!(str = qclipstr(sess)) || !var->exp) goto end;

    if(!strcmp(str, var->exp)) goto end;

    len = strlen_(str, sess->cursess->expbufsize-1);

    free(var->name);
    var->name = (char*)malloc(nsize + len + 1);

    var->exp = var->name + nsize;

    strcpy(var->name, CLIPVARNAME);
    strcpy_(var->exp, str, len+1);
    var->val = strtold(var->exp, &end);
    */
//end:
#else
    printw(MSG_IMPLEMENT, "clipboard");
    return NULL;
#endif            

            return var;
        }
    }
    return NULL;
}

struct variable* var_ptr(	struct session* sess, char* name,
                            struct variable** prev){
    struct variable* var;
    char *end;
    size_t sessnamelen;

    struct session* s;

    assert(name);

    //printD("sess: %s, name: %s"PARAEND, sess?(sess->cursess->name?sess->cursess->name:"NULL"):"NULL",name?name:"NULL");

    if(*name == dbl_chain){

        //if there is another @ inside the name this is the name of the fork
        if( (end = strchr(name + 1, dbl_chain)) ){

            var = NULL;

            pthread_mutex_lock(&sess->root->mx);
            for(s = sess->cursess->nextchild; s; s = s->nextchild){

                name++;

                sessnamelen = strlen(s->name);

//                printD("sessnamelen: %u, sessname: %s, end - name: %u, end: %s, name: %s"PARAEND,
//                       sessnamelen, s->name, end - name, end, name);

                if( (end - name == sessnamelen) &&
                    !memcmp(name, s->name, sessnamelen)){//found!

                    var = var_ptr(s, end, prev);

//                    printD("cmp ok, var: %s"PARAEND, var?var->name:"NULL");

                    break;

                }
            }
            pthread_mutex_unlock(&sess->root->mx);

            return var;

        } else {//this is variable name
            name++;
        }
    }

    if(!strcmp(name, CLIPVARNAME)){
        return get_clipvar(sess);
    }

    if(prev) *prev = sess->cursess->vfirst;

    printd16("var_ptr %s."PARAEND, name);

    for(var = sess->cursess->vfirst; var; var = var->next){
        if(!strcmp(var->name, name)){
            return var;
        }
        if(prev) *prev = var;
    }

    return NULL;
}

fnum_t var_val(struct session* sess, char* name){
    struct variable* var;

    var = var_ptr(sess, name, NULL);
    if(var && var->type != VAR_ARRAY){
        return var->val;
    }

    return 0;
}

int set_varval(struct session* sess, char* name, fnum_t val){

    struct variable* prev;
    struct variable* var;

    var = var_ptr(sess, name, &prev);

//    printd14("setting variable '%s' to %s."PARAEND, name, clean_float(val, sess->cursess->tokbuf, sess->cursess->tokbufsize));

    if(!var){
        printe(MSG_NOTFOUND, name);
        return false_;
    }

    if(var->type == VAR_SYS || var->type == VAR_CTL || var->flags & VARF_LOCK){
        printe(MSG_FORBID, name);
        return false_;
    }

    if(var->type == VAR_ARRAY){
        printe(MSG_BADUSE, name);
        return false_;
    }

    var->val = val;

    return true_;
}

int _create_var(struct session* sess, char* name, char type, int idx, int jdx, char* infixrest){
    struct variable* var;
    size_t nsize;

    assert(infixrest);

    //infixrest = NOVAREXP;

    nsize = strlen(name) + 1;

    var = (struct variable*)malloc(sizeof(struct variable));
    var->next = NULL;
    var->pre = NULL;

    var->type = type;
    var->flags = 0;
    var->i = idx;
    var->j = jdx;

    var->name = (char*)malloc( nsize + strlen(infixrest) + 1 );

    var->exp = var->name + nsize;

    var->desc = NULL;

    strcpy(var->name, name);
    strcpy(var->exp, infixrest);

    if(!sess->cursess->vfirst){
        sess->cursess->vfirst = var;
    } else {
        sess->cursess->vlast->next = var;
    }
    sess->cursess->vlast = var;
	
	if(type != VAR_SYS && type != VAR_CTL) sess->cursess->pre->exp->var = var;
    
    printd16("var %s created."PARAEND, var->name);

    return set_var_pre(sess, var);
}

int create_var(struct session* sess, char* name, char type, int idx, int jdx, char* infixrest){
    struct variable* var;

    if(type == VAR_ARRAY){
        if(idx < 0 || idx > MAXARRAYMEMBERS || jdx > MAXARRAYMEMBERS){
            printe(MSG_RANGE, name);
            return false_;
        }
        var = array_ptr(sess, name, idx, jdx, NULL);
    } else {
        var = var_ptr(sess, name, NULL);
    }

    if(var){
        //anyway, set the exp.var so the program could
        //set the var's value (not expression)
        sess->cursess->pre->exp->var = var;
        //i remove this, this could be annoying while using loops
        //printw("variable already defined."PARAEND);
        return true_;
    }

    if(!isalpha(*name) && *name != '_'){
    	printd8("BADF HERE");
        printe(MSG_FORMAT, name);
        return false_;
    }

    printd15("name: %s, infixrest: %s"PARAEND, name, infixrest);

    return _create_var(sess, name, type, idx, jdx, infixrest);

}

int rm_var(struct session* sess, char* name){
    struct variable* var, *prev;
    int found = false_;

    //if(is_array_name(sess, name, &aid) == true_) return rm_a(sess, name);

    while((var = var_ptr(sess, name, &prev))){

        if(!var){
            break;
        }

        found = true_;

        if(var->type == VAR_SYS || var->type == VAR_CTL || var->flags & VARF_LOCK){
            printe(MSG_FORBID, name);
            break;
        }

        if(var == sess->cursess->vfirst){
            sess->cursess->vfirst = sess->cursess->vfirst->next;
        } else if(var == sess->cursess->vlast){
            prev->next = NULL;
            sess->cursess->vlast = prev;
        } else if(prev){
            prev->next = var->next;
        }

        if(var->desc) free(var->desc);
        free(var->name);
        free(var);
    }

    if(!found){
        printe(MSG_NOTFOUND, name);
        return false_;
    } else {
        return true_;
    }
}

int clear_all_vars(struct session* sess){
    struct variable* var, *prev;

    var = sess->cursess->vfirst;
    prev = NULL;

    while(var){

        if(!(var->type == VAR_SYS || var->type == VAR_CTL || var->flags & VARF_LOCK)){

            if(var == sess->cursess->vfirst){
                sess->cursess->vfirst = sess->cursess->vfirst->next;
            } else if(var == sess->cursess->vlast){
                prev->next = NULL;
                sess->cursess->vlast = prev;
            } else if(prev){
                prev->next = var->next;
            }

            if(var->desc) free(var->desc);
            free(var->name);
            free(var);

            var = prev->next;

        } else {

            prev = var;
            var = var->next;

        }
    }

    return true_;
}

void listvarenum(struct session* sess, struct tok_s* tk){
	
    struct variable* var;    
    char nbuf[MAXNUMLEN];
    char* buf = sess->cursess->outbuf;
    size_t bufsize, pos;
//    struct session* child;
	
    print_listhead(sess, "variable");

    for(var = sess->cursess->vfirst; var; var = var->next){

        //if(tk){if(strcmp(var->name, tk->str)) continue;};
        if(tk){if(wildcmp(var->name, tk->str, '*', '?', '\\')) continue;};

        pos = 0;
        bufsize = sess->cursess->outbufsize;

        if(!strcmp(var->name, CLIPVARNAME)){
            var = get_clipvar(sess);
        }

        //printo("  %s", var->name);

        snprintf(buf, bufsize, "  %s", var->name);
        pos = strlen(buf);

        if(var->type == VAR_ARRAY){
            //printo("[%i]", var->i);
            snprintf(buf + pos, bufsize - pos, "[%i][%i]", var->i, var->j);
            pos = strlen(buf);
        }

        printo(	"%s { %s } := %s /* %s */ [%s, %s]"PARAEND, buf,
                var->exp?!strcmp(var->name, CLIPVARNAME)?"...":var->exp:"",
                sep_val(sess->cursess, var->val,
                        sess->cursess->tokbuf,
						sess->cursess->tokbufsize,
                        nbuf, MAXNUMLEN),
                var->desc?var->desc:NOTSET,
                var->type==VAR_SYS?"system":var->type==VAR_CTL?"control":var->type==VAR_ARRAY?"array":"normal",
                var->flags&VARF_LOCK?"locked":"unlocked");

    }

    print_listfoot(sess, "variable");

//    for(child = sess->cursess->nextchild; child; child = child->nextchild){
//        if(!(child->flags & SESSF_RUNNING)) listvarenum(child, tk);
//        else printw(MSG_BUSY, child->name);
//    }
	
}

void list_var(struct session* sess){

	struct tok_s* tk;
    struct session* child;

    tk = poptok(sess);

    pthread_mutex_lock(&sess->root->mx);

    listvarenum(sess, tk);

    for(child = sess->cursess->nextchild; child; child = child->nextchild){
        if(!(child->flags & SESSF_RUNNING)) listvarenum(child, tk);
        else printw(MSG_BUSY, child->name);
    }

    pthread_mutex_unlock(&sess->root->mx);

}

int show_vdesc(struct session* sess, char* name){
    struct variable* var;

    //if(is_array_name(sess, name, &aid) == true_) return show_adesc(sess, name);

    var = var_ptr(sess, name, NULL);
    if(!var){
        printw(MSG_NOTFOUND, name);
        return false_;
    }

    printo(	" %s /* %s */ [%s, %s]"PARAEND,
            name,
            var->desc?var->desc:NOTSET,
            var->type==VAR_SYS?"system":var->type==VAR_CTL?"control":"normal",
            var->flags&VARF_LOCK?"locked":"unlocked");
    return true_;
}

int rm_vdesc(struct session* sess, char* name){
    struct variable* var;

    //if(is_array_name(sess, name, &aid) == true_) return rm_adesc(sess, name);

    var = var_ptr(sess, name, NULL);
    if(!var){
        printw(MSG_NOTFOUND, name);
        return false_;
    }

    if(var->type == VAR_SYS || var->type == VAR_CTL){
        printe(MSG_FORBID, var->name);
        return false_;
    }

    if(var->desc){
        free(var->desc);
        var->desc = NULL;
    }
    return true_;
}

int set_vdesc(struct session* sess, char* name, char* desc){
    struct variable* var;

    //if(is_array_name(sess, name, &aid) == true_) return set_adesc(sess, name);

    var = var_ptr(sess, name, NULL);

    if(!var){
        printw(	MSG_NOTFOUND,
                name);
        return false_;
    }

    if(var->type == VAR_SYS || var->type == VAR_CTL || var->flags & VARF_LOCK){
        printe(MSG_FORBID, var->name);
        return false_;
    }

    if(var->desc){
        printe(MSG_EXISTS, name);
        return false_;
    }

    var->desc = (char*)malloc(strlen(desc)+1);
    if(!var->desc){
        printe(MSG_NOMEM);
        return false_;
    }

    strcpy(var->desc, desc);

    return true_;
}

int do_vdesc(struct session* sess){

    struct tok_s* tk, *tk1;

    tk = poptok(sess);
    if(!tk){
        printe(MSG_MOREARGS, "vdesc");
        return false_;
    } else {

        switch(tk->id){

            case TOK_VAR:
                return show_vdesc(sess, tk->str);
            break;

            case TOK_ARRAY:
                return show_adesc(sess, tk->str, tk->i, tk->j);
            break;

            case TOK_SHOW:
                tk1 = poptok(sess);
                if(tk1){
                    switch(tk1->id){
                        case TOK_VAR:
                        return show_vdesc(sess, tk1->str);
                        case TOK_ARRAY:
                        return show_adesc(sess, tk1->str, tk1->i, tk1->j);
                        default:
                            printe(MSG_BADARG, tk1->str);
                            return false_;
                    }
                }
            break;

            case TOK_REMOVE:
                tk1 = poptok(sess);
                if(tk1){
                    switch(tk1->id){
                        case TOK_VAR:
                        return rm_vdesc(sess, tk1->str);
                        case TOK_ARRAY:
                        return rm_adesc(sess, tk1->str, tk1->i, tk1->j);
                        default:
                            printe(MSG_BADARG, tk1->str);
                            return false_;
                    }
                }

            break;

            default:
                tk1 = poptok(sess);
                if(tk1){
                    switch(tk1->id){
                        case TOK_VAR:
                        return set_vdesc(sess, tk1->str, tk->str);
                        case TOK_ARRAY:
                        return set_adesc(sess, tk1->str, tk1->i, tk1->j, tk->str);
                        default:
                            printe(MSG_BADARG, tk1->str);
                            return false_;
                    }
                }

            break;
        }

        printe(MSG_MOREARGS, "vdesc");

        return false_;

    }
}

int set_vexp(struct session* sess, char* name, char* exp){
    struct variable* var;

    var = var_ptr(sess, name, NULL);

    if(!var){
        printw(	MSG_NOTFOUND,
                name);
        return false_;
    }

    if(var->type == VAR_SYS || var->type == VAR_CTL || var->flags & VARF_LOCK){
        printe(MSG_FORBID, var->name);
        return false_;
    }

    var->name = (char*)realloc(var->name, strlen(name) + strlen(exp) + 2);
    if(!var->name){
        printe(MSG_NOMEM);
        return false_;
    }

    var->exp = var->name + strlen(name) + 1;

    strcpy(var->exp, exp); 
	
	if(var->pre){
		delete_pre(sess, var->pre);
		var->pre = NULL;
	}        

    return set_var_pre(sess, var);
}

int vexp(struct session* sess){
    struct tok_s* tk, *tk1;
    struct variable* var;
    char* str;

    tk = poptok(sess);
    if(!tk){
        printe(MSG_MOREARGS, "vexp");
        return false_;
    } else {

        tk1 = poptok(sess);
        if(!tk1){
            printe(MSG_MOREARGS, "vexp");
            return false_;
        }

        if(tk->id == TOK_VAR){
            var = var_ptr(sess, tk->str, NULL);
            if(!var){
                printe(MSG_ERROR, "internal");
                return false_;
            }
            str = var->exp;
        } else if(tk->id == TOK_ARRAY){
            var = array_ptr(sess, tk->str, tk->i, tk->j, NULL);
            if(!var){
                printe(MSG_ERROR, "internal");
                return false_;
            }
            str = var->exp;
        } else {
            str = tk->str;
        }

        switch(tk1->id){
            case TOK_ARRAY:
                return set_aexp(sess, tk1->str, tk1->i, tk1->j, str);
            break;
            case TOK_VAR:
                return set_vexp(sess, tk1->str, str);
            break;
            default:
                printe(MSG_BADARG, tk1->str);
                return false_;
            break;

        }
    }

    return true_;

}

int cat_vexp(struct session* sess, char* name, char* exp){
    struct variable* var;

    var = var_ptr(sess, name, NULL);

    if(!var){
        printw(	MSG_NOTFOUND,
                name);
        return false_;
    }

    if(var->type == VAR_SYS || var->type == VAR_CTL || var->flags & VARF_LOCK){
        printe(MSG_FORBID, var->name);
        return false_;
    }

    var->name = (char*)realloc(var->name, strlen(name) + strlen(var->exp) + strlen(exp) + 2);

    if(!var->name){
        printe(MSG_NOMEM);
        return false_;
    }

    var->exp = var->name + strlen(name) + 1;

    strcat(var->exp, exp);

    return true_;
}

int vcat(struct session* sess){

    struct tok_s* tk, *tk1;
    //char* str;
    struct variable* var;

    tk = poptok(sess);
    if(!tk){
        printe(MSG_MOREARGS, "vcat");
        return false_;
    } else {

        tk1 = poptok(sess);
        if(!tk1){
            printe(MSG_MOREARGS, "vcat");
            return false_;
        }

        if(tk->id == TOK_VAR){
            var = var_ptr(sess, tk->str, NULL);
            if(!var){
                printe(MSG_ERROR, "internal");
                return false_;
            }
            //str = var->exp;
        } else if(tk->id == TOK_ARRAY){
            var = array_ptr(sess, tk->str, tk->i, tk->j, NULL);
            if(!var){
                printe(MSG_ERROR, "internal");
                return false_;
            }
            //str = var->exp;
        } //else {
            //str = tk->str;
        //}

        switch(tk1->id){
            case TOK_ARRAY:
                return cat_aexp(sess, tk1->str, tk1->i, tk1->j, tk->str);
            break;
            case TOK_VAR:
                return cat_vexp(sess, tk1->str, tk->str);
            break;
            default:
                printe(MSG_BADARG, tk1->str);
                return false_;
            break;

        }
    }
    return true_;
}

int create_sysvar(struct session* sess, char* name, char* desc, fnum_t val){
    struct variable* var;
    if(create_var(sess, name, VAR_SYS, 0, -1, "")){

//		var = var_ptr(sess, name, NULL);
//		assert(var);

        //var = sess->cursess->pre->exp->var;
        var = sess->cursess->vlast;

        var->desc = (char*)malloc(strlen(desc)+1);
        if(!var->desc){
            printe(MSG_NOMEM);
            return false_;
        }
        strcpy(var->desc, desc);
        var->val = val;

        var->type = VAR_SYS;
        var->flags = VARF_LOCK;

		//commented out - because the exp is uninitialized at this very early stage.
        //sess->cursess->pre->exp->var = NULL;        

        return true_;
    }

    return false_;
}

//int create_ctlvar(struct session* sess, char* name, char* desc, fnum_t val){
//	struct variable* var;
//	if(create_var(sess, name, "-")){
//
////		var = var_ptr(sess, name, NULL);
////		assert(var);
//
//		var = sess->cursess->pre->exp->var;
//
//		var->desc = (char*)malloc(strlen(desc)+1);
//		if(!var->desc){
//			printe("not enough memmory."PARAEND);
//			return false_;
//		}
//		strcpy(var->desc, desc);
//		var->val = val;
//
//		var->type = VAR_CTL;
//
//		return true_;
//	}

//	return false_;
//}

int rm_sysvar(struct session* sess, char* name){

    struct variable* var, *prev;
    var = var_ptr(sess, name, &prev);
    if(!var){
        printe(MSG_NOTFOUND, name);
        return false_;
    }

    if(var == sess->cursess->vfirst){
        sess->cursess->vfirst = sess->cursess->vfirst->next;
    } else if(var == sess->cursess->vlast){
        assert(prev);
        prev->next = NULL;
        sess->cursess->vlast = prev;
    } else {
        prev->next = var->next;
    }

    free(var->name);
    if(var->desc) free(var->desc);

    free(var);

    return false_;

}

int set_ansvar(struct session* sess, fnum_t ans){

    struct variable* var;

//    printd8("setting ansvar to %s"PARAEND, clean_float(ans, sess->cursess->tokbuf, sess->cursess->tokbufsize));

    var = var_ptr(sess, ANSVARNAME, NULL);
    assert(var);

    var->val = ans;
    return true_;

}

//TODO: the vstr interface in unsafe, reuse of make_stacktok is not good for health!
int vstr(struct session* sess, struct tok_s* tk){
    struct tok_s* tk1;
    struct variable* var;

    //if(is_array_name(sess, name, &aid) == true_) return set_adesc(sess, name);

    if(tk->id != TOK_VAR && tk->id != TOK_ARRAY){
        printe(MSG_BADARG, tk->str);
        return false_;
    }

    switch(tk->id){
        case TOK_VAR:
            var = var_ptr(sess, tk->str, NULL);
            break;
        case TOK_ARRAY:
            var = array_ptr(sess, tk->str, tk->i, tk->j, NULL);
            break;
        default:
            printe(MSG_BADARG, tk->str);
            return false_;
            break;
    }

    if(!var){
        printe(MSG_NOTFOUND, tk->str);
        return false_;
    }

    if(!var->exp){
        printw(MSG_EMPTY, var->name);
        return false_;
    }

    tk1 = make_stacktok(sess, var->exp);
    if(!tk1){
        printe(MSG_OVERFLOW, "stack");
        return false_;
    }

    tk1->id = TOK_STR;
    tk1->type = TOKT_STRING;

    printd7("pushing '%s' to calc stack."PARAEND, tk1->str);
    push_calc(sess, tk1);

    return true_;
}

int vlock_show(struct session* sess){

    struct variable* var;

    print_listhead(sess, "locked variable");
    for(var = sess->cursess->vfirst; var; var = var->next){
        if(var->flags & VARF_LOCK){
            printo(" %s"PARAEND, var->name);
        }
    }
    print_listfoot(sess, "locked variable");

    return true_;
}

int vunlock_show(struct session* sess){

    struct variable* var;

    print_listhead(sess, "unlocked variable");
    for(var = sess->cursess->vfirst; var; var = var->next){
        if(!(var->flags & VARF_LOCK)){
            printo(" %s"PARAEND, var->name);
        }
    }
    print_listfoot(sess, "unlocked variable");

    return true_;
}

int vlock(struct session* sess, char* name){
    struct variable* var;
    int i, j, mode;

    var = var_ptr(sess, name, NULL);

    if(!var){
        printe(MSG_NOTFOUND, name);
        return false_;
    }


    if(var->type == VAR_SYS || var->type == VAR_CTL){
        printe(MSG_FORBID, var->name);
        return false_;
    }

    if(var->type == VAR_ARRAY){
        if(array_ptr(sess, name, 0, 0, NULL)) mode = 2;
        else mode = 1;
        while(var){
            if(mode == 1){
                for(i = 0; i < MAXARRAYMEMBERS; i++){
                    var = array_ptr(sess, name, i, -1, NULL);
                    if(!var) break;
                    var->flags |= VARF_LOCK;
                }
            } else {
                for(j = 0; j < MAXARRAYMEMBERS; j++)
                for(i = 0; i < MAXARRAYMEMBERS; i++){
                    var = array_ptr(sess, name, i, j, NULL);
                    if(!var) break;
                    var->flags |= VARF_LOCK;
                }
            }
        }
    } else var->flags |= VARF_LOCK;

    return true_;
}

int vunlock(struct session* sess, char* name){
    struct variable* var;
    int i, j, mode;

    var = var_ptr(sess, name, NULL);
    if(!var){
        printe(MSG_NOTFOUND, name);
        return false_;
    }

    if(var->type == VAR_SYS || var->type == VAR_CTL){
        printe(MSG_FORBID, var->name);
        return false_;
    }

    if(var->type == VAR_ARRAY){
        if(array_ptr(sess, name, 0, 0, NULL)) mode = 2;
        else mode = 1;
        while(var){
            if(mode == 1){
                for(i = 0; i < MAXARRAYMEMBERS; i++){
                    var = array_ptr(sess, name, i, -1, NULL);
                    if(!var) break;
                    var->flags &= ~VARF_LOCK;
                }
            } else {
                for(j = 0; j < MAXARRAYMEMBERS; j++)
                for(i = 0; i < MAXARRAYMEMBERS; i++){
                    var = array_ptr(sess, name, i, j, NULL);
                    if(!var) break;
                    var->flags &= ~VARF_LOCK;
                }
            }
        }
    } else var->flags &= ~VARF_LOCK;

    return true_;

}

int vstrcmp(struct session* sess){

    struct tok_s* tk1, *tk2;
    char* str1, *str2;
    struct variable* v1, *v2;

    tk1 = poptok(sess);
    tk2 = poptok(sess);

    if(!tk1 || !tk2){
        printe(MSG_MOREARGS, "vstrcmp");
        return false_;
    }

    switch(tk1->id){
        case TOK_VAR:
            v1 = var_ptr(sess, tk1->str, NULL);
            break;
        case TOK_ARRAY:
            v1 = array_ptr(sess, tk1->str, tk1->i, tk1->j, NULL);
            break;
        default:
            v1 = NULL;
            break;
    }

    if(v1){
        str1 = v1->exp;
    } else {
        str1 = tk1->str;
    }

    switch(tk2->id){
        case TOK_VAR:
            v2 = var_ptr(sess, tk2->str, NULL);
            break;
        case TOK_ARRAY:
            v2 = array_ptr(sess, tk2->str, tk2->i, tk2->j, NULL);
            break;
        default:
            v2 = NULL;
            break;
    }

    if(v2){
        str2 = v2->exp;
    } else {
        str2 = tk2->str;
    }

    if(!str1 || !str2 || !strlen(str1) || !strlen(str2)){
        printe(MSG_EMPTY, "");
        return false_;
    }

    if(!strcmp(str1, str2)){
//        tk1->val = 1;
        settokval(tk1, 1.0);
    } else {
//        tk1->val = 0;
        settokval(tk1, 0.0);
    }

//    tk1->type = TOKT_NUMBER;
//    tk1->id = TOK_INT;
//    tk1->flags |= TOKF_RES;

    pushtok(sess, tk1);

    //sess->cursess->calc++;

    return true_;

}

static inline int _do_rename(struct session* sess, struct variable* var, char* newname){

    char* oldname, *oldexp;

    oldname = var->name;
    oldexp = var->exp;

    var->name = (char*)malloc(strlen(newname) + strlen(var->exp) + 2);
    if(!var->name){
        printe(MSG_NOMEM);
        var->name = oldname;
        return false_;
    }

    var->exp = var->name + strlen(newname) + 1;

    strcpy(var->name, newname);

    strcpy(var->exp, oldexp);

    free(oldname);

    return true_;

}

int vrename(struct session* sess){

    struct variable* var;
    struct tok_s* tksrc, *tkdst;
    int i, j, mode, imax;

    tkdst = poptok(sess);
    tksrc = poptok(sess);

    if(!tkdst || !tksrc || !strlen(tkdst->str) || !strlen(tksrc->str)){
        printe(MSG_MOREARGS, "vrename");
        return false_;
    }

    if(!isalpha(*(tkdst->str)) && *(tkdst->str) != '_'){
        printe(MSG_FORMAT, tkdst->str);
        return false_;
    }

    if(var_ptr(sess, tkdst->str, NULL)){
        printe(MSG_EXISTS, tkdst->str);
        return false_;
    }

    var = var_ptr(sess, tksrc->str, NULL);
    if(!var){
        printe(MSG_BADARG, tksrc->str);
        return false_;
    }

    if(var->type == VAR_SYS || var->type == VAR_CTL || var->flags & VARF_LOCK){
        printe(MSG_FORBID, var->name);
        return false_;
    }

    if(var->type == VAR_ARRAY){

        //TODO: so ugly!
        if(array_ptr(sess, tksrc->str, 0, 0, NULL)) mode = 2;
        else mode = 1;

        if(mode == 1){
            for(i = 0; i < MAXARRAYMEMBERS;i++){
                var = array_ptr(sess, tksrc->str, i, -1, NULL);
                if(!var) return true_;
                if(!_do_rename(sess, var, tkdst->str)){
                    return false_;
                }
            }
        } else {

            imax = 0;
            while(array_ptr(sess, tksrc->str, imax, -1, NULL)) imax++;

            for(j = 0; j < MAXARRAYMEMBERS;j++){
                i = imax;
                while(i--){

                    var = array_ptr(sess, tksrc->str, i, j, NULL);
                    if(!var){
                        return true_;
                    }
                    if(!_do_rename(sess, var, tkdst->str)){
                        return false_;
                    }
                }
            }
        }

    } else {
        return _do_rename(sess, var, tkdst->str);
    }

    return true_;
}

int vdup(struct session* sess){

    struct variable* var;
    struct tok_s* tkdst, *tksrc;

    tksrc = poptok(sess);
    tkdst = poptok(sess);
    if(!tkdst || !tksrc || !strlen(tksrc->str) || !strlen(tkdst->str)){
        printe(MSG_MOREARGS, "vdup");
        return false_;
    }

    if(var_ptr(sess, tkdst->str, NULL)){
        printe(MSG_EXISTS, tkdst->str);
        return false_;
    }

    var = var_ptr(sess, tksrc->str, NULL);
    if(!var){
        printe(MSG_BADARG, tksrc->str);
        return false_;
    }

    if(var->type == VAR_ARRAY){
        return adup(sess, tkdst, tksrc);
//        var = array_ptr(sess, tksrc->str, tksrc->i, tksrc->j, NULL);
//        if(!var){
//            printe("array variable '%s[%i][%i]' does not defined."PARAEND, tksrc->str, tksrc->);
//            return false_;
//        }
    } else {
        create_var(sess, tkdst->str, VAR_NORM, -1, -1, var->exp);
        set_varval(sess, tkdst->str, var->val);
    }

    return true_;

}

