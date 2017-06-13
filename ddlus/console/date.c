#include "double.h"

#define __true 1
#define __false 0

enum d{Y, M, D};

inum_t gdayno(int g_y, int g_m, int g_d){
    int g_days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    //int j_days_in_month[] = {31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 29};
    int i;
    inum_t gy, gm, gd, g_day_no;
    //int jy, jm, jd, j_day_no;
    //int j_np;

    gy = g_y-1600;
    gm = g_m-1;
    gd = g_d-1;

    g_day_no = 365*gy+((gy+3)/4)-((gy+99)/100)+((gy+399)/400);

    for (i=0; i < gm; ++i) g_day_no += g_days_in_month[i];

    if (gm>1 && ((gy%4==0 && gy%100!=0) || (gy%400==0))) g_day_no++; /* leap and after Feb */

    g_day_no += gd;

    return g_day_no;
}

inum_t jdayno(int j_y, int j_m, int j_d){
    //int g_days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int j_days_in_month[] = {31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 29};
    int i;
    //int gy, gm, gd, g_day_no;
    inum_t jy, jm, jd, j_day_no;
    //int leap;

    jy = j_y-979;
    jm = j_m-1;
    jd = j_d-1;

    j_day_no = 365*jy + (jy/ 33)*8 + ((jy%33+3)/ 4);
    for (i=0; i < jm; ++i) j_day_no += j_days_in_month[i];

    j_day_no += jd;

    return j_day_no;
}

int jdayno2gdate(inum_t j_day_no, int gdate[3]){

    int g_days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    //int j_days_in_month[] = {31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 29};
    int i;
    inum_t gy, gm, gd, g_day_no;
    //int jy, jm, jd, j_day_no;
    int leap;

    g_day_no = j_day_no+79;

    gy = 1600 + 400*(g_day_no/ 146097); /* 146097 = 365*400 + 400/4 - 400/100 + 400/400 */

    g_day_no = g_day_no % 146097;
    leap = __true;

    if (g_day_no >= 36525) { /* 36525 = 365*100 + 100/4 */
        g_day_no--;
        gy += 100*(g_day_no/ 36524); /* 36524 = 365*100 + 100/4 - 100/100 */
        g_day_no = g_day_no % 36524;
        if (g_day_no >= 365) g_day_no++;
        else leap = __false;
    }

    gy += 4*(g_day_no/ 1461); /* 1461 = 365*4 + 4/4 */ g_day_no %= 1461;
    if (g_day_no >= 366) {
        leap = __false;
        g_day_no--;
        gy += (g_day_no/ 365);
        g_day_no = g_day_no % 365;
    }
    for (i = 0; g_day_no >= g_days_in_month[i] + (i == 1 && leap); i++) g_day_no -= g_days_in_month[i] + (i == 1 && leap);

    gm = i+1;
    gd = g_day_no+1;

    gdate[0] = gy;
    gdate[1] = gm;
    gdate[2] = gd;

    return true_;
}

int gdayno2jdate(inum_t g_day_no, int jdate[3]){

    //int g_days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int j_days_in_month[] = {31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 29};
    int i;
    //int gy, gm, gd, g_day_no;
    inum_t jy, jm, jd, j_day_no;
    int j_np;

    j_day_no = g_day_no-79;
    j_np = (j_day_no/ 12053); /* 12053 = 365*33 + 32/4 */

    j_day_no = j_day_no % 12053;
    jy = 979+33*j_np+4*(j_day_no/1461); /* 1461 = 365*4 + 4/4 */

    j_day_no %= 1461;

    if(j_day_no >= 366) {
        jy += ((j_day_no-1)/ 365);
        j_day_no = (j_day_no-1)%365;
    }

    for(i = 0; i < 11 && j_day_no >= j_days_in_month[i]; ++i)
        j_day_no -= j_days_in_month[i];

    jm = i+1;
    jd = j_day_no+1;

    jdate[0] = jy;
    jdate[1] = jm;
    jdate[2] = jd;

    return true_;
}

