//#include "include/error13.h"
//#include "include/str13.h"
#include <stdio.h>
#include <string.h>

#include "double.h"

#define true_   1
#define false_  0

char errmsg[200];
#define snprinte(fmt, args...) snprintf(errmsg, 200, fmt, ## args)
//#define printe() perror(errmsg)

#define snprinte1 snprinte
#define printe1 perror(errmsg)

//#define MAXNUMLEN	20//for 999'999'999'999'999'999
#define FMT "%%.%uLf"
#define NUMGROUPLEN	4//x\0xx\0

#define WORD_AND "و"
#define WORD_RIALS "ريال"

#define FLAG_NEG	(0x01<<0)

enum group_pos_val{
    GPOS_ONES = 0,
    GPOS_HUNDREDS,
    GPOS_THOUSANDS,
    GPOS_MILLIONS,
    GPOS_BILLIONS,
    GPOS_TRILLIONS,
    GPOS_INVAL
};

static struct group_pos_text_s{
		char* text;
} group_pos_text[] = {
	{""},
	{"صد"},
	{"هزار"},
	{"میلیون"},
	{"میلیارد"},
	{"تریلیون"},
	{NULL},
};

static struct decimal_group_text_s{
	int val;
	char* text;
} decimal_group_text[] = {
	{0, "صفر"},
	{1, "یک"},
	{2, "دو"},
	{3, "سه"},
	{4, "چهار"},
	{5, "پنج"},
	{6, "شش"},
	{7, "هفت"},
	{8, "هشت"},
	{9, "نه"},
	{10, "ده"},
	{11, "یازده"},
	{12, "دوازده"},
	{13, "سیزده"},
	{14, "چهارده"},
	{15, "پانزده"},
	{16, "شانزده"},
	{17, "هفده"},
	{18, "هجده"},
	{19, "نوزده"},
	{20, "بیست"},
	{30, "سی"},
	{40, "چهل"},
	{50, "پنجاه"},
	{60, "شصت"},
	{70, "هفتاد"},
	{80, "هشتاد"},
	{90, "نود"},
	{-1, NULL},
};

//SPECIALS ONLY
static struct hundreds_group_text_s{
	int val;
	char* text;
} hundreds_group_text[] = {
	{2, "دویست"},
	{3, "سیصد"},
	{5, "پانصد"},
	{-1, NULL}
};

static struct sign_text_s{
	char sign;
	char* text;
} sign_text[] = {
	{'-', "منفی"},
	{'.', "ممیز"},
	{0, NULL}
};

static inline int nt_group_pos(enum group_pos_val val, char* buf, size_t bufsize, size_t* pos){
    snprintf(buf + (*pos), bufsize - (*pos), "%s", group_pos_text[val].text);
    *pos += strlen(group_pos_text[val].text);
    return true_;
}

static inline int nt_decimal_group(int val, char* buf, size_t bufsize, size_t* pos){

    if(val <= 0 || val > 99) return false_;

	if(val <= 20){
        snprintf(buf + (*pos), bufsize - (*pos), "%s", decimal_group_text[val].text);
        *pos += strlen(decimal_group_text[val].text);
        return true_;
	}
	
    if(!(val%10)){
        snprintf(buf + (*pos), bufsize - (*pos), "%s", decimal_group_text[20+(val/10)-2].text);
        *pos += strlen(decimal_group_text[20+(val/10)-2].text);
	} else {
		nt_decimal_group((val/10)*10, buf, bufsize, pos);

		snprintf(buf + (*pos), bufsize - (*pos), " %s ", WORD_AND);
		*pos += strlen(WORD_AND) + 2;

		nt_decimal_group((val%10), buf, bufsize, pos);
		
	}
	
    return true_;
}

