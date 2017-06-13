#include "double.h"

int _create_array1(struct session* sess, struct tok_s* first, struct tok_s** nexttk, int maxi, int resize){
    int idx, jdx;
    struct tok_s* tk;

    if(!resize && nexttk) tk = *nexttk;
    else tk = NULL;
    
    printd5("create array 1\n");

    if(resize){
        array_limits(sess, first->str, &idx, &jdx);
        idx++;
    } else {
        idx = 0;
    }

    for(; idx < maxi; idx++){

        if(tk){
            //printd16("tk->str: %s, val: %Lf, idx: %Lf\n", tk->str, tk->val, idx);
            if(tk->id != TOK_STR){

                if(!create_var(sess, first->str, VAR_ARRAY, idx, -1, NOVAREXP))
                    return false_;

                set_arrayval(sess, first->str, idx, -1, tokval(sess, tk));

            } else {

                if(!create_var(sess, first->str, VAR_ARRAY, idx, -1, tk->str))
                    return false_;

                set_arrayval(sess, first->str, idx, -1, 0);

            }
            tk = tk->next;
        } else {
            printd16("tk:NULL, idx: %i\n", idx);
            if(!create_var(sess, first->str, VAR_ARRAY, idx, -1, NOVAREXP)) return false_;
            set_arrayval(sess, first->str, idx, -1, 0);
        }
    }

    if(nexttk) *nexttk = tk;

    return true_;
}

int _create_array2(struct session* sess, struct tok_s* first, struct tok_s** nexttk, int idx, int _maxj, int resize, int* prevmaxj){
    int jdx, idx2;
    struct tok_s* tk;

    if(!resize && nexttk) tk = *nexttk;
    else tk = NULL;
    
    printd5("create array 2\n");

    if(resize < 0){
        //THIS FAILS IN SECOND PASS!
        if(*prevmaxj < 0){
            array_limits(sess, first->str, &idx2, &jdx);
            jdx++;
            *prevmaxj = jdx;
        } else {
            jdx = *prevmaxj;
        }
    } else {
        jdx = 0;
    }

    //printD("idx: %i, jdx: %i, resize: %i, _maxj: %i", idx, jdx, resize, _maxj);

    for(; jdx < _maxj; jdx++){

        //printD("jdx: %i", idx, jdx);

        if(tk){

            if(tk->id != TOK_STR){

                if(!create_var(sess, first->str, VAR_ARRAY, idx, jdx, NOVAREXP))
                    return false_;

                set_arrayval(sess, first->str, idx, jdx, tokval(sess, tk));

            } else {

                if(!create_var(sess, first->str, VAR_ARRAY, idx, jdx, tk->str))
                    return false_;

                set_arrayval(sess, first->str, idx, jdx, 0);

            }
            tk = tk->next;
        } else {
            if(!create_var(sess, first->str, VAR_ARRAY, idx, jdx, NOVAREXP)) return false_;
            set_arrayval(sess, first->str, idx, jdx, 0);
        }
    }

    if(nexttk) *nexttk = tk;

    return true_;
}

int set_array_direct(struct session* sess, struct tok_s* first, struct tok_s** tk){
	
    int mini, maxi, minj, maxj, i, j;
    struct variable* var;
    
    printd16("setting array %s[%i][%i]\n", first->str, first->i, first->j);

    if(!(var = var_ptr(sess, first->str, NULL))){
        printe(MSG_NOTFOUND, first->str);
        return false_;
    }
    
    array_limits(sess, first->str, &maxi, &maxj);

    if(first->i < 0){
        printe(MSG_BADARG, first->str);
        return false_;        
    } else {
        mini = first->i;
    }

    if(first->j < 0){
        minj = -1;
    } else {        
        minj = first->j;
    }

    if(minj > -1){
		for(i = mini; i <= maxi; i++){
			for(j = minj; j <= maxj; j++){
				if(*tk){
					set_arrayval(sess, first->str, i, j, tokval(sess, *tk));
					*tk = (*tk)->next;
				} else {
					return true_;
				}
			}			
		}
    } else {
		for(i = mini; i <= maxi; i++){
			if(*tk){
				set_arrayval(sess, first->str, i, -1, tokval(sess, *tk));
				*tk = (*tk)->next;
			} else {
				return true_;
			}
		}
    }

    return true_;	
}

