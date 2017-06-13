#include "double.h"

#include <sys/time.h>

enum sysui_target_id{
    UIT_HELP,
    UIT_INFO,
    UIT_NUMSEP,
    UIT_HILIGHT,
    UIT_DECIM,
    UIT_VERBOSE,//loud
    UIT_ANSWER,
    UIT_DETAILS,
    UIT_TEXTLOAD,
    UIT_64738,
    UIT_NERD,
    UIT_EXP,
    UIT_TOK,
    UIT_OUTPUT,
    UIT_HIST,
    UIT_CTL,
    UIT_LOCALE,
	UIT_CAPS,
    UIT_INVAL
};

#define SYSUI_64738_ITEMS 13
struct sysui_64738{
    char* msg;
} const ui_64738[] = {
{INFO_ICON"\n"
    " .oO    BOOM!   BOOM!    BOOM!   Oo.\n"
    " *** B.A.K S.W.E ROCKS! FOREVER! ***\n"
    " ***       A.V.P SUCKS! FOREVER! *** ;)\n"},
{WARN_ICON"\n DOOM!\n"
 " HELL ON EARTH!\n"},
{WARN_ICON"\n C=64 user? need reset?\n"},
{HELPBLOCK_ICON"\n WOOOO! WHO WANTS SOME WANG?\n"},
{ERR_ICON"\n LO WANG COMES FOR YOU LITTLE SNAKE COWAD!\n"},
{"\n ;>\n"},
{HELPBLOCK_ICON"\n The Green Release Organization (GRO). [was 90's?]\n"},
{ERR_ICON"\n I LIKE SHURIKEN!\n"},
{"\n still trying?\n"},
{"\n not tired yet?\n"},
{"\n salad is good!\n"},
{"\n Computer Magazine, Black Media (C)\n"},
{"\n Turbo C + initgraph()... That was fun! farsi.bas was fun too!\n"},
{NULL}
};

struct sysui_target{
    char* name[20];
    enum sysui_target_id id;
} ui_target[] = {
    {{"help", NULL}, UIT_HELP},
    {{"info", NULL}, UIT_INFO},
    {{"numsep", "groupnum", NULL}, UIT_NUMSEP},
    {{"hilight", NULL}, UIT_HILIGHT},
    {{"decim", "decimal", NULL}, UIT_DECIM},    
    {{"verbose", "v", NULL}, UIT_VERBOSE},
    {{"answer", "ans", NULL}, UIT_ANSWER},
    {{"details", "det", NULL}, UIT_DETAILS},
    {{"textload", NULL}, UIT_TEXTLOAD},
    {{"64738", NULL}, UIT_64738},
    {{"nerd", NULL}, UIT_NERD},
    {{"exp", NULL}, UIT_EXP},
    {{"tok", NULL}, UIT_TOK},
    {{"out", NULL}, UIT_OUTPUT},
    {{"hist", NULL}, UIT_HIST},
    {{"ctl", NULL}, UIT_CTL},
    {{"locale", NULL}, UIT_LOCALE},
	{{"caps", NULL}, UIT_CAPS},
    {{NULL}, UIT_INVAL}
};

enum sysui_val_id{
    UIV_ON,
    UIV_OFF,
    UIV_TRIM,
    UIV_FIX,
    UIV_STAT,
    UIV_BULK,
    UIV_LINE,
    UIV_SHOW,
    UIV_HIDE,
    UIV_ADD,
    UIV_SUB,
    UIV_FA,
    UIV_EN,
    UIV_INVAL
};

