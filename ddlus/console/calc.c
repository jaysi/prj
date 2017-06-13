#include "double.h"

fnum_t radtodeg(fnum_t rad){
    return ((PI*rad)/(180));
}

fnum_t degtorad(fnum_t deg){
    return ((180*deg)/(PI));
}

inum_t intval(fnum_t val){
    inum_t ival;
    ival = val;
    return (fnum_t)ival;
}

fnum_t fval(inum_t val){
    fnum_t fv;
    fv = val;
    return (inum_t)fv;
}

fnum_t round_float(fnum_t val, inum_t prec, enum tok_t mode){

    fnum_t f, n, nf, ff;

    fnum_t mult = pow(10, prec);
    f = modfl(val, &n);

    ff = modfl(f*mult, &nf);

//	if(!strcmp(mode, "up")){
//		if(ff) nf++;
//	} else if(!strcmp(mode, "down")){
//		if(ff) nf--;
//	} else if(!strcmp(mode, "trim")){

//	} else {
//		if(ff >= 0.5) nf++;
//	}

    switch(mode){
        case TOK_UP:
            if(ff) nf++;
            break;
        case TOK_DOWN:
            if(ff) nf--;
            break;
        case TOK_TRIM:
            break;
        default://TOK_NORMAL
            if(ff >= 0.444444444445) nf++;
            break;
    }

    return (n + nf/mult);
}

fnum_t acount(struct session* sess, struct tok_s* tk){
    int i, j;

    if(!array_limits(sess, tk->str, &i, &j)){
        printw(MSG_ANOTFOUND, tk->str, tk->i, tk->j);
        return 0.0;
    }

    if(i < 0){
        printw(MSG_BADARG, tk->str);
        return 0.0;
    } else {
        if(j < 0){
            printd18("i: %i", i+1);
            return fval(i+1);
        } else {
            printd18("i: %i, j: %i", i+1, j+1);
            return fval((i+1)*(j+1));
        }
    }

    return -1.0;
}

int calc_logic(struct session* sess, struct tok_s* tk){
    struct tok_s* tk1, *tk2;
    fnum_t ans, val2;

    printd12("extracting tok1\n");
    tk1 = poptok(sess);
    if(!tk1){
        printd8("failed\n");
        return false_;
    }
   // printd5("tk1:: str: %s, val: %Lf, resolved = %i\n", tk1->str, tk1->val, tk1->resolved);

    switch(tk->id){
        case TOK_NOT:
        ans = tokval(sess, tk1)?0:1;
        //tk1->val = ans;
        settokval(tk1, ans);
        push_calc(sess, tk1);
        sess->cursess->calc++;
        return true_;
        break;
        default:
        break;
    }

    printd12("extracting tok2\n");
    tk2 = poptok(sess);
    if(tk2){
        val2 = tokval(sess, tk2);
        //printd8("failed\n");
        //return false_;
    } else {
    	val2 = var_val(sess, ANSVARNAME);
        //printe(MSG_MOREARGS, "logic");
    }
    //printd5("tk2:: str: %s, val: %Lf, resolved = %i\n", tk2->str, tk2->val, tk2->resolved);

    switch(tk->id){

        case TOK_LT:
            ans = val2<tokval(sess, tk1)?1:0;
        break;

        case TOK_GT:
            ans = val2>tokval(sess, tk1)?1:0;
        break;

        case TOK_EQ:
            ans = val2==tokval(sess, tk1)?1:0;
        break;

        case TOK_NE:
            ans = val2!=tokval(sess, tk1)?1:0;
        break;

        case TOK_LE:
            ans = val2<=tokval(sess, tk1)?1:0;
        break;

        case TOK_GE:
            ans = val2>=tokval(sess, tk1)?1:0;
        break;

        case TOK_AND_AND:
            ans = val2&&tokval(sess, tk1)?1:0;
        break;

        case TOK_OR_OR:
            ans = val2||tokval(sess, tk1)?1:0;
        break;

        default:
        break;
    }

    printd5("ans is now %Lf\n", ans);

    if(tk2){
//    	tk2->val = ans;
        settokval(tk2, ans);
    	push_calc(sess, tk2);
    } else {
//    	tk1->val = ans;
        settokval(tk1, ans);
    	push_calc(sess, tk1);
    }

    sess->cursess->calc++;

    return true_;
}