int create_array_direct(struct session* sess, struct tok_s* first, struct tok_s** tk){
	
    int idx, _maxi, _maxj, resize = 0, prevmaxj;
//    struct tok_s* tk;
    struct variable* ptr;
    
    printd16("creating array %s[%i][%i]\n", first->str, first->i, first->j);

    if((ptr = var_ptr(sess, first->str, NULL))){
        resize = -1;
        if(ptr->i > -1){
            resize = 1;
        }
        if(ptr->j > -1){
            resize = 2;
        }
        //printe(MSG_EXISTS, first->str);
        //return false_;
    }

    if(resize == -1){
        printe(MSG_EXISTS, first->str);
        return false_;
    }

    printd16("creating array %s[%i][%i]\n", first->str, first->i, first->j);

    if(first->i < 0){
        printe(MSG_BADARG, first->str);
        return false_;
        //_maxi = MAXARRAYMEMBERS;
    } else {
        _maxi = first->i;
    }

    if(first->j < 0){
        _maxj = -1;
    } else {        
        _maxj = first->j;
    }

    //check for same dim resize
    if(resize == 1){
        if(_maxj > -1){
            printe(MSG_BADARG, first->str);
            return false_;
        }
    } else if(resize == 2){
        if(_maxi < 0 || _maxj < 0){
            printe(MSG_BADARG, first->str);
            return false_;
        }
    }

//  tk = first->next;
//	tk = NULL;

    if(_maxj > -1){

        if(!resize){

            idx = 0;

        } else {

            prevmaxj = -1;
            array_limits(sess, first->str, &resize, &idx);
            for(idx = 0; idx <= resize; idx++){
                if(!_create_array2(sess, first, tk, idx, _maxj, -1, &prevmaxj)) return false_;
            }

            array_limits(sess, first->str, &idx, &resize);
            idx++;

        }

        for(;idx < _maxi; idx++){
            if(!_create_array2(sess, first, tk, idx, _maxj, resize, NULL)) return false_;
        }

    } else {

        return _create_array1(sess, first, tk, _maxi, resize);

    }

    return true_;	
}

int create_array(struct session* sess){

    int idx, _maxi, _maxj, resize = 0, prevmaxj;
    struct tok_s* tk, *first = NULL, *last = NULL;
    struct variable* ptr;        

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
        printe(MSG_MOREARGS, "array");
        return false_;
    }
    
    printd5("create array: %s\n", first->str);

    if((ptr = var_ptr(sess, first->str, NULL))){
        resize = -1;
        if(ptr->i > -1){
            resize = 1;
        }
        if(ptr->j > -1){
            resize = 2;
        }
        //printe(MSG_EXISTS, first->str);
        //return false_;
    }

    if(resize == -1){
        printe(MSG_EXISTS, first->str);
        return false_;
    }

    printd16("creating array %s[%i][%i]\n", first->str, first->i, first->j);

    if(first->i < 0){
        printe(MSG_BADARG, first->str);
        return false_;
        //_maxi = MAXARRAYMEMBERS;
    } else {
        _maxi = first->i;
    }

    if(first->j < 0){
        _maxj = -1;
    } else {        
        _maxj = first->j;
    }

    //check for same dim resize
    if(resize == 1){
        if(_maxj > -1){
            printe(MSG_BADARG, first->str);
            return false_;
        }
    } else if(resize == 2){
        if(_maxi < 0 || _maxj < 0){
            printe(MSG_BADARG, first->str);
            return false_;
        }
    }

    tk = first->next;

    if(_maxj > -1){

        if(!resize){

            idx = 0;

        } else {

            prevmaxj = -1;
            array_limits(sess, first->str, &resize, &idx);
            for(idx = 0; idx <= resize; idx++){
                if(!_create_array2(sess, first, &tk, idx, _maxj, -1, &prevmaxj)) return false_;
            }

            array_limits(sess, first->str, &idx, &resize);
            idx++;

        }

        for(;idx < _maxi; idx++){
            if(!_create_array2(sess, first, &tk, idx, _maxj, resize, NULL)) return false_;
        }

    } else {

        return _create_array1(sess, first, &tk, _maxi, resize);

    }

    return true_;

}

int array_limits(struct session* sess, char* name, int* i, int* j){
    struct variable* var, *v;//, *prev;

    char *end;
    size_t sessnamelen;

    struct session* s;

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

//                    var = array_ptr(s, end, idx, jdx, prev);

                    //printD("cmp ok, var: %s"PARAEND, var?var->name:"NULL");                    

                    for(var = sess->cursess->vfirst; var; var = var->next){
                        if(!strcmp(var->name, name)){

                            *i = var->i;
                            *j = var->j;

                            break;
                        }
                    }

                    break;
                }
            }
            pthread_mutex_unlock(&sess->root->mx);

        } else {//this is variable name
            name++;
        }
    } else {

        for(var = sess->cursess->vfirst; var; var = var->next){
            if(!strcmp(var->name, name)){

                *i = var->i;
                *j = var->j;

                break;
            }
        }

    }

    if(!var) return false_;

    for(v = var; v; v = v->next){
        //prev = v;        
        if(!strcmp(v->name, name)){
            if(v->i > *i) *i = v->i;
            if(v->j > *j) *j = v->j;
        }
    }