struct sysui_val{
    char* name[10];
    enum sysui_val_id id;
} ui_val[] = {
    {{"on", "1", NULL}, UIV_ON},
    {{"off", "0", NULL}, UIV_OFF},
    {{"trim", "tr", NULL}, UIV_TRIM},
    {{"fix", "fixed", NULL}, UIV_FIX},
    {{"stat", "info", NULL}, UIV_STAT},
    {{"bulk", NULL}, UIV_BULK},
    {{"line", NULL}, UIV_LINE},
    {{"show", NULL}, UIV_SHOW},
    {{"hide", NULL}, UIV_HIDE},
    {{"+", NULL}, UIV_ADD},
    {{"-", NULL}, UIV_SUB},
    {{"fa", NULL}, UIV_FA},
    {{"en", NULL}, UIV_EN},
    {{NULL}, UIV_INVAL}
};

enum sysui_target_id get_sysui_target(char* cmd){
    char* eoc;
    int i;
    struct sysui_target *t;
    eoc = strchr(cmd, ':');
    if(!eoc){
        if(!strcmp(cmd, "64738")) return UIT_64738;
        return UIT_INVAL;
    }

    *eoc = 0;

    for(t = ui_target; t->id != UIT_INVAL; t++){
		if(t->name){
			i = 0;
			while(t->name[i]){
	            if(!strcmp(t->name[i], cmd)){
	                *eoc = ':';
	                return t->id;
	            }
	            i++;
			}
		}
    }

    return UIT_INVAL;

}

enum sysui_val_id get_sysui_val(char* cmd){
    char* eoc;
    int i;
    struct sysui_val *t;
    eoc = strchr(cmd, ':');
    if(!eoc) return UIV_INVAL;

    eoc++;       

    while(*eoc == ' ' || *eoc == '\t') eoc++;         

    for(t = ui_val; t->id != UIV_INVAL; t++){    
		if(t->name){
			i = 0;
			while(t->name[i]){
                if(!strcmp(t->name[i], eoc)){
//	                *eoc = ':';
	                return t->id;
	            }
	            i++;
			}
		}
    }

    return UIV_INVAL;
}

int get_sysui_numval(char* cmd, int* val){
    char* eoc;
    eoc = strchr(cmd, ':');
    if(!eoc) return false_;

    eoc++;

    while(*eoc == ' ' || *eoc == '\t') eoc++;

    if(!isint(eoc)) return false_;

    *val = atoi(eoc);

    return true_;

}

int _sysui_handle_flag(struct session* sess, char* cmd, unsigned short flag){

    switch(get_sysui_val(cmd)){
    case UIV_ON:
        sess->cursess->uiflags &= ~flag;
        break;
    case UIV_OFF:
        sess->cursess->uiflags |= flag;
        break;
    case UIV_STAT:
        if(sess->cursess->uiflags & flag){
            printi("off"PARAEND);
        } else {
            printi("on"PARAEND);
        }
        break;
    default:
        printe(MSG_BADARG, cmd);
        return false_;
        break;
    }

    return true_;
}

int _sysui_numsep(struct session* sess, char* cmd){

    switch(get_sysui_val(cmd)){
    case UIV_ON:
        sess->cursess->uiflags &= ~SESS_UIF_NOSEP;
        break;
    case UIV_OFF:
        sess->cursess->uiflags |= SESS_UIF_NOSEP;
        break;
    case UIV_STAT:
        if(sess->cursess->uiflags & SESS_UIF_NOSEP){
            printi("off"PARAEND);
        } else {
            printi("on"PARAEND);
        }
        break;
    default:
        printe(MSG_BADARG, cmd);
        return false_;
        break;
    }

    return true_;
}

int _sysui_hilight(struct session* sess, char* cmd){

    switch(get_sysui_val(cmd)){
    case UIV_OFF:
        sess->cursess->uiflags &= ~SESS_UIF_HILIGHT;
        break;
    case UIV_ON:
        sess->cursess->uiflags |= SESS_UIF_HILIGHT;
        break;
    case UIV_STAT:
        if(sess->cursess->uiflags & SESS_UIF_HILIGHT){
            printi("on"PARAEND);
        } else {
            printi("off"PARAEND);
        }
        break;
    default:
        printe(MSG_BADARG, cmd);
        return false_;
        break;
    }

    return true_;
}


