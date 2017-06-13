
//array(x[200]);array(y[200]);x[0]:-100*.1;y[0]:x[0]^2;i:0;label a;i:i+1;x[i]:x[i-1]+.1;y[i]:x[i]^2;if(i==199){jump;};goto a;
//TODO:array(x[200]);array(y[200]);x[0]:-100*.1;y[0]:x[0]^2-x[0];i:0;label a;i:i+1;x[i]:x[i-1]+.1;y[i]:x[i]^2-x[i];if(i==199){jump;};goto a;
#ifndef _DOUBLECALCULATOR_H
#define _DOUBLECALCULATOR_H

#define _CRT_RAND_S

#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#include <pthread.h>

#ifdef WIN32
#include <Windows.h>
#endif

#define CONSOLE

//defined in dev-cpp project file, uncomment to make console version on other platforms
//#define CONSOLE
#ifndef CONSOLE
#define NDEBUG
#endif

#ifndef NDEBUG
#include <assert.h>
#else
#define assert(x)
#endif

#ifdef CONSOLE
#undef QT
#endif

//#ifdef QT
//#define NOFORK
//#endif

#ifndef QT
#define PARAEND "\n"
#else
#define PARAEND ""
#endif

#ifndef NDEBUG
#define _GLIBCXX_DEBUG
#endif

//define it to use mutextes on output
//#ifdef QT
//#define OUT_MX
//#endif
#undef OUT_MX

//safe 99999999999999999999999999
//or 10^19
//or 1000000000000000000000000000

#ifdef WIN32
#define EOL	"\n\r"
#else
#define EOL	"\n"
#endif

typedef long double fnum_t;
typedef long long inum_t;

//++ on engine feature change
#define ENGINEVER   (0x001b+0x001b)
#define ENGINECAPS_LONG	"+SSE2 +CodeInjection +Daedalus -TokenizeOnType"
#define ENGINECAPS_SHORT	"+sse2 +ci +ddlus -tot"

//++ on upper change
#define SRCVER  "0.92"
#define SRCSTAGE    "-b"

#ifdef QT
//next Cute2, ++ on GUI change
#define UIVER    "Cute1G_0.1"
#define UISTAGE  "-b"
#define UICAPS_LONG	"+MultiLine +ArrayEdit +Plot +KeyPad"
#define UICAPS_SHORT "+mline +aed +pl +kpad"
#else
//++ on CUI change
#define UIVER    "Con_0.1"
#define UISTAGE  "-rc"
#define UICAPS_LONG	"-Window"
#define UICAPS_SHORT	"-w"
#endif

//++ on bugfix
#define VERSION SRCVER""SRCSTAGE"_"UIVER""UISTAGE

//#ifndef NDEBUG
//#define VERSION VERSION"-debug"
//#endif

#define ROOT_SESSION_NAME   "root"

#define NOTSET "N/A"
#define ANSDESC "Last answer"

#define ICON_SIZE       4
#define ERR_ICON        "[#] "
#define WARN_ICON       "[!] "
#define INFO_ICON       "[i] "
#define ANS_ICON        "[=] "
#define RESULT_ICON     "[:] "
#define HELPBLOCK_ICON  "--] "
#define INFOBLOCK_ICON  "[{] "
#define HELPINFO_ICON   "- [ "
#define NOP_ICON        "[   "
#define QUESTION_ICON	"[?] "

#define COMMENT_SIGN "#"
#define COMMENT_CHAR '#'
#define COMMENT_SIGN_SIZE 1

#define MSG_MOREARGS    "Need more arguments [%s]."PARAEND
#define MSG_BADARG      "Bad argument type [%s]."PARAEND
#define MSG_NOMEM       "Out of memmory."PARAEND
#define MSG_EXISTS      "Already exists [%s]."PARAEND
#define MSG_NOTFOUND    "Not found [%s]."PARAEND
#define MSG_ANOTFOUND   "Array Not found [%s[%i][%i]]."PARAEND
#define MSG_FORBID      "Operation not permitted [%s]."PARAEND
#define MSG_BADCALC     "Calculation error."PARAEND
#define MSG_BADLOGIC    "Logic error."PARAEND
#define MSG_BADLOGICOM  "Combination error '%s %s'"PARAEND
#define MSG_BADEXP      "Expression error [%s]."PARAEND
#define MSG_IMPLEMENT   "Not implemented on your platform [%s]."PARAEND
#define MSG_FORMAT      "Bad format [%s]."PARAEND
#define MSG_READY       " > "
#define MSG_NOOPEN      "Could not open file [%s]."PARAEND
#define MSG_NOREAD      "Could not read from file [%s]."PARAEND
#define MSG_ERROR       "Error [%s]."PARAEND
#define MSG_OK          "Ok."PARAEND
#define MSG_EMPTY       "Empty resource [%s]."PARAEND
#define MSG_FAIL        "Failed to complete operation [%s]."PARAEND
#define MSG_BUSY        "Resource is busy [%s]."PARAEND
#define MSG_RANGE       "Out of range [%s]."PARAEND
#define MSG_BADUSE      "Wrong usage [%s]."PARAEND
#define MSG_OVERFLOW    "Overflow [%s]."PARAEND
#define MSG_SYNTAX2     "Syntax error near '%s %s'."PARAEND
#define MSG_MATCH       "Not found a match near '%s %s'."PARAEND
#define MSG_PARSE2      "Parse error near '%s %s'."PARAEND
#define MSG_CHILDPROC   "Child Process"
#define MSG_BADLOCALE   "Bad locale."PARAEND
#define MSG_SIZEMIS     "Size mismatch [%s->%i != %s->%i]."PARAEND
#define MSG_SSTAT       "State saved to [ %s ]."PARAEND
#define MSG_CIRCULAR    "Circular refrence %s[%i][%i] & %s[%i][%i]."PARAEND
#define MSG_NOLOCKFILE  "Could not create lock file."PARAEND
#define MSG_LOCALLOCK   "Another instance is running, Continue?"