int calc_basic(struct session* sess, struct tok_s* tk){
    struct tok_s* tk1, *tk2, *tkz = NULL, *first, *last;
    fnum_t ans, val2, ansreg;
    struct variable* var = NULL;

    printd("extracting tok1\n");
    tk1 = poptok(sess);
    if(!tk1){
        printd("failed\n");
        return false_;
    }
    printd5("tk1:: str: %s, val: %Lf, flags = %i\n", tk1->str, tk1->val, tk1->flags);

    printd("extracting tok2\n");
    tk2 = poptok(sess);
    if(!tk2){
    	val2 = var_val(sess, ANSVARNAME);
        printd("failed\n");
        //return false_;
    } else {
    	val2 = tokval(sess, tk2);
    }
    //printd5("tk2:: str: %s, val: %Lf, resolved = %i\n", tk2?tk2->str:"", tk2?tk2->val:val2, tk2?tk2->flags:0);

    //printval(sess, val2);
	printd5("(%i)%s\n", tk->id, tk->str);
    //printval(sess, tk1->val);
	//printd5("\n");

    switch(tk->id){

//	case TOK_ABR_OPEN:
//
//		if(sess->cursess->pre->exp->flags & EXPF_ABR){
//			printe("array already open.\n");
//			return false_;
//		}
//
//		sess->cursess->pre->exp->flags |= EXPF_ABR;
//
//		break;
//
//	case TOK_ABR_CLOSE:
//
//		if(!(sess->cursess->pre->exp->flags |= EXPF_ABR)){
//			printe("array not open yet.\n");
//			return false_;
//		}
//
//		sess->cursess->pre->exp->flags &= ~EXPF_ABR;
//
//		break;

        case TOK_PLUS:
            ans = val2 + tokval(sess, tk1);
            //printd5("%Lf + %Lf = %Lf\n", val2, tokval(sess, tk1), ans);
        break;

        case TOK_MINUS:
            ans = val2 - tokval(sess, tk1);
            //printd5("%Lf - %Lf = %Lf\n", tk2->val, tk1->val, ans);
        break;
        case TOK_MULT:
            ans = val2 * tokval(sess, tk1);
            //printd5("%Lf * %Lf = %Lf\n", tk2->val, tk1->val, ans);
        break;

        case TOK_DIV:
            ans = val2 / tokval(sess, tk1);
            //printd5("%Lf / %Lf = %Lf\n", tk2->val, tk1->val, ans);
        break;

        case TOK_POW:
            ans = pow(val2, tokval(sess, tk1));
            break;

        case TOK_MOD:
            //ans = intval(tokval(sess, tk2)) % intval(tokval(sess, tk1));
            ans = fmod(val2, tokval(sess, tk1));
            break;

        case TOK_ASSIGN:

        	printd17("assignment got!\n");
        	_show_prefix(sess);
        	_show_stack(sess);

        	if(!tk2) return false_;

        	ansreg = var_val(sess, ANSVARNAME);

            //ans = tokval(sess, tk1);

            printd17("requesting %s: %s\n", tk2->id == TOK_ARRAY?"array":"variable", tk2->str);

            var = var_ptr(sess, tk2->str, NULL);
			if(!var) var = array_ptr(sess, tk2->str, 0, 0, NULL);

//            if(tk2->id == TOK_ARRAY) var = array_ptr(sess, tk2->str, tk2->i, tk2->j, NULL);
//            else var = var_ptr(sess, tk2->str, NULL);

            printd17("tk1(ans): %s $id: %i, tk2(var): %s[%i][%i]\n", tk1->str, tk1->id, tk2->str, tk2->i, tk2->j);
            //printval(sess, ans);

            if(!var){

            	printd17("no vars defined, or an array=(...) form!\n");

            	if(tk2->id == TOK_STR && tk2->i < 0){

            		if(!create_var(sess, tk2->str, VAR_NORM, -1, -1, sess->cursess->pre->exp->varinfix)){
            			sess->cursess->pre->exp->varinfix = NULL;
            			return false_;
            		} else {
            			sess->cursess->pre->exp->varinfix = NULL;
            			set_varval(sess, tk2->str, tokval(sess, tk1));
            		}

            	} else {

            		//array def or array=(...) form

            		first = tk2;
            		last = tk1;
            		first->next = last;

					if(tk2->id != TOK_ARRAY){

						//probably a array[i][j]=(...) form definition

						push_calc(sess, tk2);
						push_calc(sess, tk1);

					    //reverse stack
						last = NULL;
						first = NULL;
					    while((tkz = poptok(sess))){
					    	printd16("tkz: %s\n", tkz->str);
					        if(!last){
					            last = tkz;
					            last->next = NULL;
					            printd16("last: %s\n", last->str);
					        } else {
					            tkz->next = first;
					            printd16("tkz->next: %s\n", tkz->next->str);
					        }

					        first = tkz;
							printd16("first: %s\n", first->str);
					    }

					    if(!first){
					        printe(MSG_MOREARGS, "...");
					        return false_;
					    }

					}

					tkz = first->next;

            		if(!create_array_direct(sess, first, tkz?&tkz:NULL)){
            			return false_;
            		}

	                //printw(MSG_ANOTFOUND, tk2->str, tk2->i, tk2->j);
	                //return false_;
            	}

            } else { //if(var)

            	printd17("var defined!\n");

	            if(var->i > -1){
	            	printd17("array defined!\n");
	            	if(tk2->i > 0){

	            		set_arrayval(sess, tk2->str, tk2->i, tk2->j, tk1->val);

	            	} else {

						//probably a array=(...) form definition

						push_calc(sess, tk2);
						push_calc(sess, tk1);

					    //reverse stack
						last = NULL;
						first = NULL;
					    while((tkz = poptok(sess))){
					    	printd16("tkz: %s\n", tkz->str);
					        if(!last){
					            last = tkz;
					            last->next = NULL;
					            printd16("last: %s\n", last->str);
					        } else {
					            tkz->next = first;
					            printd16("tkz->next: %s\n", tkz->next->str);
					        }

					        first = tkz;
							printd16("first: %s\n", first->str);
					    }

					    if(!first){
					        printe(MSG_MOREARGS, "...");
					        return false_;
					    }

					    tkz = first->next;

					    //fill array

					    if(!set_array_direct(sess, first, &tkz)){
					    	return false_;
					    }

					}

	            } else /*if(tk2->id == TOK_STR && tk2->i < 0)*/ var->val = tokval(sess, tk1);
            }

            ans = ansreg;

            var = var_ptr(sess, ANSVARNAME, NULL);//)set_varval(sess, ANSVARNAME, ansreg);
            if(var) var->val = ansreg;

            break;

        case TOK_AND:
            ans = intval(val2)&intval(tokval(sess, tk1));
        break;

        case TOK_OR:
            ans = intval(val2)|intval(tokval(sess, tk1));
        break;

        case TOK_ABR_OPEN:

			if(!tk2) return false_;

			if(tk2->id != TOK_ARRAY) tk2->id = TOK_ARRAY;

            if(tk2->i == -1){
                printd17("set array i to %i"PARAEND, intval(tokval(sess, tk1)));
                tk2->i = intval(tokval(sess, tk1));
            } else {
                printd17("set array j to %i"PARAEND, intval(tokval(sess, tk1)));
                tk2->j = intval(tokval(sess, tk1));
            }

            //tk2->flags &= ~TOKF_RES;

            break;

        case TOK_PLUS_EQ:

        	ans = val2 + tokval(sess, tk1);

        	if(tk2) {

        		if(tk2->id == TOK_VAR) {

        			set_varval(sess, tk2->str, ans);

        		} else if(tk2->id == TOK_ARRAY) {
        			set_arrayval(sess, tk2->str, tk2->i, tk2->j, ans);
        		} else {
        			//error
        			printe(MSG_BADARG, tk2->str);
        		}

        	} else {

        		//set_varval(sess, ANSVARNAME, ans);
        		var = var_ptr(sess, ANSVARNAME, NULL);
        		if(var) var->val = ans;

        	}

        case TOK_MINUS_EQ:

            ans = val2 - tokval(sess, tk1);

            if(tk2) {

                if(tk2->id == TOK_VAR) {

                    set_varval(sess, tk2->str, ans);

                } else if(tk2->id == TOK_ARRAY) {
                    set_arrayval(sess, tk2->str, tk2->i, tk2->j, ans);
                } else {
                    //error
                    printe(MSG_BADARG, tk2->str);
                }

            } else {

                //set_varval(sess, ANSVARNAME, ans);
                var = var_ptr(sess, ANSVARNAME, NULL);
                if(var) var->val = ans;

            }


        	break;

        	case TOK_PLUS_PLUS:

        		if(!tk2) ans = tokval(sess, tk1) + 1;
        		else ans = tokval(sess, tk2) + tokval(sess, tk1);

        		break;

            case TOK_MINUS_MINUS:

                if(!tk2) ans = tokval(sess, tk1) - 1;
                else ans = tokval(sess, tk2) + tokval(sess, tk1);

                break;

        default:
            printd("tok \'%s\'(%i) not implemented\n", tk->str, tk->id);
            return false_;
        break;
    }

    //printd5("ans is now", ans);
    //printval(sess, ans);
    //printd5("\n", ans);

    if(tk2){
//    	tk2->val = ans;
        settokval(tk2, ans);
    	push_calc(sess, tk2);
    } else {
//    	tk1->val = ans;
        settokval(tk1, ans);
    	push_calc(sess, tk1);
    }

    sess->cursess->calc++;

    return true_;
}