//    *i = prev->i;
//    *j = prev->j;

    //printD("i: %i, j: %i", *i, *j);

    return true_;
}

struct variable* array_ptr(	struct session* sess, char* name, int idx, int jdx,
                            struct variable** prev){
    struct variable* var;

    char *end;
    size_t sessnamelen;

    struct session* s;

    if(idx < 0 || idx > MAXARRAYMEMBERS || jdx > MAXARRAYMEMBERS) return NULL;

    printd("getting array ptr of %s[%i][%i]"PARAEND, name, idx, jdx);

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

                    var = array_ptr(s, end, idx, jdx, prev);

                    //printD("cmp ok, var: %s"PARAEND, var?var->name:"NULL");

                    break;

                }
            }
            pthread_mutex_unlock(&sess->root->mx);

            return var;

        } else {//this is variable name
            name++;
        }
    }

    if(prev) *prev = sess->cursess->vfirst;

    for(var = sess->cursess->vfirst; var; var = var->next){
        if(!strcmp(var->name, name) && var->i == idx){
            if(jdx > -1){
                if(var->j == jdx){
                    return var;
                }
            } else {
                printd("ok, got it!"PARAEND);
                return var;
            }
        }
        if(prev) *prev = var;
    }

    return NULL;
}

fnum_t array_val(struct session* sess, char* name, int idx, int jdx){
    struct variable* var;

    var = array_ptr(sess, name, idx, jdx, NULL);
    if(var){
        return var->val;
    }

    return 0;
}

int set_arrayval(struct session* sess, char* name, int idx, int jdx, fnum_t val){

    struct variable* var;

    var = array_ptr(sess, name, idx, jdx, NULL);

//    printd14("setting variable '%s' to %s."PARAEND, name, clean_float(val, sess->cursess->tokbuf, sess->cursess->tokbufsize));

    if(!var){
        printe(MSG_ANOTFOUND, name, idx, jdx);
        return false_;
    }

    if(var->type == VAR_SYS || var->type == VAR_CTL || var->flags & VARF_LOCK){
        printe(MSG_FORBID, name);
        return false_;
    }

    var->val = val;

    return true_;
}

static inline int _set_arrayval_silent(struct session* sess, char* name, int idx, int jdx, fnum_t val){

    struct variable* var;

    var = array_ptr(sess, name, idx, jdx, NULL);

//    printd14("setting variable '%s' to %s."PARAEND, name, clean_float(val, sess->cursess->tokbuf, sess->cursess->tokbufsize));

    if(!var){
        return false_;
    }

    if(var->type == VAR_SYS || var->type == VAR_CTL || var->flags & VARF_LOCK){
        return false_;
    }

    var->val = val;

    return true_;
}

int afill(struct session* sess, char* name, int idx, int jdx, struct tok_s* tk){

    int i, j, imax, jmax, imin, jmin, mode, type;
    fnum_t val;

    if(tk->type == TOKT_NUMBER || tk->id == TOK_VAR || tk->id == TOK_ARRAY){
        val = tokval(sess, tk);
        type = 1;
    } else {
        type = 2;
    }

    if(array_ptr(sess, name, 0, 0, NULL)) mode = 2;
    else mode = 1;

    //TODO: so ugly!
    if(mode == 2){

        array_limits(sess, name, &imax, &jmax);

        if(idx >= 0){
            imax = idx + 1;
            imin = idx;
        } else {
//            imax = MAXARRAYMEMBERS;
            imax++;
            imin = 0;
        }

        if(jdx >= 0){
            jmax = jdx + 1;
            jmin = jdx;
        } else {
//            jmax = MAXARRAYMEMBERS;
            jmax++;
            jmin = 0;
        }

        for(j = jmin; j < jmax; j++){
            for(i = imin; i < imax; i++){
                if(type == 1){
                    if(!_set_arrayval_silent(sess, name, i, j, val)) break;
                } else {
                    if(!set_aexp_silent(sess, name, i, j, tk->str)) break;
                }
            }
        }
    } else {

        array_limits(sess, name, &imax, &jmax);

        if(idx > 0){
            imax = idx + 1;
            imin = idx;
        }
        else{
//            imax = MAXARRAYMEMBERS;
            imax++;
            imin = 0;
        }

        for(i = imin; i < imax; i++){
            if(type == 1){
                if(!_set_arrayval_silent(sess, name, i, -1, val)) break;
            } else {
                if(!set_aexp_silent(sess, name, i, -1, tk->str)) break;
            }
        }
    }
    return true_;
}