//ABOUT MSGS
#define MSG_ABOUT_HELP      HELPINFO_ICON"\n[ Double, the programmable command line calculator. ]"\
                            PARAEND

#define MSG_ABOUT_VER       NOP_ICON"[ engine version: "SRCVER", ui version: "UIVER" ]"\
                            PARAEND

#define MSG_ABOUT_NOTES NOP_ICON"* The purpose was, creating a program to make some engineering\n"\
                        "  tasks ( which may need some simple scripting and automation )\n"\
                        "  easier, it helped me lots while writing invioces; I hope it\n"\
                        "  becomes useful for others with similar needs.\n"\
                        "* Try \"help item\" for a quick help on item.\n"\
                        "* Be careful using EXPERIMENTAL features, they are\n"\
                        "  still under developement and better to be avoided\n"\
                        "  while being tested, see quick help of the functions."\
                        PARAEND

#define MSG_ABOUT_ME     HELPINFO_ICON"[ by:    %s                        ]\n"\
                         "[ email: %s                    ]\n"\
                         "[ web:   %s ]"\
                         PARAEND

#define MSG_ABOUT_LIC   NOP_ICON"* The current version of this software is free for\n"\
                        "  commercial and non-commercial use,\n"\
                        "  this may change in future versions, however!"\
                        PARAEND

#define dbl_blocklist "{}"
#define dbl_brlist "()"
#define dbl_logiclist "<>!=\"\'{}"
#define dbl_delimlist ",~$[](){}<>!=&|:+-*/^% \t\"\'\n\r;"
#define dbl_packlist "\"\'"
#define dbl_escape '\\'
#define dbl_lf	";"
#define dbl_array "[]"
#define dbl_chain '@'
#define dbl_chain_str "@"
#define NOVAREXP ""

#define PI	3.1415926535897932384626433832795
#define E	2.7182818284590452353602874713527

#define ANSVARNAME	"ans"
#define PIVARNAME	"pi"
#define EVARNAME	"e"
#define CLIPVARNAME	"clip"

#define ANSVARDESC	"last answer"
#define PIVARDESC	"pi number"
#define EVARDESC	"e number"
#define CLIPVARDESC	"clipboard contents"

#define DEF_EXP_BUFSIZE	1024
#define DEF_TOK_BUFSIZE	128
#define DEF_OUT_BUFSIZE 1024
#define DEF_HIST        20
#define DEF_EXP_FLAGS	0
#define DEF_MAX_HIST    DEF_HIST*10
#define DEF_MIN_HIST    1
#define DEF_MAX_EXPBUF  DEF_EXP_BUFSIZE*10
#define DEF_MIN_EXPBUF  DEF_EXP_BUFSIZE
#define DEF_MAX_OUTBUF  DEF_OUT_BUFSIZE*10
#define DEF_MIN_OUTBUF  DEF_OUT_BUFSIZE
#define DEF_MAX_TOKBUF  DEF_TOK_BUFSIZE*10
#define DEF_MIN_TOKBUF  DEF_TOK_BUFSIZE
#define DEF_STAT_FILENAME   "last.stt"
#define DEF_AUTOLOAD_FILENAME   "autoload"
#define DEF_LOCALLOCK_FILENAME  ".double.lk"
#define MAXPATHNAME	1024

#define MAXNUMLEN 21

#define DEF_LOAD_RES_VARS()  int autorun_reg, ifcond_reg;struct pre_exp* autopre_reg, *pre_reg; struct expression* exp_reg

//1 used in recalc
#define LOAD(dis_auto)     MACRO(\
                            if(dis_auto){\
                                autorun_reg = sess->cursess->autorun;\
                                sess->cursess->autorun = AUTORUN_NOTDEF;\
                                autopre_reg = sess->cursess->autopre;\
                                sess->cursess->autopre = NULL;\
                            }\
                            pre_reg = sess->cursess->pre;\
                            if(pre_reg)exp_reg = sess->cursess->pre->exp; else exp_reg=NULL;\
                            if(exp_reg)ifcond_reg = sess->cursess->pre->exp->ifcond; else ifcond_reg = IFCOND_NOIF;\
                            )

