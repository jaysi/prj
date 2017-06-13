#include "double.h"

/*	internal functions	*/
#ifndef QT
int init_clipboard(struct session *sess){
    return true_;
}
#endif

size_t realstrlen(char* str){
    size_t i = 0UL;
    while(str[i++]);
    return i-1;
}

int cp(struct session* sess, char* str){

#if (defined(WIN32)&&(!defined(QT)))
    size_t len = realstrlen(str) + 1;
    HGLOBAL hglbCopy;

    if (!OpenClipboard(GetClipboardOwner())) return false_;

    EmptyClipboard();

    hglbCopy = GlobalAlloc(GMEM_FIXED, (len) * sizeof(char));
    if (hglbCopy == NULL)
    {
        CloseClipboard();
        return false_;
    }

    //strcpy((char*)hglbCopy, str);
    memcpy(hglbCopy, str, len);

    if( SetClipboardData(CF_TEXT, hglbCopy) == NULL){
    printe("failed to copy."PARAEND);
        return false_;
    }

    CloseClipboard();

    //printi(MSG_OK);

#elif (defined(QT))
    qcp(sess, str);
    //printi(MSG_OK);
#else
    printw(MSG_IMPLEMENT, "cc");
#endif
    return true_;
}

int do_autorun(struct session* sess){

    struct tok_s* tk;
    struct variable* var;
    char* autoexp;

    tk = poptok(sess);
    if(!tk){//show autorun status
        switch(sess->cursess->autorun){
            case AUTORUN_NOTDEF:
                printi("autorun is not defined."PARAEND);
                break;
            case AUTORUN_DEFON:
            printi("auto run is enable, \"%s\"; try auto stop."PARAEND, "");
                break;
            case AUTORUN_DEFOFF:
            printi("auto run is defined but disable, \"%s\"; try auto resume."PARAEND, "");
                break;
            case AUTORUN_DEFNOW:
            printi("auto run is defined right now, \"%s\"."PARAEND, "");
                break;
            default:
                break;
        }
    } else {//set status
        if(tk->id == TOK_RESUME){
            if(!sess->cursess->autopre || sess->cursess->autorun == AUTORUN_NOTDEF){
                printw(MSG_EMPTY, tk->str);
                return false_;
            } else {
                sess->cursess->autorun = AUTORUN_DEFNOW;//resume after this
            }
        } else if(tk->id == TOK_STOP){
            if(sess->cursess->autorun == AUTORUN_NOTDEF){
                printw(MSG_EMPTY, tk->str);
                return false_;
            } else {
                sess->cursess->autorun = AUTORUN_DEFOFF;
            }
        } else {//set autorun
            if(sess->cursess->autopre){
                delete_pre(sess, sess->cursess->autopre);
                free(sess->cursess->autopre);
            }

            if(tk->id == TOK_VAR){
                var = var_ptr(sess, tk->str, NULL);
                assert(var);
                autoexp = var->exp;
            } else if(tk->id == TOK_ARRAY){
                var = array_ptr(sess, tk->str, tk->i, tk->j, NULL);
                assert(var);
                autoexp = var->exp;
            } else {
                autoexp = tk->str;
            }

            if(!_precompile(sess, autoexp, &sess->cursess->autopre)){
                sess->cursess->autorun = AUTORUN_NOTDEF;
                return false_;
            }

            sess->cursess->autorun = AUTORUN_DEFNOW;
            return true_;
        }
    }

    return true_;

}