int calc_math_fn(struct session* sess, struct tok_s* tk){
    struct tok_s* tk1, *tk2, *tk3, *anstk;
    fnum_t ans;

    tk1 = poptok(sess);
    if(!tk1){
        printd("failed to x tk1\n");
        return false_;
    }

    anstk = tk1;

    printd("extracting tok1 %s\n", tk1->str);

    switch(tk->id){
        case TOK_SIN:
            ans = sinl(radtodeg(tokval(sess, tk1)));
        break;
        case TOK_ASIN:
            ans = degtorad(asin(tokval(sess, tk1)));
        break;
        case TOK_SINH:
            ans = sinhl(radtodeg(tokval(sess, tk1)));
        break;
        case TOK_COS:
            ans = cosl(radtodeg(tokval(sess, tk1)));
        break;
        case TOK_ACOS:
            ans = degtorad(acos(tokval(sess, tk1)));
        break;
        case TOK_COSH:
            ans = coshl(radtodeg(tokval(sess, tk1)));
        break;
        case TOK_TAN:
            ans = tanl(radtodeg(tokval(sess, tk1)));
        break;
        case TOK_ATAN:
            ans = degtorad(atan(tokval(sess, tk1)));
        break;
        case TOK_RAD:
            ans = radtodeg(tokval(sess, tk1));
        break;
        case TOK_DEG:
            ans = degtorad(tokval(sess, tk1));
        break;
        case TOK_LOG:
            ans = log10l(tokval(sess, tk1));
        break;
        case TOK_LN:
            ans = logl(tokval(sess, tk1));
        break;
        case TOK_SQRT:
            ans = sqrtl(tokval(sess, tk1));
        break;
        case TOK_ABS:
            ans = fabs(tokval(sess, tk1));
        break;

        case TOK_ROUND:

            tk2 = poptok(sess);
            if(!tk2){
                ans = round_float(tokval(sess, tk1), 2LL, TOK_NORMAL);
                break;
            }

            tk3 = poptok(sess);
            if(!tk3){
                ans = round_float(tokval(sess, tk2), intval(tokval(sess, tk1)), TOK_NORMAL);
                break;
            }

            printd("extracting tok2 %s\n", tk2->str);

            ans = round_float(tokval(sess, tk3), intval(tokval(sess, tk2)), tk1->id);

//        	tk1->resolved = 1;//for the TOKT_STRING case
            tk1->flags |= TOKF_RES;

            anstk = tk3;

        break;

        case TOK_NROUND:
            ans = round_float(tokval(sess, tk1), 2LL, TOK_NORMAL);
            break;

        case TOK_JDAY:
            ans = jdate(sess, tk1);
        break;

        case TOK_GDAY:
            ans = gdate(sess, tk1);
        break;

        case TOK_COUNT:
            ans = acount(sess, tk1);
            printd18("acount ans is");
            //printval(sess, ans);
            //TODO: this leads to bug?
            anstk->flags |= TOKF_RES;
        break;

        case TOK_ANS_REF:
            ans = array_val(sess, "ans", tokval(sess, tk1), 0);
            break;

        default:
            printd("math function \'%s\' not implemented\n", tk->str);
            return false_;
        break;
    }

#ifndef NDEBUG
    printd("ans is now ");
    //printval(sess, ans);
    printo("\n");
#endif

//    anstk->val = ans;
    settokval(anstk, ans);

    push_calc(sess, anstk);

    sess->cursess->calc++;

    return true_;

}