#define RESTORE(dis_auto)  MACRO(\
                            if(dis_auto){\
                                sess->cursess->autorun = autorun_reg;\
                                sess->cursess->autopre = autopre_reg;\
                            }\
                            sess->cursess->pre = pre_reg;\
                            if(pre_reg)sess->cursess->pre->exp = exp_reg;\
                            if(exp_reg)sess->cursess->pre->exp->ifcond = ifcond_reg;\
                            )

#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

/*
**  Macros to manipulate bits in an array of char.
**  These macros assume CHAR_BIT is one of either 8, 16, or 32.
*/

#define MASK  (CHAR_BIT-1)
#define SHIFT ((CHAR_BIT==8)?3:(CHAR_BIT==16)?4:5)

#define BITOFF(a,x)  ((void)((a)[(x)>>SHIFT] &= ~(1 << ((x)&MASK))))
#define BITON(a,x)   ((void)((a)[(x)>>SHIFT] |=  (1 << ((x)&MASK))))
#define BITFLIP(a,x) ((void)((a)[(x)>>SHIFT] ^=  (1 << ((x)&MASK))))
#define ISBIT(a,x)   ((a)[(x)>>SHIFT]        &   (1 << ((x)&MASK)))


#define false_ 0
#define true_ 1
#define continue_ 2

#define ASSIGN_PRED 201

enum ifcond_t{
    IFCOND_NOIF,
    IFCOND_TRUE,
    IFCOND_FALSE,
    IFCOND_ENDIF	//if condition ends
};

enum autorunstat_t{
    AUTORUN_NOTDEF,
    AUTORUN_DEFON,
    AUTORUN_DEFOFF,
    AUTORUN_RUNNING,
    AUTORUN_DEFNOW
};

enum vtable_rec_type{
    VTAB_TT, //type-type
    VTAB_II, //id-id
    VTAB_TI, //type-id
    VTAB_IT, //id-type
    VTAB_ENDS //ENDS
};

enum tok_t{

    TOK_EMPTY,

    TOK_SEP,
    TOK_COL,
    TOK_WHITE,
    TOK_STRPACK,

    TOK_CHAIN,//5
//    TOK_COLON,

    TOK_BR_OPEN,
    TOK_BR_CLOSE,

    TOK_BLOCK,
    TOK_BLOCK_OPEN,
    TOK_BLOCK_CLOSE,//10

    TOK_ABR_OPEN,
    TOK_ABR_CLOSE,

    TOK_INT,
    TOK_REAL,
    TOK_SCIENCE,
    TOK_DATE,

    TOK_VAR,
    TOK_ARRAY,
    TOK_STR,//19

//    TOK_EQ_LONE,
//    TOK_COLUMN,

    TOK_PLUS,
    TOK_MINUS,
    TOK_MULT,
    TOK_DIV,
    TOK_POW,
    TOK_MOD,
    TOK_ASSIGN,
    TOK_AND,
    TOK_OR,
    TOK_PLUS_PLUS,
    TOK_MINUS_MINUS,
    TOK_PLUS_EQ,
    TOK_MINUS_EQ,

    TOK_SIN,
    TOK_ASIN,
    TOK_SINH, //20
    TOK_COS,
    TOK_ACOS,
    TOK_COSH,
    TOK_TAN,
    TOK_ATAN,
    TOK_RAD,
    TOK_DEG,
    TOK_LOG,
    TOK_LN,
    TOK_SQRT,//45
    TOK_ABS,
    TOK_ROUND,
    TOK_NROUND,
    TOK_JDAY,
    TOK_GDAY,
    TOK_COUNT,
    TOK_ANS_REF,

    TOK_LT, //<
    TOK_GT, //>
    TOK_EQ, //==
    TOK_NE, //!=
    TOK_LE, //<=
    TOK_GE, //>=

    TOK_AND_AND,//&&
    TOK_OR_OR,//||
    TOK_NOT,//!

    TOK_IF,
    TOK_ELSEIF,
    TOK_ELSE,
    TOK_FOR,

    TOK_CP,
    TOK_SEPCP,
    TOK_NUM2TEXT,

    TOK_VCLR,
    //TOK_VCLRALL,
    TOK_VDESC,
    TOK_VEXP,
    TOK_VCAT,
    TOK_VLIST,
    TOK_VSTR,
    TOK_VSTRCMP,
    TOK_VLOCK,
    TOK_VUNLOCK,
    TOK_VREN,
    TOK_VDUP,
    TOK_ARRAYDEF,
    TOK_AFILL,

    TOK_LOAD,
    TOK_SAVE,
    TOK_FLUSHF,
    TOK_VIEW,