static inline int nt_sign_text(char sign, char* buf, size_t bufsize, size_t* pos){

	struct sign_text_s* s;
	
	for(s = sign_text; s->text; s++){
		if(s->sign == sign){
			snprintf(buf + (*pos), bufsize - (*pos), "%s ", s->text);
			*pos += strlen(s->text) + 1;
            return true_;
		}
	}
	
    return false_;

}

static inline int nt_numgroup_hundreds(int val, char* buf, size_t bufsize, size_t* pos){
	
	switch(val){
		case 2:
		snprintf(buf + (*pos), bufsize - (*pos), "%s", hundreds_group_text[0].text);
		*pos += strlen(hundreds_group_text[0].text);
		break;
	
		case 3:
		snprintf(buf + (*pos), bufsize - (*pos), "%s", hundreds_group_text[1].text);
		*pos += strlen(hundreds_group_text[1].text);
		break;
		
		case 5:
		snprintf(buf + (*pos), bufsize - (*pos), "%s", hundreds_group_text[2].text);
		*pos += strlen(hundreds_group_text[2].text);
		break;
		
		default:
				
		nt_decimal_group(val, buf, bufsize, pos);
		snprintf(buf + (*pos), bufsize - (*pos), "%s", group_pos_text[GPOS_HUNDREDS].text);
		*pos += strlen(group_pos_text[GPOS_HUNDREDS].text);
		
		break;
		
	}

    return true_;
}

/*	format functions	*/
static inline char* nt_clean_float(long double num, int decim, char* buf, size_t bufsize){
    int i;

    char fmt[10];

    if(decim > MAXNUMLEN - 1){
        decim = MAXNUMLEN - 1;
    }

    snprintf(fmt, 10, FMT, decim);

    snprintf(buf, bufsize, fmt, num);

    if(decim){

        i = strlen(buf)-1;

        while(buf[i] == '0') i--;

        if(buf[i] != '.') buf[i+1] = '\0';
        else buf[i] = '\0';

    }

    return buf;
}

static inline int nt_convert_numgroup(char* numgroup, char* buf, size_t bufsize, size_t* pos){

	int nonzero = 0;
	int val;
    char hbuf[2] = {0, 0};

    //perror(numgroup);

    //if(gpos == GPOS_INVAL) return E13_ABORT;

    if(*numgroup != '0'){
        *hbuf = *numgroup;
        nt_numgroup_hundreds(atoi(hbuf), buf, bufsize, pos);
		nonzero = 1;
	}
	
    if((val = atoi(numgroup + 1))){

		if(nonzero){

			snprintf(buf + (*pos), bufsize - (*pos), " %s ", WORD_AND);
			*pos += strlen(WORD_AND) + 2;

		}

        nt_decimal_group(atoi(numgroup + 1), buf, bufsize, pos);
		
		nonzero++;
	}
	
//    if(gpos > GPOS_HUNDREDS){

//        snprintf(buf + (*pos), bufsize - (*pos), " %s", group_pos_text[gpos].text);
//        *pos += strlen(group_pos_text[gpos].text) + 1;
	
//    } else {

//		return E13_CONTINUE;

//	}
	
    return true_;

}