int runprefix(struct session* sess){
    struct tok_s* ent, *ent2;

    _show_prefix(sess);
    _show_stack(sess);

    DEBUG1_DEF();
    DEBUG1();

    assert(sess->cursess->pre);
    assert(sess->cursess->pre->exp);
    //TODO: this fails on load.
    //assert(sess->cursess->pre->exp->prefixfirst);
    pop_all(sess);
    assert(!sess->cursess->pre->exp->stacktop);
    assert(sess->cursess->pre);
    assert(sess->cursess->pre->exp);
    //assert(sess->cursess->pre->exp->prefixfirst);

    _show_prefix(sess);
    _show_stack(sess);

    printd("save prefix first: %s"PARAEND, sess->cursess->pre->exp->saveprefixfirst->str);

    sess->cursess->pre->exp->prefixfirst = sess->cursess->pre->exp->saveprefixfirst;

    while( sess->cursess->pre->exp->prefixfirst ) {

        sess->cursess->pre->exp->prefixfirst->prenext = sess->cursess->pre->exp->prefixfirst->next;

        ent = sess->cursess->pre->exp->prefixfirst;
        sess->cursess->pre->exp->prefixfirst = sess->cursess->pre->exp->prefixfirst->next;

        printd("ent = %s, next = %s\n", ent->str,
                ent->next?ent->next->str:"NULL");

        switch( ent->type ){
            case TOKT_MATH_BASIC:
                printd("got math basic %s\n", ent->str);
                if(!calc_basic(sess, ent)){
                    printe(MSG_BADCALC);
                    return false_;
                }
            break;
            case TOKT_MATH_FUNC:
                if(!calc_math_fn(sess, ent)){
                    printe(MSG_BADCALC);
                    return false_;
                }
            break;
            case TOKT_MATH_LOGIC:
                if(!calc_logic(sess, ent)){
                    printe(MSG_BADLOGIC);
                    return false_;
                }
            break;
            case TOKT_FUNC:
                if(!run_fn(sess, ent)){
                    //printe("error running function \"%s\"."PARAEND, ent->str);
                    return false_;
                }
                //return true_;
            break;
            case TOKT_COMMAND:
                if(!run_cmd(sess, ent)){
                    //printe("error running command \"%s\"."PARAEND, ent->str);
                    return false_;
                }
                return true_;
            break;
            case TOKT_BLOCK:

            if(sess->cursess->pre->exp->flags & EXPF_FOR_ADVANCE){
                printd_for("running code block of FOR_ADVANCE"PARAEND);
                if(!_run_codeblock(sess, ent)){
                    return false_;
                }
            } else {
                push_calc(sess, ent);
            }

            break;
            case TOKT_NUMBER:
                sess->cursess->calc++;
            case TOKT_SPEC:
                if(ent->id == TOK_COL) break;
            default:
                //now use stack as a calc stack
                push_calc(sess, ent);
            break;
        }

        //ent = ent->next;
    }

    //handle the "there is at least a calculation done" case.
    if(sess->cursess->calc){
        ent = poptok(sess);
        if(!ent){//unless previous is an IF... or recalc

            printd14("no tokens left, this may happen when using if or recalc.\n");

            switch(sess->cursess->pre->exp->ifcond){
                case IFCOND_NOIF:
                    break;
                default:
                    sess->cursess->pre->exp->ifcond = IFCOND_NOIF;
                    break;
            }

            //printe("there is an error in expression.\n");
            //return false_;
            //goto end;
            return true_;//TODO: FIX THIS

        } else {
            printd8("ent->str [setting ans] = %s\n", ent->str);

            if((ent2 = poptok(sess))){
                switch(ent2->id){
                    case TOK_VAR:
                    case TOK_ARRAY:
//                    case TOK_INT:
//                    case TOK_REAL:
                        break;
                    default:
                        printe(MSG_BADEXP, ent2->str);
                        return false_;
                        break;
                }
            }

            set_ansvar(sess, tokval(sess, ent));
        }
    } else {
        //if there is no calc done, empty stack too!
        if(sess->cursess->next && strlen(sess->cursess->next)) pop_all(sess);
    }

    return true_;
}