int _sysui_decim(struct session* sess, char* cmd){
    int i;
    switch(get_sysui_val(cmd)){
    case UIV_TRIM:
        sess->cursess->uiflags &= ~SESS_UIF_DECIMNOTR;
        break;
    case UIV_FIX:
        sess->cursess->uiflags |= SESS_UIF_DECIMNOTR;
        break;
    case UIV_STAT:
        if(sess->cursess->uiflags & SESS_UIF_DECIMNOTR){
            printi("fix@%i"PARAEND, sess->cursess->decim);
        } else {
            printi("trim@%i"PARAEND, sess->cursess->decim);
        }
        break;
    case UIV_ADD:
        sess->cursess->decim++;
        break;
    case UIV_SUB:
        sess->cursess->decim--;
        break;
    case UIV_INVAL:
        if(!get_sysui_numval(cmd, &i)) return false_;
        else sess->cursess->decim = i;
        break;
    default:
        printe(MSG_BADARG, cmd);
        return false_;
        break;
    }

    if(sess->cursess->decim > SESS_MAX_DECIM) sess->cursess->decim = SESS_MAX_DECIM;
    if(sess->cursess->decim < SESS_MIN_DECIM) sess->cursess->decim = SESS_MIN_DECIM;

    return true_;
}

int _sysui_verbose(struct session* sess, char* cmd){

    switch(get_sysui_val(cmd)){
    case UIV_OFF:
        sess->cursess->uiflags &= ~SESS_UIF_LOUD;
        break;
    case UIV_ON:
        sess->cursess->uiflags |= SESS_UIF_LOUD;
        break;
    case UIV_STAT:
        if(sess->cursess->uiflags & SESS_UIF_LOUD){
            printi("on"PARAEND);
        } else {
            printi("off"PARAEND);
        }
        break;
    default:
        printe(MSG_BADARG, cmd);
        return false_;
        break;
    }

    return true_;
}

int _sysui_answer(struct session* sess, char* cmd){

    switch(get_sysui_val(cmd)){
    case UIV_OFF:
    case UIV_HIDE:
        sess->cursess->uiflags &= ~SESS_UIF_ANS;
        break;
    case UIV_ON:
    case UIV_SHOW:
        sess->cursess->uiflags |= SESS_UIF_ANS;
        break;
    case UIV_STAT:
        if(sess->cursess->uiflags & SESS_UIF_ANS){
            printi("on"PARAEND);
        } else {
            printi("off"PARAEND);
        }
        break;
    default:
        printe(MSG_BADARG, cmd);
        return false_;
        break;
    }

    return true_;
}

int _sysui_details(struct session* sess, char* cmd){

    switch(get_sysui_val(cmd)){
    case UIV_OFF:
    case UIV_HIDE:
        sess->cursess->uiflags &= ~SESS_UIF_DET;
        break;
    case UIV_ON:
    case UIV_SHOW:
        sess->cursess->uiflags |= SESS_UIF_DET;
        break;
    case UIV_STAT:
        if(sess->cursess->uiflags & SESS_UIF_DET){
            printi("on"PARAEND);
        } else {
            printi("off"PARAEND);
        }
        break;
    default:
        printe(MSG_BADARG, cmd);
        return false_;
        break;
    }

    return true_;
}

int _sysui_nerd(struct session* sess, char* cmd){

    switch(get_sysui_val(cmd)){
    case UIV_OFF:
        sess->cursess->uiflags &= ~SESS_UIF_NERD;
        break;
    case UIV_ON:
        sess->cursess->uiflags |= SESS_UIF_NERD;
        break;
    case UIV_STAT:
        if(sess->cursess->uiflags & SESS_UIF_NERD){
            printi("on"PARAEND);
        } else {
            printi("off"PARAEND);
        }
        break;
    default:
        printe(MSG_BADARG, cmd);
        return false_;
        break;
    }

    return true_;
}