int mode(struct session* sess){
    struct tok_s* tk;
    tk = poptok(sess);
    if(!tk){

        switch(sess->cursess->mode1){
            case SESSM1_NR:
                snprintf(sess->cursess->outbuf, sess->cursess->outbufsize, "%s@%s: %s", INFO_ICON, sess->cursess->name, "NORMAL");
            break;
            case SESSM1_ACC:
                snprintf(sess->cursess->outbuf + strlen(sess->cursess->outbuf), sess->cursess->outbufsize - strlen(sess->cursess->outbuf), "%s", "FINANCIAL");
            break;
            case SESSM1_PROG:
                snprintf(sess->cursess->outbuf + strlen(sess->cursess->outbuf), sess->cursess->outbufsize - strlen(sess->cursess->outbuf), "%s", "PROGRAMMING");
            break;
            default:
                snprintf(sess->cursess->outbuf + strlen(sess->cursess->outbuf), sess->cursess->outbufsize - strlen(sess->cursess->outbuf), "%s", "UNKNOWN");
            break;
        }

//        if(sess->cursess->mode2 & SESSM2_DET){
//            snprintf(sess->cursess->outbuf + strlen(sess->cursess->outbuf), sess->cursess->outbufsize - strlen(sess->cursess->outbuf), "%s", "| DETAILS");
//        } else {
//            snprintf(sess->cursess->outbuf + strlen(sess->cursess->outbuf), sess->cursess->outbufsize - strlen(sess->cursess->outbuf), "%s", "| NO DETAILS");
//        }

//        if(sess->cursess->mode2 & SESSM2_LOUD){
//            snprintf(sess->cursess->outbuf + strlen(sess->cursess->outbuf), sess->cursess->outbufsize - strlen(sess->cursess->outbuf), "%s", "| LOUD");
//        } else {
//            snprintf(sess->cursess->outbuf + strlen(sess->cursess->outbuf), sess->cursess->outbufsize - strlen(sess->cursess->outbuf), "%s", "| SILENT");
//        }

//        if(sess->cursess->mode2 & SESSM2_LINE){
//            snprintf(sess->cursess->outbuf + strlen(sess->cursess->outbuf), sess->cursess->outbufsize - strlen(sess->cursess->outbuf), "%s", "| LINEAR LOAD");
//        } else {
//            snprintf(sess->cursess->outbuf + strlen(sess->cursess->outbuf), sess->cursess->outbufsize - strlen(sess->cursess->outbuf), "%s", "| BULK LOAD");
//        }

//        if(sess->cursess->mode2 & SESSM2_ANS){
//            snprintf(sess->cursess->outbuf + strlen(sess->cursess->outbuf), sess->cursess->outbufsize - strlen(sess->cursess->outbuf), "%s", "| SHOW RESULTS");
//        } else {
//            snprintf(sess->cursess->outbuf + strlen(sess->cursess->outbuf), sess->cursess->outbufsize - strlen(sess->cursess->outbuf), "%s", "| HIDE RESULTS");
//        }

        strcat(sess->cursess->outbuf, PARAEND);

        textout_(sess->cursess->outbuf);

        return true_;
    }


    do{ switch(tk->id){
        case TOK_NORMAL:
            sess->cursess->outbuf[0] = 0;
            sess->cursess->mode1 = SESSM1_NR;
        break;
        case TOK_ACC:
            sess->cursess->outbuf[0] = 0;
            sess->cursess->mode1 = SESSM1_ACC;
        break;
        case TOK_PROG:
            sess->cursess->outbuf[0] = 0;
            sess->cursess->expbuf[0] = 0;
            sess->cursess->mode1 = SESSM1_PROG;
        break;
//        case TOK_DET:
//            sess->cursess->mode2 |= SESSM2_DET;
//            //printi("mode2 set to details."PARAEND);
//        break;
//        case TOK_NODET:
//            sess->cursess->mode2 &= ~SESSM2_DET;
//            //printi("mode2 set to no details."PARAEND);
//        break;

//        case TOK_NOSILENT:
//            sess->cursess->mode2 |= SESSM2_LOUD;
//            //printi("mode2 set to loud."PARAEND);
//        break;

//        case TOK_SILENT:
//            sess->cursess->mode2 &= ~SESSM2_LOUD;
//            //printi2("mode2 set to silent."PARAEND);
//        break;

//        case TOK_BULK:
//            sess->cursess->mode2 &= ~SESSM2_LINE;
//            //printi("mode2 set to bulk load mode."PARAEND);
//        break;

//        case TOK_LINE:
//            sess->cursess->mode2 |= SESSM2_LINE;
//            //printi("mode2 set to single line load mode."PARAEND);
//        break;

//        case TOK_NOANS:
//            sess->cursess->mode2 &= ~SESSM2_ANS;
//            //printi("mode2 set to do not show answer mode."PARAEND);
//        break;

//        case TOK_SHOWANS:
//            sess->cursess->mode2 |= SESSM2_ANS;
//            //printi("mode2 set to show answer mode."PARAEND);
//        break;

        default:
            printw(MSG_BADARG, tk->str);
        break;
    } } while((tk = poptok(sess)));

    return true_;
}