int d13_g2j(int g_y, int g_m, int g_d, int jdate[3]){

    int g_days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int j_days_in_month[] = {31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 29};
    int i;
    int gy, gm, gd, g_day_no;
    int jy, jm, jd, j_day_no;
    int j_np;

    gy = g_y-1600;
    gm = g_m-1;
    gd = g_d-1;

    g_day_no = 365*gy+((gy+3)/4)-((gy+99)/100)+((gy+399)/400);

    for (i=0; i < gm; ++i) g_day_no += g_days_in_month[i];

    if (gm>1 && ((gy%4==0 && gy%100!=0) || (gy%400==0))) g_day_no++; /* leap and after Feb */

    g_day_no += gd;
    j_day_no = g_day_no-79;
    j_np = (j_day_no/ 12053); /* 12053 = 365*33 + 32/4 */

    j_day_no = j_day_no % 12053;
    jy = 979+33*j_np+4*(j_day_no/1461); /* 1461 = 365*4 + 4/4 */

    j_day_no %= 1461;

    if(j_day_no >= 366) {
        jy += ((j_day_no-1)/ 365);
        j_day_no = (j_day_no-1)%365;
    }

    for(i = 0; i < 11 && j_day_no >= j_days_in_month[i]; ++i)
        j_day_no -= j_days_in_month[i];

    jm = i+1;
    jd = j_day_no+1;

    jdate[0] = jy;
    jdate[1] = jm;
    jdate[2] = jd;

    return true_;
}

int jdayno2jdate(inum_t j_day_no, int jdate[3]){
    /*
     * jd1->j1, j1->gd1, gd1->jd1;
    */

    int gdate[3];
    jdayno2gdate(j_day_no, gdate);
    d13_g2j(gdate[0], gdate[1], gdate[2], jdate);
    return true_;
}


int d13_j2g(int j_y, int j_m, int j_d, int gdate[3]) {

    int g_days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int j_days_in_month[] = {31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 29};
    int i;
    int gy, gm, gd, g_day_no;
    int jy, jm, jd, j_day_no;
    int leap;

    jy = j_y-979;
    jm = j_m-1;
    jd = j_d-1;

    j_day_no = 365*jy + (jy/ 33)*8 + ((jy%33+3)/ 4);
    for (i=0; i < jm; ++i) j_day_no += j_days_in_month[i];

    j_day_no += jd;
    g_day_no = j_day_no+79;

    gy = 1600 + 400*(g_day_no/ 146097); /* 146097 = 365*400 + 400/4 - 400/100 + 400/400 */

    g_day_no = g_day_no % 146097;
    leap = __true;

    if (g_day_no >= 36525) { /* 36525 = 365*100 + 100/4 */
        g_day_no--;
        gy += 100*(g_day_no/ 36524); /* 36524 = 365*100 + 100/4 - 100/100 */
        g_day_no = g_day_no % 36524;
        if (g_day_no >= 365) g_day_no++;
        else leap = __false;
    }

    gy += 4*(g_day_no/ 1461); /* 1461 = 365*4 + 4/4 */ g_day_no %= 1461;
    if (g_day_no >= 366) {
        leap = __false;
        g_day_no--;
        gy += (g_day_no/ 365);
        g_day_no = g_day_no % 365;
    }
    for (i = 0; g_day_no >= g_days_in_month[i] + (i == 1 && leap); i++) g_day_no -= g_days_in_month[i] + (i == 1 && leap);

    gm = i+1;
    gd = g_day_no+1;

    gdate[0] = gy;
    gdate[1] = gm;
    gdate[2] = gd;

    return true_;

}

static inline int _d13_get_proper_sys_time(time_t* t, int it13[6]){
    struct tm* tp;
    char tmp[10];
    tp = localtime(t);
    if(!tp) return false_;

    strftime(tmp, 10, "%Y", tp);
    it13[0] = atoi(tmp);
    strftime(tmp, 10, "%m", tp);
    it13[1] = atoi(tmp);
    strftime(tmp, 10, "%d", tp);
    it13[2] = atoi(tmp);

    it13[3] = tp->tm_hour;
    it13[4] = tp->tm_min;
    it13[5] = tp->tm_sec;

    return true_;
}

int d13_today(int it13[6]){

    time_t t;

    if(time(&t) == -1){
        return false_;
    }

    _d13_get_proper_sys_time(&t, it13);

    d13_g2j(it13[0], it13[1], it13[2], it13);

    return true_;
}

struct tok_s* convert_date(struct session* sess, struct tok_s* tk, inum_t(ndays)(int yy, int mm, int dd)){

    int date[3];//, jdate[3];
    int bigdate;
    char datestr[11];
    int i;    
    char* brk, *start;    
    //char ch;

    if(tk->id == TOK_STR){
        snprintf(datestr, 11, "%s", tk->str);
        start = datestr;
        for(i = 0; i < 3; i++){
            brk = strpbrk(start, "./- ");            
            if(brk) *brk = (char)0;
            date[i] = atoi(start);
            if(!brk){
                if(i < 2){
                    printe(MSG_FORMAT, tk->str);
                    return NULL;
                }
            } else {
                start = brk+1;
                //if(!i) ch = *brk;
            }
        }
    } else {
        //decrease calc value by one!
        sess->cursess->calc--;
        bigdate = intval(tokval(sess, tk));
        date[Y] = bigdate / 10000;
        date[M] = (bigdate - date[Y] * 10000)/100;
        date[D] = (bigdate - date[Y] * 10000 - date[M] * 100);
    }