    TOK_RECALC,
    TOK_XRECALC,
//    TOK_LOOP,
    TOK_LABEL,
    TOK_GOTO,
    TOK_RETURN,
    TOK_JUMP,
    TOK_AUTORUN,

    TOK_HISTORY,
    TOK_HLOAD,

    TOK_HIJRI,
    TOK_GREG,
    TOK_TODAY,
    TOK_JDATE,
    TOK_GDATE,

    TOK_MODE,

    TOK_FORK,
    TOK_ENDFORK,
    TOK_SWITCH,
    TOK_BREAK,
    TOK_SLEEP,
    TOK_WAIT,
    TOK_EXEC,

    TOK_PRINT,
    TOK_PLOT,
    TOK_AEDIT,

    TOK_STOP,
    TOK_RESUME,
    TOK_SHOW,
    TOK_REMOVE,
    TOK_UP,
    TOK_DOWN,
    TOK_TRIM,
    TOK_NORMAL,
    TOK_ACC,
    TOK_PROG,
    TOK_ROOT,
    TOK_PARENT,
    TOK_ALL,
    TOK_HEX,
    TOK_BIN,
    TOK_UI,
    TOK_FILE,
    TOK_BUF,
	TOK_CORE,

    TOK_SYS,
    TOK_VER,
    TOK_HELP,
    TOK_EXIT,
    TOK_TERM,
    TOK_SAVESTAT,

    TOK_INVAL
};

enum tok_type_t{
    TOKT_EMPTY,//for compatibility with tok_t
    TOKT_ALL,
    TOKT_BLOCK,
    TOKT_SPEC,
    TOKT_NUMBER,
    TOKT_MATH_BASIC,
    TOKT_STRING,
    TOKT_MATH_FUNC,
    TOKT_MATH_LOGIC,
    TOKT_FUNC,
    TOKT_COMMAND,
    TOKT_RESERVE,
    TOKT_INVAL
};

#define MACRO(A) do { A; } while(0)

#define textout_(str)		MACRO( outtext_(sess, str); )

#define printo(fmt, ... )   MACRO(  snprintf(sess->cursess->outbuf, sess->cursess->outbufsize, fmt, ## __VA_ARGS__);\
                                    textout_(sess->cursess->outbuf); )

#define printI(ICON, fmt, ...)  MACRO(  strcpy(sess->cursess->outbuf, ICON);\
                                        if(sess->cursess->uiflags & SESS_UIF_DET){\
                                            snprintf(sess->cursess->outbuf + ICON_SIZE,\
                                            sess->cursess->outbufsize - ICON_SIZE, "@%s: ", sess->cursess->name);\
                                        }\
                                        snprintf(   sess->cursess->outbuf + strlen(sess->cursess->outbuf),\
                                                    sess->cursess->outbufsize - strlen(sess->cursess->outbuf), fmt,\
                                                    ## __VA_ARGS__);\
                                        textout_(sess->cursess->outbuf);\
                                            )


#define printe(fmt, ... )   MACRO( printI(ERR_ICON, fmt, ## __VA_ARGS__); )

#define printw(fmt, ... )   MACRO(  if(sess->cursess->uiflags&SESS_UIF_LOUD){printI(WARN_ICON, fmt, ## __VA_ARGS__);} )

#define printi(fmt, ... )   MACRO(  if(sess->cursess->uiflags&SESS_UIF_LOUD){printI(INFO_ICON, fmt, ## __VA_ARGS__);} )

#define printi2(fmt, ...)   MACRO(  printI(INFO_ICON, fmt, ## __VA_ARGS__); )

#define printq(fmt, ...)   MACRO(  printI(QUESTION_ICON, fmt, ## __VA_ARGS__); )

#define printD(fmt, ... )	MACRO(  snprintf(sess->cursess->outbuf,\
                                        sess->cursess->outbufsize, "$%s@%s()#%i> ", __FILE__,\
                                        __func__, __LINE__);\
                                    snprintf(sess->cursess->outbuf + strlen(sess->cursess->outbuf),\
                                        sess->cursess->outbufsize - strlen(sess->cursess->outbuf),\
                                        fmt, ## __VA_ARGS__);\
                                    textout_(sess->cursess->outbuf);\
                                    )

#define printd_for(fmt, ...) printD(fmt, ## __VA_ARGS__ )

//#define DEBUG1_DEF() struct tok_s* tk_reg
//#define DEBUG1() MACRO(tk_reg = sess->cursess->pre->exp->prefixfirst->prenext; sess->cursess->pre->exp->prefixfirst->prenext = sess->cursess->pre->exp->prefixfirst->next; sess->cursess->pre->exp->prefixfirst->prenext = tk_reg;)

#define DEBUG1_DEF()
#define DEBUG1()