int _sysui_locale(struct session* sess, char* cmd){

    switch(get_sysui_val(cmd)){
    case UIV_FA:
        sess->cursess->uilocale = SESS_LOCALE_FA;
        break;
    case UIV_EN:
        sess->cursess->uilocale = SESS_LOCALE_EN;
        break;
    case UIV_STAT:
        if(sess->cursess->uilocale == SESS_LOCALE_FA){
            printi("Farsi (fa)"PARAEND);
        } else if(sess->cursess->uilocale == SESS_LOCALE_EN){
            printi("English (en)"PARAEND);
        }
        break;
    default:
        printe(MSG_BADARG, cmd);
        return false_;
        break;
    }

    return true_;
}

int _sysui_64738(struct session* sess){    

    unsigned int seed;

    struct timeval tv;

    gettimeofday(&tv, NULL);

    seed = tv.tv_usec + tv.tv_sec;

    srand(seed);

    int i = rand();

    i = i % SYSUI_64738_ITEMS;    

    printo(ui_64738[i].msg);

    return true_;

}

int sysui(struct session* sess, char* cmd){

    switch(get_sysui_target(cmd)){

    case UIT_NUMSEP:
        return _sysui_numsep(sess, cmd);
        break;

    case UIT_HILIGHT:
        return _sysui_hilight(sess, cmd);
        break;

    case UIT_DECIM:
        return _sysui_decim(sess, cmd);
        break;

    case UIT_VERBOSE:
        return _sysui_verbose(sess, cmd);
        break;

    case UIT_ANSWER:
        return _sysui_answer(sess, cmd);
        break;

    case UIT_DETAILS:
        return _sysui_details(sess, cmd);
        break;

    case UIT_64738:
        return _sysui_64738(sess);
        break;

    case UIT_NERD:
        return _sysui_nerd(sess, cmd);
        break;

    case UIT_LOCALE:
        return _sysui_locale(sess, cmd);
        break;

    default:
        printe(MSG_BADARG, cmd);
        return false_;
        break;
    }

    return true_;

}

int _sysfile_textload(struct session* sess, char* cmd){

    switch(get_sysui_val(cmd)){
    case UIV_BULK:
        sess->cursess->fileflags &= ~SESS_FILEF_LINE;
        break;
    case UIV_LINE:
        sess->cursess->fileflags |= SESS_FILEF_LINE;
        break;
    case UIV_STAT:
        if(sess->cursess->fileflags & SESS_FILEF_LINE){
            printi("line"PARAEND);
        } else {
            printi("bulk"PARAEND);
        }
        break;
    default:
        printe(MSG_BADARG, cmd);
        return false_;
        break;
    }

    return true_;
}

int sysfile(struct session* sess, char* cmd){

    switch(get_sysui_target(cmd)){

    case UIT_TEXTLOAD:
        return _sysfile_textload(sess, cmd);
        break;

    default:
        printe(MSG_BADARG, cmd);
        return false_;
        break;
    }

    return true_;

}

int _sysbuf_exp(struct session* sess, char* cmd){
    int i;

    switch(get_sysui_val(cmd)){
    case UIV_STAT:
        printi("size: %i"PARAEND, sess->cursess->expbufsize);
        break;
    case UIV_ADD:
        sess->cursess->expbufsize = sess->cursess->expbufsize*2;
        break;
    case UIV_SUB:
        sess->cursess->expbufsize = sess->cursess->expbufsize/2;
        break;
    case UIV_INVAL:
        if(!get_sysui_numval(cmd, &i)) return false_;
        sess->cursess->expbufsize = i;
        break;
    default:
        printe(MSG_BADARG, cmd);
        return false_;
        break;
    }

    if(sess->cursess->expbufsize > DEF_MAX_EXPBUF) sess->cursess->expbufsize = DEF_MAX_EXPBUF;
    if(sess->cursess->expbufsize < DEF_MIN_EXPBUF) sess->cursess->expbufsize = DEF_MIN_EXPBUF;

    return realloc_buf(sess);

}