    //printi("%i,%i,%i", date[Y], date[M], date[D]);

    if(date[Y] < 100 || date[M] > 12 || date[M] < 1 || date[D] > 31 || date[D] < 1){
        printe(MSG_FORMAT, tk->str);
        return NULL;
    }

    //fn(date[Y], date[M], date[D], jdate);

    //printi(datestr);

    //tk->val = jdate[Y]*10000+jdate[M]*100+jdate[D];

//    tk->val = ndays(date[Y], date[M], date[D]);
    settokval(tk, ndays(date[Y], date[M], date[D]));

//    if(tk->id == TOK_STR){
//        snprintf(datestr, 11, "%i%c%i%c%i", jdate[Y], ch, jdate[M], ch, jdate[D]);
//        strcpy(tk->str, datestr);
//    }

    return tk;
}

int hijri(struct session* sess){
    struct tok_s* tk;

    tk = poptok(sess);
    if(!tk){
        printe(MSG_MOREARGS, "shamsi");
        return false_;
    }

    tk = convert_date(sess, tk, jdayno);
    if(!tk) return false_;

   // tk->val -= 79;

//    tk->id = TOK_DATE;
//    tk->type = TOKT_NUMBER;
    morph_tok(sess, tk, TOK_DATE);

    push_calc(sess, tk);
    return true_;
}

int greg(struct session* sess){
    struct tok_s* tk;

    tk = poptok(sess);
    if(!tk){
        printe(MSG_MOREARGS, "greg");
        return false_;
    }

    tk = convert_date(sess, tk, gdayno);
    if(!tk) return false_;

    //convert g_day_no to j_day_no;
//    tk->val -= 79;
    settokval(tk, tokval(sess, tk)-79.0);

//    tk->id = TOK_DATE;
//    tk->type = TOKT_NUMBER;
    morph_tok(sess, tk, TOK_DATE);

    push_calc(sess, tk);

    return true_;

}

int today(struct session* sess, struct tok_s* tk){

    int it13[6];

    d13_today(it13);

//    tk->id = TOK_DATE;
//    tk->type = TOKT_NUMBER;
    morph_tok(sess, tk, TOK_DATE);

//    tk->val = jdayno(it13[0], it13[1], it13[2]);
    settokval(tk, intval(jdayno(it13[0], it13[1], it13[2])));

    push_calc(sess, tk);

    return true_;

}

inum_t jdate(struct session* sess, struct tok_s* tk){

    tk = convert_date(sess, tk, jdayno);
    if(!tk) return 0;
    
    //tk->id = TOK_INT;
    //tk->type = TOKT_NUMBER;
    tk->flags |= TOKF_RES;

    return intval(tokval(sess, tk));
}

inum_t gdate(struct session* sess, struct tok_s* tk){

    tk = convert_date(sess, tk, gdayno);
    if(!tk) return 0;

//    //convert g_day_no to j_day_no;
//    tk->val -= 79;

    //tk->id = TOK_INT;
    //tk->type = TOKT_NUMBER;
    tk->flags |= TOKF_RES;

    return intval(tokval(sess, tk));
}

int jdateconv(struct session* sess){
    struct tok_s* tk;

    tk = poptok(sess);
    if(!tk){
        printe(MSG_MOREARGS, "jdate");
        return false_;
    }

//    if(tk->type != TOKT_NUMBER){
//        printe(MSG_BADARG, tk->str);
//        return false_;
//    }

    //tk = convert_date(sess, tk, gdayno);
    //if(!tk) return false_;

    //convert g_day_no to j_day_no;
    //tk->val -= 79;

//    tk->id = TOK_DATE;
    //tk->type = TOKT_NUMBER;
    morph_tok(sess, tk, TOK_DATE);

    sess->cursess->calc--;

    push_calc(sess, tk);

    return true_;
}

int gdateconv(struct session* sess){
    struct tok_s* tk;

    tk = poptok(sess);
    if(!tk){
        printe(MSG_MOREARGS, "gdate");
        return false_;
    }

//    if(tk->type != TOKT_NUMBER || tk->type != TOKT_){
//        printe(MSG_BADARG, tk->str);
//        return false_;
//    }

    //tk = convert_date(sess, tk, gdayno);
    //if(!tk) return false_;
    morph_tok(sess, tk, TOK_DATE);

    //convert g_day_no to j_day_no;
//    tk->val -= 79;
    settokval(tk, tokval(sess, tk)-79.0);

//    tk->id = TOK_DATE;
    //tk->type = TOKT_NUMBER;

    sess->cursess->calc--;

    push_calc(sess, tk);

    return true_;
}