#ifndef NDEBUG
#define printd(fmt, ... ) printD(fmt, ## __VA_ARGS__ )
#define printd0(fmt, ...) printD(fmt, ## __VA_ARGS__ )
#define printd2(fmt, ...) printD(fmt, ## __VA_ARGS__ )
#define printd3(fmt, ...) printD(fmt, ## __VA_ARGS__ )
#define printd4(fmt, ...) printD(fmt, ## __VA_ARGS__ )
#define printd5(fmt, ...) printD(fmt, ## __VA_ARGS__ )
#define printd6(fmt, ...) printD(fmt, ## __VA_ARGS__ )
#define printd7(fmt, ...) printD(fmt, ## __VA_ARGS__ )
#define printd8(fmt, ...) printD(fmt, ## __VA_ARGS__ )
#define printd9(fmt, ...) printD(fmt, ## __VA_ARGS__ )
#define printd10(fmt, ...) printD(fmt, ## __VA_ARGS__ )
#define printd11(fmt, ...) printD(fmt, ## __VA_ARGS__ )
#define printd12(fmt, ...) printD(fmt, ## __VA_ARGS__ )
#define printd13(fmt, ...) printD(fmt, ## __VA_ARGS__ )
#define printd14(fmt, ...) printD(fmt, ## __VA_ARGS__ )
#define printd15(fmt, ...) printD(fmt, ## __VA_ARGS__ )
#define printd16(fmt, ...) printD(fmt, ## __VA_ARGS__ )
#define printd17(fmt, ...) printD(fmt, ## __VA_ARGS__ )
#define printd18(fmt, ...) printD(fmt, ## __VA_ARGS__ )
#define printd_comb1(fmt, ...) printD(fmt, ## __VA_ARGS__ )
#else
#define printd(fmt, ... )
#define printd2(fmt, ...)
#define printd3(fmt, ...)
#define printd4(fmt, ...)
#define printd5(fmt, ...)
#define printd6(fmt, ...)
#define printd7(fmt, ...)
#define printd8(fmt, ...)
#define printd9(fmt, ...)
#define printd10(fmt, ...)
#define printd11(fmt, ...)
#define printd12(fmt, ...)
#define printd13(fmt, ...)
#define printd14(fmt, ...)
#define printd15(fmt, ...)
#define printd16(fmt, ...)
#define printd17(fmt, ...)
#define printd18(fmt, ...)
#define printd_comb1(fmt, ...)
#endif

#define TOKF_NEG	(0x01<<0)//negative
#define TOKF_RES	(0x01<<1)//resolved
//#define TOKF_DYN	(0x01<<2)//dynamically allocated string

#define VAR_NORM	0x00//normal
#define VAR_SYS		0x01//system, e.g. ans, pi, e
#define VAR_CTL		0x02//control, e.g. bufsize
#define VAR_ARRAY	0x03//array

#define VARF_LOCK	(0x01<<0)//locked
#define VARF_DATE	(0x01<<1)//jalali-date, for ans

#define MAXARRAYMEMBERS  10000
#define NMAXARRAYMEMBERS 4

#define EXPF_NEGNEXT	 (0x01<<0)
#define EXPF_FOR_LOGIC   (0x01<<1)
#define EXPF_FOR_ADVANCE (0x01<<2)

#define SESSF_EXIT      (0x01<<0)
#define SESSF_GOTO      (0x01<<1)
#define SESSF_RUNNING   (0x01<<2)//thread running, avoid @
#define SESSF_BREAK     (0x01<<3)//break;

#define PREF_FOR_ADVANCE (0x01<<0)

#define SESSM1_NR        0x00
#define SESSM1_ACC       0x01
#define SESSM1_PROG      0x02

#define SESS_LOCALE_EN  (0x00)
#define SESS_LOCALE_FA  (0x01)

#define SESS_UIF_DET        (0x0001<<0)
#define SESS_UIF_LOUD       (0x0001<<1)
#define SESS_UIF_ANS        (0x0001<<2)
#define SESS_UIF_HILIGHT    (0x0001<<3)
#define SESS_UIF_DECIMNOTR  (0x0001<<4)//do not trim decimal
#define SESS_UIF_NOSEP      (0x0001<<5)//no sepval
#define SESS_UIF_NERD       (0x0001<<6)
#define SESS_UIF_CTLBUF     (0x0001<<7)//enable buffer control messages
#define SESS_FILEF_LINE     (0x0001<<1)
#define SESS_FILEF_TEXT     (0x0001<<2)
#ifdef CONSOLE
#define SESS_DEF_UIF    (SESS_UIF_DET|SESS_UIF_LOUD|SESS_UIF_ANS)
#define SESS_DEF_FILEF  (SESS_FILEF_LINE|SESS_FILEF_TEXT)
#define SESS_DEF_LOCALE SESS_LOCALE_EN
#else
#define SESS_DEF_UIF    (SESS_UIF_LOUD|SESS_UIF_ANS|SESS_UIF_HILIGHT)
#define SESS_DEF_FILEF  (SESS_FILEF_LINE|SESS_FILEF_TEXT)
#define SESS_DEF_LOCALE SESS_LOCALE_FA
#endif