int run_fn(struct session* sess, struct tok_s* ent){
    struct tok_s* tk, *tk1;
    char buf[MAXNUMLEN];

    switch(ent->id){

        case TOK_CP:
            tk = poptok(sess);
            if(!tk){//copy last answer

                //printD("copying last answer."PARAEND);

                cp(sess,clean_float(sess->cursess, var_val(sess, ANSVARNAME),
                                    sess->cursess->tokbuf,
                                    sess->cursess->tokbufsize));

            } else {
                switch(tk->id){
                    case TOK_VAR:
                        cp(sess,clean_float(sess->cursess, var_val(sess, tk->str),
                                            sess->cursess->tokbuf,
                                            sess->cursess->tokbufsize));
                        break;
                    case TOK_ARRAY:
                        cp(sess,clean_float(sess->cursess, array_val(sess, tk->str, tk->i, tk->j),
                                            sess->cursess->tokbuf,
                                            sess->cursess->tokbufsize));
                        break;
                    case TOK_INT:
                    case TOK_REAL:
                    case TOK_SCIENCE:
                        cp(sess,clean_float(sess->cursess, tokval(sess, tk),
                                            sess->cursess->tokbuf,
                                            sess->cursess->tokbufsize));
                        break;
                    case TOK_ACC:
                        cp(sess, sess->cursess->outbuf);
                        break;
                    default:
                        //printD("copying %s."PARAEND, tk->str);
                        cp(sess, tk->str);
                        break;
                }
                //pushtok(sess, tk);
            }

        break;//CP

        case TOK_SEPCP:

        tk = poptok(sess);
        if(!tk){//copy last answer
            cp(	sess,sep_val(sess->cursess, var_val(sess, ANSVARNAME), sess->cursess->tokbuf,
                sess->cursess->tokbufsize, buf, MAXNUMLEN));
        } else {

            switch(tk->id){
                case TOK_VAR:
                    cp(sess,sep_val(sess->cursess, var_val(sess, tk->str),
                                        sess->cursess->tokbuf,
                                        sess->cursess->tokbufsize,
                                        buf, MAXNUMLEN));
                    break;
                case TOK_ARRAY:
                    cp(sess,sep_val(sess->cursess, array_val(sess, tk->str, tk->i, tk->j),
                                                sess->cursess->tokbuf,
                                                sess->cursess->tokbufsize,
                                                buf, MAXNUMLEN));
                            break;
                case TOK_INT:
                case TOK_REAL:
                case TOK_SCIENCE:
                    cp(sess,sep_val(sess->cursess, tokval(sess, tk),
                                        sess->cursess->tokbuf,
                                        sess->cursess->tokbufsize,
                                        buf, MAXNUMLEN));
                    break;

                case TOK_ACC:
                    cp(sess, sess->cursess->outbuf);
                    break;

                default:
                    cp(sess, tk->str);
                    break;
            }

            //pushtok(sess, tk);
        }


        break;//SEPCP

    case TOK_NUM2TEXT:

        return num2text(sess);

        break;

        case TOK_VCLR:

            tk = poptok(sess);
            if(!tk){
                printe(MSG_MOREARGS, ent->str);
                return false_;
            } else {
                do{
                    if(tk->id == TOK_ALL){
                        clear_all_vars(sess);
                    } else if(tk->id == TOK_VAR || tk->id == TOK_ARRAY){
                        rm_var(sess, tk->str);
                    } else {
                        printe(MSG_BADARG, tk->str);
                        return false_;
                    }
                } while((tk = poptok(sess)));
            }

        break;//VCLR

//        case TOK_VCLRALL:
//            clear_all_vars(sess);
//        break;//CLRALL

        case TOK_VDESC:

        return do_vdesc(sess);

        break;//TOK_VDESC

        case TOK_VEXP:

        return vexp(sess);

            break;

        case TOK_VCAT:
            return vcat(sess);
            break;

        case TOK_VLIST:
            list_var(sess);
        break;//TOK_VLIST

        case TOK_VSTR:
            tk = poptok(sess);
            if(!tk){
                printe(MSG_MOREARGS, ent->str);
                return false_;
            }

            return vstr(sess, tk);

        break;

        case TOK_VSTRCMP:

            if(!vstrcmp(sess)) return false_;

        break;

        case TOK_VLOCK:
            tk = poptok(sess);
            if(!tk){//show locked var list
                return vlock_show(sess);
            } else {//lock the var
                do{
                    if(!vlock(sess, tk->str)) return false_;
                } while((tk = poptok(sess)));
                return true_;
            }
            break;

        case TOK_VUNLOCK:
            tk = poptok(sess);
            if(!tk){//show unlocked var list
                return vunlock_show(sess);
            } else {//unlock the var
                do{
                    if(!vunlock(sess, tk->str)) return false_;
                }while((tk = poptok(sess)));
                return true_;
            }
            break;

        case TOK_VREN:
            return vrename(sess);
            break;

        case TOK_VDUP:
        return vdup(sess);
        break;

        case TOK_ARRAYDEF:
            return create_array(sess);
            break;

        case TOK_AFILL:

            tk = poptok(sess);
            if(!tk){
                printe(MSG_MOREARGS, ent->str);
                return false_;
            } else {

                tk1 = poptok(sess);
                if(!tk1){
                    printe(MSG_MOREARGS, ent->str);
                    return false_;
                }

                return afill(sess, tk1->str, tk1->i, tk1->j, tk);
            }

            break;

        case TOK_LOAD:

        return do_load(sess);

        break;//TOK_LOAD

        case TOK_SAVE:

        return do_save(sess);

        break;//TOK_SAVE

        case TOK_FLUSHF:
            return flushf(sess);
        break;

    case TOK_VIEW:
        return do_view(sess);
        break;

        case TOK_RECALC:

            tk = poptok(sess);
            if(!tk){
                return recalc(sess, NULL);
            } else {
                do{
                    if(!recalc(sess, tk)) return false_;
                }while((tk = poptok(sess)));
            }

        break;//TOK_RECALC

//        case TOK_LOOP:

//            tk = poptok(sess);
//            if(!tk){
//                printe(MSG_MOREARGS, ent->str);
//                return false_;
//            } else {
//                if(tk->id == TOK_VAR || tk->id == VAR_ARRAY || tk->id == TOK_INT){
//                    sess->cursess->exploop = tokval(sess, tk);
//        } else if(tk->id == TOK_STOP){
//            sess->cursess->exploop = 0;
//                } else {
//            printe(MSG_BADARG, tk->str);
//                    return false_;
//                }
//            }

//        break;//TOK_LOOP

        case TOK_LABEL:
//            printw("sorry, not available now."PARAEND);
//            return false_;
            return labl(sess);
        break;

        case TOK_GOTO:
//            printw("sorry, not available now."PARAEND);
//            return false_;
            return jump(sess);
        break;

        case TOK_RETURN:
        return _ret(sess);
        break;

        case TOK_JUMP:
        return do_jump(sess);
        break;

        case TOK_AUTORUN:
        return do_autorun(sess);
        break;//TOK_AUTORUN

        case TOK_PRINT:
        return print(sess);
        break;

    case TOK_PLOT:
        return plotxy(sess);
        break;

    case TOK_AEDIT:
        return editarray(sess);
        break;

        case TOK_HISTORY:
        list_history(sess);
        break;//TOK_HISTORY

        case TOK_HLOAD:

        tk = poptok(sess);
        if(!tk){
            printe("this function needs more arguments."PARAEND);
            return false_;
        } else {
            if(tk->id == TOK_VAR || tk->id == TOK_ARRAY || tk->id == TOK_INT){
                get_history(sess, sess->cursess->expbuf, sess->cursess->expbufsize,
                    tokval(sess, tk));
                //printi("loaded \"%s\", hit [Enter] to run."PARAEND, sess->cursess->expbuf);
                printd("running history");
                run(sess);
            } else {
                printe(MSG_BADARG, tk->str);
                return false_;
            }
        }

        break;//TOK_HLOAD

        case TOK_HIJRI:
            return hijri(sess);
        break;//TOK_HIJRI:

        case TOK_GREG:
            return greg(sess);
        break;//TOK_GREG:

        case TOK_TODAY:
            return today(sess, ent);
        break;//TOK_GREG:

        case TOK_JDATE:
        return jdateconv(sess);
        break;

        case TOK_GDATE:
        return gdateconv(sess);
        break;

        case TOK_MODE:
            return mode(sess);
        break;

        case TOK_FORK:
#ifndef NOFORK
            return do_fork(sess);
#else
            printe(MSG_IMPLEMENT, ent->str);
#endif
        break;

    case TOK_ENDFORK:
#ifndef NOFORK
        return do_endfork(sess);
#else
            printe(MSG_IMPLEMENT, ent->str);
#endif
        break;

    case TOK_SWITCH:
#ifndef NOFORK
        return do_switch(sess);
#else
            printe(MSG_IMPLEMENT, ent->str);
#endif
        break;

    case TOK_BREAK:
#ifndef NOFORK
        return do_break(sess);
#else
            printe(MSG_IMPLEMENT, ent->str);
#endif
        break;

    case TOK_SLEEP:
        return do_sleep(sess);
        break;

    case TOK_WAIT:
        return do_wait(sess);
                break;

    case TOK_EXEC:
        return do_exec(sess);
        break;

        case TOK_IF:

        return _if(sess);

        break;//TOK_IF

        case TOK_ELSE:

        return _else(sess);

        break;//TOK_ELSE

        case TOK_ELSEIF:

        return _else_if(sess);

        break;//TOK_ELSEIF

        case TOK_FOR:

        return _for(sess);

        break;//TOK_FOR

        default:
            printe(MSG_IMPLEMENT, ent->str);
        break;
    }

    return true_;
}