int show_adesc(struct session* sess, char* name, int idx, int jdx){
    struct variable* var;

    //if(is_array_name(sess, name, &aid) == true_) return show_adesc(sess, name);

    var = array_ptr(sess, name, idx, jdx, NULL);
    if(!var){
        printw(MSG_ANOTFOUND, name, idx, jdx);
        return false_;
    }

    printo(	" %s[%i][%i] /* %s */ [%s, %s]"PARAEND,
            name, var->i, var->j,
            var->desc?var->desc:NOTSET,
            var->type==VAR_SYS?"system":var->type==VAR_CTL?"control":"normal",
            var->flags&VARF_LOCK?"locked":"unlocked");
    return true_;
}


int rm_adesc(struct session* sess, char* name, int idx, int jdx){
    struct variable* var;

    //if(is_array_name(sess, name, &aid) == true_) return rm_adesc(sess, name);

    var = array_ptr(sess, name, idx, jdx, NULL);
    if(!var){
        printw(MSG_ANOTFOUND, name, idx, jdx);
        return false_;
    }

    if(var->type == VAR_SYS || var->type == VAR_CTL){
        printe(MSG_FORBID, name);
        return false_;
    }

    if(var->desc){
        free(var->desc);
        var->desc = NULL;
    }
    return true_;
}


int set_adesc(struct session* sess, char* name, int idx, int jdx, char* desc){
    struct variable* var;

    //if(is_array_name(sess, name, &aid) == true_) return set_adesc(sess, name);

    var = array_ptr(sess, name, idx, jdx, NULL);

    if(!var){
        printw(MSG_ANOTFOUND, name, idx, jdx);
        return false_;
    }

    if(var->type == VAR_SYS || var->type == VAR_CTL || var->flags & VARF_LOCK){
        printe(MSG_FORBID, name);
        return false_;
    }

    if(var->desc){
        printe(MSG_EXISTS, var->name);
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


int set_aexp(struct session* sess, char* name, int idx, int jdx, char* exp){
    struct variable* var;

    var = array_ptr(sess, name, idx, jdx, NULL);

    if(!var){
        printw(MSG_ANOTFOUND, name, idx, jdx);
        return false_;
    }

    if(var->type == VAR_SYS || var->type == VAR_CTL || var->flags & VARF_LOCK){
        printe(MSG_FORBID, name);
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

int set_aexp_silent(struct session* sess, char* name, int idx, int jdx, char* exp){
    struct variable* var;

    var = array_ptr(sess, name, idx, jdx, NULL);

    if(!var){
        return false_;
    }

    if(var->type == VAR_SYS || var->type == VAR_CTL || var->flags & VARF_LOCK){
        return false_;
    }

    var->name = (char*)realloc(var->name, strlen(name) + strlen(exp) + 2);
    if(!var->name){
        return false_;
    }

    var->exp = var->name + strlen(name) + 1;

    strcpy(var->exp, exp);
    
	if(var->pre){
		delete_pre(sess, var->pre);
		var->pre = NULL;
	}    

    return true_;
}

int cat_aexp(struct session* sess, char* name, int idx, int jdx, char* exp){
    struct variable* var;

    var = array_ptr(sess, name, idx, jdx, NULL);

    if(!var){
        printw(MSG_ANOTFOUND, name, idx, jdx);
        return false_;
    }

    if(var->type == VAR_SYS || var->type == VAR_CTL || var->flags & VARF_LOCK){
        printe(MSG_FORBID, name);
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

int adup(struct session* sess, struct tok_s* tkdst, struct tok_s* tksrc){

    struct variable* var;
    int i, j, imax, jmax;

    array_limits(sess, tksrc->str, &imax, &jmax);
    imax++;
    jmax++;

    if(tksrc->i < 0 && tksrc->j < 0){//copy whole

        for(i = 0; i < imax; i++){

            for(j = 0; j < jmax; j++){

                var = array_ptr(sess, tksrc->str, i, j, NULL);
                if(!var){
                    break;
                }
                create_var(sess, tkdst->str, VAR_ARRAY, i, j, var->exp);
                set_arrayval(sess, tkdst->str, i, j, var->val);
            }

        }

    } else if(tksrc->i < 0){//copy all rows of col j

        for(i = 0; i < imax; i++){

            var = array_ptr(sess, tksrc->str, i, tksrc->j, NULL);
            if(!var){
                break;
            }
            create_var(sess, tkdst->str, VAR_ARRAY, i, 0, var->exp);
            set_arrayval(sess, tkdst->str, i, 0, var->val);

        }

    } else if(tksrc->j < 0){//copy all cols of row i

        for(j = 0; j < jmax; j++){

            var = array_ptr(sess, tksrc->str, tksrc->i, j, NULL);
            if(!var){
                break;
            }
            create_var(sess, tkdst->str, VAR_ARRAY, 0, j, var->exp);
            set_arrayval(sess, tkdst->str, 0, j, var->val);
        }

    }

    return true_;

}