int _sysbuf_tok(struct session* sess, char* cmd){
    int i;

    switch(get_sysui_val(cmd)){
    case UIV_STAT:
        printi("size: %i"PARAEND, sess->cursess->tokbufsize);
        break;
    case UIV_ADD:
        sess->cursess->tokbufsize = sess->cursess->tokbufsize*2;
        break;
    case UIV_SUB:
        sess->cursess->tokbufsize = sess->cursess->tokbufsize/2;
        break;
    case UIV_INVAL:
        if(!get_sysui_numval(cmd, &i)) return false_;
        sess->cursess->tokbufsize = i;
        break;
    default:
        printe(MSG_BADARG, cmd);
        return false_;
        break;
    }

    if(sess->cursess->tokbufsize > DEF_MAX_TOKBUF) sess->cursess->tokbufsize = DEF_MAX_TOKBUF;
    if(sess->cursess->tokbufsize < DEF_MIN_TOKBUF) sess->cursess->tokbufsize = DEF_MIN_TOKBUF;

    return realloc_buf(sess);

}

int _sysbuf_out(struct session* sess, char* cmd){
    int i;

    switch(get_sysui_val(cmd)){
    case UIV_STAT:
        printi("size: %i"PARAEND, sess->cursess->outbufsize);
        break;
    case UIV_ADD:
        sess->cursess->outbufsize = sess->cursess->outbufsize*2;
        break;
    case UIV_SUB:
        sess->cursess->outbufsize = sess->cursess->outbufsize/2;
        break;        
    case UIV_INVAL:
        if(!get_sysui_numval(cmd, &i)) return false_;
        sess->cursess->outbufsize = i;
        break;
    default:
        printe(MSG_BADARG, cmd);
        return false_;
        break;
    }

    if(sess->cursess->outbufsize > DEF_MAX_OUTBUF) sess->cursess->outbufsize = DEF_MAX_OUTBUF;
    if(sess->cursess->outbufsize < DEF_MIN_OUTBUF) sess->cursess->outbufsize = DEF_MIN_OUTBUF;

    return realloc_buf(sess);

}

int _sysbuf_hist(struct session* sess, char* cmd){
    int i = sess->cursess->maxhist;

    switch(get_sysui_val(cmd)){
    case UIV_STAT:
        printi("size: %i"PARAEND, sess->cursess->maxhist);
        break;
    case UIV_ADD:
        i++;
        break;
    case UIV_SUB:
        i--;
        break;
    case UIV_INVAL:
        if(!get_sysui_numval(cmd, &i)) return false_;
        break;
    default:
        printe(MSG_BADARG, cmd);
        return false_;
        break;
    }

    if(i > DEF_MAX_HIST) i = DEF_MAX_HIST;
    if(i < DEF_MIN_HIST) i = DEF_MIN_HIST;

    return resize_hist(sess, i);

}

int _sysbuf_ctl(struct session* sess, char* cmd){

    switch(get_sysui_val(cmd)){
    case UIV_OFF:
        sess->cursess->uiflags &= ~SESS_UIF_CTLBUF;
        break;
    case UIV_ON:
        sess->cursess->uiflags |= SESS_UIF_CTLBUF;
        break;
    case UIV_STAT:
        if(sess->cursess->uiflags & SESS_UIF_CTLBUF){
            printi("on"PARAEND);
        } else {
            printi("off"PARAEND);
        }
        break;
    default:
        printe(MSG_BADARG, cmd);
        return false_;
        break;
    }

    return true_;
}


int sysbuf(struct session* sess, char* cmd){

    switch(get_sysui_target(cmd)){

    case UIT_EXP:
        return _sysbuf_exp(sess, cmd);
        break;

    case UIT_TOK:
        return _sysbuf_tok(sess, cmd);
        break;

    case UIT_OUTPUT:
        return _sysbuf_out(sess, cmd);
        break;

    case UIT_HIST:
        return _sysbuf_hist(sess, cmd);
        break;
    case UIT_CTL:
        return _sysbuf_ctl(sess, cmd);
        break;

    default:
        printe(MSG_BADARG, cmd);
        return false_;
        break;
    }

    return true_;

}