#define SESS_DEF_DECIM  2
#define SESS_MAX_DECIM  10
#define SESS_MIN_DECIM  0

struct variable{
    char type;
    char flags;
    char* name;
    fnum_t val;
    char* exp;
    char* desc;
    int i, j;
    struct variable* next;
    struct pre_exp* pre;
    struct variable* dep_next;
};

struct tok_s{
    enum tok_t id;
    enum tok_type_t type;
    char* form[15];//max 15 characters
    char* desc;
    char* str;
    int pred;
    struct tok_s* next;
    char flags;
    fnum_t val;
    int i, j;//for now, this acts only as array index
    struct tok_s* prenext;//prefix next, this helps me to save the order of prefix tokens and restore them later.
    fnum_t valreg;
    struct tok_s* infixnext;//infix next
};

struct expression{

    char flags;
    char* infix;
    char* infixrest;
    char* varinfix;

    enum ifcond_t ifcond;

    struct tok_s* curtok, *prevtok;
    struct tok_s* infixfirst, *infixlast, *vdeftok;
    char delim[2];

    struct tok_s* prefixfirst, *prefixlast, *saveprefixfirst;
    struct tok_s* stacktop;

    struct variable* var;//the current variable to assign value

    int nstacktok;
    int stacktoki;
    struct tok_s* stacktoks;

    size_t stacktokstrbufi;
    size_t stacktokstrbufsize;
    char* stacktokstrbuf;

//    char* bufex;//extended buffer, keeps overflow from some calls like vstr and num2text
//    size_t bufexsize;

    struct expression* next;
    struct expression* prev;
    struct pre_exp* branch;//this is mostly for codeblocks
};

struct vtable{
    int cur[10];
    int prev[10];
    enum vtable_rec_type rt;
    int stat;
};

struct label{
    //char* next;
    //char* infixrest;
    char* name;
    int stacktoki;
    int stacktokstrbufi;
    struct pre_exp* pre;//the pre that the expression resides!
    struct expression* nextexp;//pointer to the next expression
};

//struct autorun{
//    int stat;
//    char* autoexp;
//    int autostack;//what's this? for god's sake!

//    struct tok_s* prefixfirst, *prefixlast;
//};

//this is precompiled expression
struct pre_exp{

    char flags;

    size_t nexp;
    struct expression* expfirst, *explast;
    struct expression* exp;

    char* gsbuf;//global stack string buffer
    size_t gsbufi;//gsbuf index
    size_t gsbufsize;
    struct tok_s* tokpool;//token pool
    size_t tokpooli;//index of above
    size_t tokpoolsize;

};

#ifdef WIN32
typedef int timerid_t;
#else
typedef int timerid_t;
#endif

struct timerinfo{
    timerid_t id;
    time_t sched;
    unsigned int len;
    char* name;
    char* exp;
    struct timerinfo* next;
};

struct session{

    char flags;

    int autorun;
//    char* autoexp;
//    int autostack;

    //int exploop;
    int njump;

    unsigned short uiflags;
    unsigned short fileflags;

    char uilocale;

    unsigned short decim;//decimal pt

    char mode1;//mode1, [ acc, nr ]

    int calc;//number of calculations done inside the input line.
    int calcreg;//calc register, helps to keep the track of real calcs.

    struct variable* vfirst, *vlast;

    //precompiled expression
    struct pre_exp* pre;
    struct pre_exp* autopre;

    char* tokbuf;
    char* expbuf;
    char* next;

    size_t nlabels;
    struct label* labels;

    struct timerinfo* timers;

    size_t expbufsize;
    size_t tokbufsize;
    size_t outbufsize;

    FILE* outfile;

    size_t maxhist;
    size_t nhist;
    char** history;
    int curhist;

    char* outbuf;

    void* rd;
    void* wr;
    void* ui;
    void* aed;//array editor

    #ifdef QT
    void* clp;
    #endif

    //timing
#ifdef _WIN32
    double PCFreq;
    __int64 CounterStart;
    LARGE_INTEGER li;
#else
    struct timespec tS;
#endif

    double rt;

    //term
    unsigned short term_id;//terminal id

    //fork()
    int nchild;
    //int id;
    char* name;
    pthread_mutex_t mx;//, uimx;
#ifdef OUT_MX
    pthread_mutex_t uimx;
#endif
    struct session* cursess;
    struct session* root;
    struct session* parent;
    struct session* nextchild;

};