static inline int nt_convert_part(char* part, char* buf, size_t bufsize, size_t* pos){

    //int len;
    int ngpos, gpos;
    char numgroup[NUMGROUPLEN];
	char* needle;
	int i;
	
    ngpos = strlen(part)/3;
    if(strlen(part)%3) ngpos++;

    if(ngpos >= GPOS_INVAL){
        return false_;
    }

    //snprinte1("part: %s, len: %u, ngpos: %i", part, ngpos);
    //perror(errmsg);
    //printe1();

    needle = part;
    for(gpos = 0; gpos < ngpos; gpos++){

        //if(gpos > GPOS_TRILLIONS) break;

        assert(gpos <= GPOS_TRILLIONS);
	
        if(!gpos && (i = strlen(part)%3)){

            memset(numgroup, '0', NUMGROUPLEN - 1);

			
			if(i == 2){
                *(numgroup + 1) = *needle;
				needle++;
			}
			
            *(numgroup + 2) = *needle;
			needle++;

		} else {

            *numgroup = *needle;
			needle++;
            *(numgroup + 1) = *needle;
			needle++;
            *(numgroup + 2) = *needle;
			needle++;

		}
		
        //atoi := strtol(nptr, (char **) NULL, 10);

        //snprinte1("%s", numgroup);
        //perror(errmsg);
        //printe1();

        if(atoi(numgroup)){

            if(gpos){
                snprintf(buf + (*pos), bufsize - (*pos), " %s ", WORD_AND);
                *pos += strlen(WORD_AND) + 2;
            }

            nt_convert_numgroup(numgroup, buf, bufsize, pos);

            if(ngpos - gpos > GPOS_HUNDREDS){

                snprintf(buf + (*pos), bufsize - (*pos), " %s", group_pos_text[ngpos - gpos].text);
                *pos += strlen(group_pos_text[ngpos - gpos].text) + 1;

            }
		}
	
	}
	
    return true_;
}

static inline int nt_convert_str(char* numbuf, char* buf, size_t bufsize){
    size_t pos = 0;
    char* int_part;
    char* float_part;
    int ret;

    int_part = numbuf;        

    snprinte1("numbuf: %s", numbuf);

    *buf = 0;

    if((float_part = strchr(numbuf, '.'))){
        *float_part = 0;
        float_part++;
        if(!(*float_part)) float_part = NULL;
    }

    if(*numbuf == '-'){
        nt_sign_text('-', buf, bufsize, &pos);
        int_part++;
    }

    if(!atoi(int_part)){

        snprintf(buf + (pos), bufsize - (pos), "%s", decimal_group_text[0].text);
        pos += strlen(decimal_group_text[0].text);

    } else {
        if((ret = nt_convert_part(int_part, buf, bufsize, &pos) != true_)) return ret;
    }

    if(float_part){

        strcat(buf, " ");
        pos++;
        nt_sign_text('.', buf, bufsize, &pos);

        if(!atoi(float_part)){
            snprintf(buf + (pos), bufsize - (pos), "%s", decimal_group_text[0].text);
            pos += strlen(decimal_group_text[0].text);
        } else {
            if((ret = nt_convert_part(float_part, buf, bufsize, &pos) != true_)) return ret;
        }

        switch(strlen(float_part)){
            default:
            break;
        }

    }

    return true_;
}

int nt_convert(long double val, char* buf, size_t bufsize){
	
    char numbuf[MAXNUMLEN*2];
	
	//handle sign
	//handle decimal pt

    snprinte1("%Lf %f", val, val);
    //printe1();
	
    nt_clean_float(val, MAXNUMLEN, numbuf, MAXNUMLEN*2);
	
    return nt_convert_str(numbuf, buf, bufsize);
}

int num2text(struct session* sess){
    struct tok_s *tk;
    fnum_t val;

    char buf[2048];

    tk = poptok(sess);
    if(!tk){
        val = var_val(sess, ANSVARNAME);
    } else {
        val = tokval(sess, tk);
    }

    switch(sess->cursess->uilocale){
    case SESS_LOCALE_EN:
        if(!nt_convert_en(val, buf, 2048)){
            return false_;
        }
        break;
    case SESS_LOCALE_FA:
        if(!nt_convert(val, buf, 2048)){
            return false_;
        }
        break;
    default:
        printe(MSG_BADLOCALE);
        return false_;
        break;
    }

    return cp(sess, buf);

//    tk1 = make_stacktok(sess, buf);
//    if(!tk1){
//        printe(MSG_OVERFLOW, "stack");
//        return false_;
//    }

//    tk1->id = TOK_STR;
//    tk1->type = TOKT_STRING;

//    printd7("pushing '%s' to calc stack."PARAEND, tk1->str);
//    push_calc(sess, tk1);

//    return true_;
}