int _syscore_caps(struct session* sess, char* cmd){

    switch(get_sysui_val(cmd)){
    case UIV_SHOW:
		printi("Engine Caps: %s"PARAEND, ENGINECAPS_LONG);
		printi("UI Caps: %s"PARAEND, UICAPS_LONG);
        break;
    default:
        printe(MSG_BADARG, cmd);
        return false_;
        break;
    }

    return true_;
}

int syscore(struct session* sess, char* cmd){

    switch(get_sysui_target(cmd)){

    case UIT_CAPS:
        return _syscore_caps(sess, cmd);
        break;

    default:
        printe(MSG_BADARG, cmd);
        return false_;
        break;
    }

    return true_;

}


int sys(struct session* sess){
    struct tok_s* tk, *first = NULL, *last=NULL;
//    size_t oldsize;

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
        printi("expbuf: %u, tokbuf: %u, outbuf: %u, maxhist: %u, decim: %u"PARAEND,
                sess->cursess->expbufsize, sess->cursess->tokbufsize, sess->cursess->outbufsize,
                sess->cursess->maxhist, sess->cursess->decim);
        return true_;
    }

    tk = first->next;
    if(!tk){
        switch(first->id){

        case TOK_INT:
            sess->cursess->calc--;
        case TOK_STR:
            if(!strcmp(first->str, "64738")) return _sysui_64738(sess);
        break;

        default:
            printe(MSG_MOREARGS, "sys");
            break;
        }
        return false_;
    }

    switch(first->id){            

//        case TOK_EXPBUF:

//            oldsize = sess->cursess->expbufsize;
//            sess->cursess->expbufsize = intval(tokval(sess, tk));
//            if(!sess->cursess->expbufsize){
//                printe(MSG_FORBID, tk->str);
//                sess->cursess->expbufsize = oldsize;
//            } else {
//                return realloc_buf(sess);
//            }

//        break;

//        case TOK_TOKBUF:

//            oldsize = sess->cursess->tokbufsize;
//            sess->cursess->tokbufsize = intval(tokval(sess, tk));
//            if(!sess->cursess->tokbufsize){
//                printe(MSG_FORBID, tk->str);
//                sess->cursess->tokbufsize = oldsize;
//            } else {
//                return realloc_buf(sess);
//            }

//        break;

//        case TOK_OUTBUF:

//            oldsize = sess->cursess->outbufsize;
//            sess->cursess->outbufsize = intval(tokval(sess, tk));
//            if(!sess->cursess->outbufsize){
//                printe(MSG_FORBID, tk->str);
//                sess->cursess->outbufsize = oldsize;
//            } else {
//                return realloc_buf(sess);
//            }

//        break;

//        case TOK_MAXHIST:
//            if(!intval(tokval(sess, tk))){
//                printe(MSG_RANGE, first->str);
//                break;
//            }
//            return resize_hist(sess, intval(tokval(sess, tk)));
//        break;

		case TOK_CORE:
			return syscore(sess, tk->str);
		break;

        case TOK_BUF:
            return sysbuf(sess, tk->str);
        break;

        case TOK_UI:
            return sysui(sess, tk->str);
        break;

        case TOK_FILE:
            return sysfile(sess, tk->str);
        break;

        default:
            printe(MSG_BADARG, first->str);
        break;
    }

    return false_;

}