#ifdef __cplusplus
    extern "C" {
#endif

char* gettok(   char* exp,
                const char* dlist,
                const char esc,//escape character
                const char* plist,//must be defined inside the delimlist too
                const char* blist,//like above, blist[0]=bstart, blist[1]=bend
                char* buf,
                size_t* bufsize,
                char* delim
                );
int isfloat(char* str);
int isint(char* str);
int syntax(struct session* sess);
enum tok_t delimtokid(char ch);
struct tok_s* delimtokptr(char ch);
enum tok_t tokid(struct session *sess, char* str);
enum tok_type_t toktype(char* str);
struct tok_s* tokptr(struct session *sess, char* str);
int istokid(char* str, enum tok_t id);
int pred(char* str1, char* str2);
int charcount(const char* str, const char* chlist);
struct tok_s* strtoks(struct session* sess, char* str);
int alloc_stack_toks(struct session* sess, int ntok);
int record(struct session* sess);
int prefix(struct session* sess);
int runprefix(struct session* sess);
int show_ans(struct session* sess);
int savefile(struct session* sess, char* filename);
int loadfile(struct session* sess, char* filename);
struct tok_s* get_stacktok_ptr(struct session* sess);
void unget_stacktok_ptr(struct session* sess);
struct tok_s* make_stacktok(struct session* sess, char* str);
void pop_to_pre(struct session* sess);
int pop_to_br(struct session* sess);
void push(struct session* sess);
int run(struct session* sess);
void insert_pre(struct session* sess, struct tok_s* tk);
struct tok_s* poptok(struct session* sess);
fnum_t tokval(struct session* sess, struct tok_s* tk);
int pop_all(struct session* sess);
void push_calc(struct session* sess, struct tok_s* tk);
int create_var(struct session* sess, char* name, char type, int idx, int jdx, char* infixrest);
fnum_t var_val(struct session* sess, char* name);
int set_varval(struct session* sess, char* name, fnum_t val);
int run_cmd(struct session* sess,struct tok_s* ent);
void list_var(struct session* sess);
int run_fn(struct session* sess, struct tok_s* ent);
void show_help(struct session* sess);
char* clean_float(struct session* sess, fnum_t num, char* buf, size_t bufsize);
char* sep_val(struct session* sess, fnum_t num, char* buf, size_t bufsize, char* tmp, size_t tmpsize);
struct variable* var_ptr(struct session* sess, char* name, struct variable** prev);
int closesavefile(struct session* sess);
int show_vdesc(struct session* sess, char* name);
int rm_var(struct session* sess, char* name);
int clear_all_vars(struct session* sess);
int rm_vdesc(struct session* sess, char* name);
struct tok_s* tokidptr(struct session* sess, enum tok_t id);
int create_sysvar(struct session* sess, char* name, char* desc, fnum_t val);
//int create_ctlvar(struct session* sess, char* name, char* desc, fnum_t val);
int rm_ansvar(struct session* sess);
int set_ansvar(struct session* sess, fnum_t ans);
int set_vdesc(struct session* sess, char* name, char* desc);
int init_hist(struct session* sess);
int get_history(struct session* sess, char* buf, size_t bufsize, int order);
int save_history(struct session* sess);
int list_history(struct session* sess);
void remove_prehead(struct session* sess);
size_t strcpy_(char *dest, const char *src, size_t size);
size_t strcat_(char *dest, const char *src, size_t count);
void print_listhead(struct session* sess, char* name);
void print_listfoot(struct session* sess, char* name);
void show_tok_desc(struct session* sess, struct tok_s* tk);
int show_result(struct session* sess);
int handle_if(struct session* sess);
int init_stack(struct session* sess);
int free_stack(struct session* sess);
void printval(struct session* sess, fnum_t val);
//int _run_codeblock(struct session* sess, char* buf);
int vlock(struct session* sess, char* name);
int vunlock(struct session* sess, char* name);
int vlock_show(struct session* sess);
int vunlock_show(struct session* sess);
fnum_t strval(struct session* sess, char* str);
int pop_to_br_for_col(struct session* sess);
struct variable* array_ptr(	struct session* sess,
                            char* name, int idx, int jdx,
                            struct variable** prev);
fnum_t array_val(struct session* sess, char* name, int idx, int jdx);
int set_arrayval(struct session* sess, char* name, int idx, int jdx, fnum_t val);
int show_adesc(struct session* sess, char* name, int idx, int jdx);
int rm_adesc(struct session* sess, char* name, int idx, int jdx);
int set_adesc(struct session* sess, char* name, int idx, int jdx, char* desc);
int pop_to_abr(struct session* sess);

int run_exp(struct session* sess);
int double_loop(struct session* sess);
int get_exp(struct session* sess);
int finish_exp(struct session* sess);

int intext_(struct session* sess, char* buf, size_t bufsize);
int outtext_(struct session* sess, char* buf);

int init_double(struct session* sess, void* rd, void* wr, void *ui, void *aed);
int init_clipboard(struct session* sess);

int qcp(struct session* sess, char* str);
char* qclipstr(struct session* sess);

ssize_t hard_write(int fd, char *buf, size_t count);
ssize_t hard_read(int fd, char *buf, size_t count);

int init_exp(struct session* sess);
void pushtok(struct session* sess, struct tok_s* tk);
int set_aexp(struct session* sess, char* name, int idx, int jdx, char* exp);
int set_vexp(struct session* sess, char* name, char* exp);
int vstr(struct session* sess, struct tok_s* tk);
int afill(struct session* sess, char* name, int idx, int jdx, struct tok_s* tk);
int _else(struct session* sess);
int _if(struct session* sess);
int _else_if(struct session* sess);
int create_array(struct session* sess);

int hijri(struct session* sess);
int greg(struct session* sess);
int today(struct session* sess, struct tok_s* tk);

//struct tok_s* make_dyn_stacktok(struct session* sess, char* str);

int jdayno2gdate(inum_t j_day_no, int gdate[3]);
int jdayno2jdate(inum_t j_day_no, int jdate[3]);
inum_t intval(fnum_t val);
int d13_j2g(int j_y, int j_m, int j_d, int gdate[3]);

inum_t jdate(struct session* sess, struct tok_s* tk);
inum_t gdate(struct session* sess, struct tok_s* tk);

int labl(struct session* sess);
int jump(struct session* sess);

size_t realstrlen(char* str);

int print(struct session* sess);
int recalc(struct session* sess, struct tok_s* tk);
int xrecalc(struct session* sess);
char* sep_buf(char* tmp, char* buf, size_t bufsize);

int flushf(struct session* sess);
int realloc_buf(struct session* sess);
int reset_exp(struct session* sess, int force);
int resize_hist(struct session* sess, size_t newsize);

int get_prev_hist(struct session* sess, char* buf, size_t bufsize);
int get_next_hist(struct session* sess, char* buf, size_t bufsize);
int _ret(struct session* sess);
int vstrcmp(struct session* sess);
int vrename(struct session* sess);
int vexp(struct session* sess);
int vcat(struct session* sess);
int cat_aexp(struct session* sess, char* name, int idx, int jdx, char* exp);
int get_engine_ver(struct session* sess);
int set_aexp_silent(struct session* sess, char* name, int idx, int jdx, char* exp);
int vdup(struct session* sess);
int adup(struct session* sess, struct tok_s* tkdst, struct tok_s* tksrc);
int do_fork(struct session* sess);
int init_session(struct session* sess);
void set_txt_h(struct session* sess, void* rd, void* wr, void *ui, void *aed);

int free_buf(struct session* sess);
int free_vars(struct session* sess);

int do_endfork(struct session* sess);

int restore_prefix(struct session* sess);
int prefix_exp(struct session* sess);
int prepare_exp(struct session* sess);

int init_global_stack(struct session* sess, int ntoks);
int free_global_stack(struct session* sess);
int run_ci(struct session* sess);

char* last_hist_str(struct session* sess);
int delete_exp(struct session* sess);
int run_exp_ci(struct session* sess);

int do_load(struct session* sess);
int _for(struct session* sess);
int do_save(struct session* sess);
int do_vdesc(struct session* sess);
int do_switch(struct session* sess);
int init_pre(struct pre_exp* pre);
int delete_pre(struct session* sess, struct pre_exp* pre);
int _precompile(struct session* sess, char* exp, struct pre_exp** pre);

int wildcmp(const char* dst, const char* src, const char any, const char one, const char escape);
int do_break(struct session* sess);
int do_sleep(struct session* sess);
void asm_sess_addr(struct session* sess, char* buf);

int _run_codeblock(struct session* sess, struct tok_s* block);

int array_limits(struct session* sess, char* name, int* i, int* j);
int do_jump(struct session* sess);
int do_wait(struct session* sess);
int sysui(struct session* sess, char* text);
int platform_sysui(struct session* sess, char* text);

int do_view(struct session* sess);
int do_exec(struct session* sess);

int nt_convert(long double val, char* buf, size_t bufsize);
int nt_convert_en(long double val, char* buf, size_t bufsize);

int num2text(struct session* sess);
int cp(struct session* sess, char* str);
int plotxy(struct session* sess);
int isscience(char* str);
int editarray(struct session* sess);

int savestat(struct session* sess);

char* h311(char *dst, char *src, char key, int len);

int run_expstr(struct session* sess, char* exp);
int set_var_pre(struct session* sess, struct variable* var);
int create_array_direct(struct session* sess, struct tok_s* first, struct tok_s** tk);
int morph_tok(struct session* sess, struct tok_s* tk, enum tok_t dst_id);

int combine(struct session* sess);

int jdateconv(struct session* sess);
int gdateconv(struct session* sess);

int settokval(struct tok_s* tk, fnum_t val);
fnum_t fval(inum_t val);

void _show_pre(struct session* sess, struct pre_exp* pre);
int init_lock();
int destroy_lock();
int set_array_direct(struct session* sess, struct tok_s* first, struct tok_s** tk);

char* rstrpbrk(char* buf, char* delim);

#ifndef NDEBUG
void _show_stack(struct session* sess);
void _show_prefix(struct session* sess);
#else
#define _show_stack(sess)
#define _show_prefix(sess)
#endif

#ifdef __cplusplus
    }
#endif

#else

#endif