int get_engine_ver(struct session* sess){
    struct tok_s* tk;
    struct variable* var;
    tk = poptok(sess);
    if(!tk){
//        printi("%u"PARAEND, ENGINEVER);
//        return true_;
        return set_ansvar(sess, (fnum_t)(ENGINEVER));
    } else {
        if(!tk->str || !strlen(tk->str)){
            printe(MSG_BADARG, "");
            return false_;
        }
        var = var_ptr(sess, tk->str, NULL);
        if(!var){
            printe(MSG_NOTFOUND, tk->str);
            return false_;
        }
        return set_varval(sess, tk->str, (fnum_t)(ENGINEVER));
    }
    return false_;
}


int savestat(struct session *sess){
    struct variable* var;
    char* filename;
    FILE* f;
    int maxi, maxj;
    struct tok_s* tk = poptok(sess);
    char buf[MAXNUMLEN];

    if(!tk){
        filename = DEF_STAT_FILENAME;
    } else {
        filename = tk->str;
    }

    f = fopen(filename, "w+");
    if(!f){
        printe(MSG_NOOPEN, filename);
        return false_;
    }

    char* array = NULL;

    for(var = sess->cursess->vfirst; var; var = var->next){

        if(var->type == VAR_SYS || var->type == VAR_CTL) continue;

        if(var->type == VAR_ARRAY){
            if(!array){
                goto define_array;
            } else {
                if(!strcmp(array, var->name)){//this is the old array
add_aval:
                    fprintf(f, "%s[%i][%i]=%s;\n", var->name, var->i, var->j, clean_float(sess, var->val, buf, MAXNUMLEN));
                    if(var->desc && strlen(var->desc)) fprintf(f, "vdesc(%s[%i][%i], \"%s\");\n", var->name, var->i, var->j, var->desc);
                    if(var->exp && strlen(var->exp)) fprintf(f, "strcpy(%s[%i][%i], \"%s\");\n", var->name, var->i, var->j, var->exp);
                    if(var->flags & VARF_LOCK) fprintf(f, "vlock(%s[%i][%i]);\n", var->name, var->i, var->j);
                } else {//array has been changed, define it!
define_array:
                    array = var->name;
                    array_limits(sess, var->name, &maxi, &maxj);
                    if(maxj < 0){
                        fprintf(f, "%s[%i]=0;\n", var->name, maxi+1);
                    } else {
                        fprintf(f, "%s[%i][%i]=0;\n", var->name, maxi+1, maxj+1);
                    }
                    goto add_aval;
                }
            }
        } else {
            array = NULL;
            fprintf(f, "%s=%s;\n", var->name, clean_float(sess, var->val, buf, MAXNUMLEN));
            if(var->desc && strlen(var->desc)) fprintf(f, "vdesc(%s, \"%s\");\n", var->name, var->desc);
            if(var->exp && strlen(var->exp)) fprintf(f, "strcpy(%s, \"%s\");\n", var->name, var->exp);
            if(var->flags & VARF_LOCK) fprintf(f, "vlock(%s);\n", var->name);
        }
    }

    fprintf(f, "%s;\n", clean_float(sess, var_val(sess, ANSVARNAME), buf, MAXNUMLEN));
    fclose(f);
    printi(MSG_SSTAT, filename);
    return true_;
}


int run_cmd(struct session* sess,struct tok_s* ent){
	
	struct tok_s* tk;
	
    switch(ent->id){

        case TOK_SYS:
            return sys(sess);
        break;

    case TOK_VER:
            return get_engine_ver(sess);
        break;

        case TOK_HELP:

            tk = sess->cursess->pre->exp->prefixfirst;
			
		    if(!tk){
		    	tk = poptok(sess);
		    } else {
		    	remove_prehead(sess);
		    }
		    
			if(!tk){
				show_help(sess);
			} else {
				show_tok_desc(sess, tk);
		    }
	    
        break;

        case TOK_EXIT:
            sess->cursess->flags |= SESSF_EXIT;
            closesavefile(sess);            
        break;

    case TOK_SAVESTAT:
        return savestat(sess);
        break;

        default:
			printd("id: %i\n", ent->id);
            printe(MSG_IMPLEMENT, ent->str);
        break;
    }
    
    return true_;
}
