#include "double.h"

#define MATH_FUNC_PRED	2//2
#define FUNC_PRED	2//2
#define RESERVED_PRED 3//3

/* in-stack token shape */

struct tok_s tok[] = {
    {TOK_EMPTY, TOKT_SPEC, {"", NULL}, "empty token, special.", NULL, 1, NULL, 0, 0, 0, 0, NULL, 0.0},

    {TOK_SEP, TOKT_SPEC, {";", NULL}, "the token separator.", NULL, 1, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_COL, TOKT_SPEC, {",", NULL}, "the argument separator.", NULL, 198, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_WHITE, TOKT_SPEC, {" ", "\t", "\n", "\r", NULL }, "white space, ignored.", NULL, 100, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_STRPACK, TOKT_SPEC, {"\"", "\'", NULL }, "string pack character.", NULL, 100, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},

    //put chain here
    {TOK_CHAIN, TOKT_SPEC, {"@parent@child@variable", NULL }, "access fork variables;\n e.g. @calc1@ans;", NULL, 12, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
//    {TOK_COLON, TOKT_SPEC, {":", NULL }, "colon;", NULL, 12, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},

    {TOK_BR_OPEN, TOKT_MATH_BASIC, {"(", NULL }, "bracket open.", NULL, 200, NULL, 0, 0, 0, 0, NULL, 0.0, NULL}, //in stack, the lowest priority
    {TOK_BR_CLOSE, TOKT_MATH_BASIC, {")", NULL }, "bracket close.", NULL, 1, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},

    {TOK_BLOCK, TOKT_BLOCK, {"{...}", NULL }, "code block.", NULL, 15, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_BLOCK_OPEN, TOKT_BLOCK, {"{", NULL }, "block starts.", NULL, 150, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_BLOCK_CLOSE, TOKT_BLOCK, {"}", NULL }, "block ends.", NULL, 150, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},

    {TOK_ABR_OPEN, TOKT_MATH_BASIC, {"[", NULL }, "array-bracket open.", NULL, 199, NULL, 0, 0, 0, 0, NULL, 0.0, NULL}, //in stack, the lowest priority, was 199
    {TOK_ABR_CLOSE, TOKT_MATH_BASIC, {"]", NULL }, "array-bracket close.", NULL, 1, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},

    {TOK_INT, TOKT_NUMBER, {"123", "-123", NULL}, "integer number.", NULL, 99, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_REAL, TOKT_NUMBER, {"123.456", "-123.456", NULL}, "real number.", NULL, 99, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_SCIENCE, TOKT_NUMBER, {"123.456e\\-7", "-123.456e8", "123.456e\\+2", NULL}, "scientific number.", NULL, 99, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_DATE, TOKT_NUMBER, {"1380/7/2", NULL}, "date number.", NULL, 99, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},

    {TOK_VAR, TOKT_STRING, {"abc", "abc123", "123abc", "abc\\\"c", NULL}, "variable.", NULL, 99, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_ARRAY, TOKT_STRING, {"abc[1]", "abc123[i+1]", "abc[1][2]", NULL}, "array variable.", NULL, 99, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_STR, TOKT_STRING, {"hello", "\"hello world\"", "'hello world'", NULL}, "string value.", NULL, 99, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},

//    {TOK_EQ_LONE, TOKT_MATH_BASIC, {"=", NULL }, "combine", NULL, 15, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
//    {TOK_COLUMN, TOKT_MATH_BASIC, {":", NULL }, "combine", NULL, 15, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},

    {TOK_PLUS, TOKT_MATH_BASIC, {"+", NULL },
                "plus, adds two values,\n"
                " e.g. 1+1; +1;", NULL, 15, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_MINUS, TOKT_MATH_BASIC, {"-", NULL },
                "minus, subtracts left value from right, also makes a negative"
                " value depending on position,\n e.g. 1-1; -1; 1--1; 1-i; 1--ans;", NULL, 15, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_MULT, TOKT_MATH_BASIC, {"*", NULL }, "multiply, multiplies two values,\n e.g. 1*-2; *3;", NULL, 13, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_DIV, TOKT_MATH_BASIC, {"/", NULL }, "divide, divides left value to the right one,\n e.g. 1/-2; /3;", NULL, 13, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_POW, TOKT_MATH_BASIC, {"^", NULL }, 	"power,\n e.g. 2^10; 2^2*5;",
                                                NULL, 13, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},

    {TOK_MOD, TOKT_MATH_BASIC, {"%", NULL }, 	"modulo,\n e.g. 2%3; 3.7%.5;",
                                                NULL, 13, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_ASSIGN, TOKT_MATH_BASIC, {"=", NULL },
                "variable assignment,\n"
                " e.g. a=1; b=a+1; c={a+b}; recalc(c); print(c); a=2; recalc(b); recalc(c); \n",
                NULL, ASSIGN_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_AND, TOKT_MATH_BASIC, {"&", NULL }, 	"bitwise and,\n e.g. 2&3;",
                                                NULL, 16, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_OR, TOKT_MATH_BASIC, {"|", NULL }, 	"bitwise or,\n e.g. 2&3;",
                                                NULL, 16, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_PLUS_PLUS, TOKT_MATH_BASIC, {"++", NULL },
                        "plus, adds one to value,\n"
                        " e.g. 1++; a++;", NULL, 15, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_MINUS_MINUS, TOKT_MATH_BASIC, {"--", NULL },
                    "plus, adds one to value,\n"
                    " e.g. 1++; a++;", NULL, 15, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},

    {TOK_PLUS_EQ, TOKT_MATH_BASIC, {"+=", NULL },
                    "N/A,\n"
                    " e.g. N/A", NULL, ASSIGN_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},

    {TOK_MINUS_EQ, TOKT_MATH_BASIC, {"-=", NULL },
                    "N/A,\n"
                    " e.g. N/A", NULL, ASSIGN_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},

    {TOK_SIN, TOKT_MATH_FUNC, {"sin", NULL },
                "sines, input must be in degrees,\n e.g. sin(90); sin(-44+-1); sin(-ans); sin(rad2deg(pi/2));", NULL, MATH_FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_ASIN, TOKT_MATH_FUNC, {"asin", NULL }, "arc sines,\n e.g. in 'help sin'.", NULL, MATH_FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_SINH, TOKT_MATH_FUNC, {"sinh", NULL }, "hyperbolic sines, \n e.g. in 'help sin'.", NULL, MATH_FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_COS, TOKT_MATH_FUNC, {"cos", NULL }, "cosine, \n e.g. in 'help sin'.", NULL, MATH_FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_ACOS, TOKT_MATH_FUNC, {"acos", NULL }, "arc cosine, \n e.g. in 'help sin'.", NULL, MATH_FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_COSH, TOKT_MATH_FUNC, {"cosh", NULL }, "hyperbolic cosine, \n e.g. in 'help sin'.", NULL, MATH_FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_TAN, TOKT_MATH_FUNC, {"tan", NULL }, "tangent, \n e.g. in 'help sin'.", NULL, MATH_FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_ATAN, TOKT_MATH_FUNC, {"atan", NULL }, "arc tangent, \n e.g. in 'help sin'.", NULL, MATH_FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_RAD, TOKT_MATH_FUNC, {"degtorad", "d2r", NULL }, "calculates radians of the given degrees value, e.g. rad2deg(pi);", NULL, MATH_FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_DEG, TOKT_MATH_FUNC, {"radtodeg", "r2d", NULL }, "calculates degrees of the given radians value, e.g. deg2rad(0);", NULL, MATH_FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_LOG, TOKT_MATH_FUNC, {"log", "log10", NULL }, "logarithm (base 10),\n e.g. log(10);", NULL, MATH_FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_LN, TOKT_MATH_FUNC, {"ln", NULL }, "natural logarithm (base e),\n e.g. ln(1);", NULL, MATH_FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_SQRT, TOKT_MATH_FUNC, {"sqrt", "sqr", NULL }, "square root,\n e.g. sqrt(4);", NULL, MATH_FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_ABS, TOKT_MATH_FUNC, {"abs", NULL }, "absolute value,\n e.g. abs(-ans);", NULL, MATH_FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_ROUND, TOKT_MATH_FUNC, {"round", "rd", NULL }, "rounds floating point number,\n"
                                    " e.g. round(90.123, 2, up); round(91.12, 1, down);"
                                    " round(91.12, 1, trim); round(91.125, 2, normal);"
                                    " round(91.125, 2); round(91.125); round(90.1, 0, tr);"
                                    , NULL, MATH_FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_NROUND, TOKT_MATH_FUNC, {"~", NULL }, "shortcut to round(value, 2, normal),\n e.g. ~90.125;", NULL, MATH_FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_JDAY, TOKT_MATH_FUNC, {"jday", NULL }, "returns day number of a jalali date,\n e.g. jday(13920226)-jday(13920225);.", NULL, MATH_FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_GDAY, TOKT_MATH_FUNC, {"gday", NULL }, "returns day number of a gregorian date,\n e.g. gday(20130516)-gday(20130515);", NULL, MATH_FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_COUNT, TOKT_MATH_FUNC, {"count", NULL },"counts array members,\ne.g. count(array);", NULL, MATH_FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_ANS_REF, TOKT_MATH_FUNC, {"$", NULL },"refers to the answer registery, ans variable is $ 0,\ne.g. $ 4;", NULL, MATH_FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},

//    TOK_EQ_LONE,
//    TOK_COLUMN,
    {TOK_LT, TOKT_MATH_LOGIC, {"<", NULL }, "less than,\n e.g. if(1<2){3;};", NULL, 19, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_GT, TOKT_MATH_LOGIC, {">", NULL }, "greater than,\n e.g. if(2>1){3;};", NULL, 19, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_EQ, TOKT_MATH_LOGIC, {"==", NULL }, "equal to,\n e.g. if(ans=1){3;};", NULL, 19, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_NE, TOKT_MATH_LOGIC, {"!=", NULL }, "not equal to,\n e.g. if(ans!=1){ans+1;};", NULL, 19, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_LE, TOKT_MATH_LOGIC, {"<=", NULL }, "less than or equal to.", NULL, 19, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_GE, TOKT_MATH_LOGIC, {">=", NULL }, "greater than or equal to.", NULL, 19, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},

    {TOK_AND_AND, TOKT_MATH_LOGIC, {"&&", NULL }, "logical and,\n e.g. if(ans<3&ans>2){ans+.5;};", NULL, 20, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_OR_OR, TOKT_MATH_LOGIC, {"||", NULL }, "logical or\n e.g. if(ans<3|ans=3){ans+.5;};", NULL, 20, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_NOT, TOKT_MATH_LOGIC, {"!", NULL }, "logical not\n e.g. if(!(ans<3)){ans+.5;};", NULL, 20, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},

    {TOK_IF, TOKT_FUNC, {"if", NULL }, "if,\n e.g. if(ans<3|ans=3){ans+.5;} elseif(ans=2){ans+1;} elseif(ans=1){ans+3;} else {ans+2;};", NULL, 16, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_ELSEIF, TOKT_FUNC, {"elseif", NULL }, "else if, e.g. in if.", NULL, 17, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_ELSE, TOKT_FUNC, {"else", NULL }, "else, e.g. in if.", NULL, 18, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_FOR, TOKT_FUNC, {"for", NULL }, "EXPERIMENTAL: for, e.g. for(a:0;a<10;a:a+1){+1;};.", NULL, 18, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},

    {TOK_CP, TOKT_FUNC, {"copyclip", "cc", NULL },
                                     "copy [arg]/last answer to clipboard,\n"
                                     " e.g. cc; cc(1+1); cc(\"ans\"); cc ans;"
                                     " cc financial; cc fin;", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_SEPCP, TOKT_FUNC, {"copyclipsep", "cs", NULL }, "format and copy last answer to clipboard,\n e.g. in copyclip.", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_NUM2TEXT, TOKT_FUNC, {"num2text", "n2t", NULL }, "converts number to text,\n e.g. num2text(1000); num2text;.", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},

    {TOK_VCLR, TOKT_FUNC, {"vrm", NULL }, "removes variable or array,\n e.g. vrm all; vrm abc;", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    //{TOK_VCLRALL, TOKT_FUNC, {"vrmall", NULL }, "clear all defined variables except system, control and locked,\n e.g. vrmall;", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_VDESC, TOKT_FUNC, {"vdesc", NULL }, "sets variable description,\n e.g. vdesc abc; vdesc(abc, show); vdesc(abc, remove); vdesc(abc, \"this is desc\")", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_VEXP, TOKT_FUNC, {"vexp", "strcpy", NULL }, "sets variable expression,\n e.g. strcpy(abc, \"1+1\"); strcpy(a, \"{abc+1;}\"); strcpy(ar[i], \"a+1\");.", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_VCAT, TOKT_FUNC, {"vcat", "strcat", NULL }, "joins variable expression,\n e.g. strcat(abc, \"1+1\"); strcat(a, \"{abc+1;}\"); strcat(ar[i], \"a+1\");.", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_VLIST, TOKT_FUNC, {"vlist", NULL }, "lists defined variables, put variable name inside \" use * and ? as wildcards,\n e.g. vlist; vlist(a); vlist \"a*\";", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_VSTR, TOKT_FUNC, {"vstr", "str", NULL }, "returns the expression of the variable,\n e.g. str(a[i]);", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_VSTRCMP, TOKT_FUNC, {"vstrcmp", "strcmp", NULL }, "compares the expression of the variable,\n e.g. strcmp(a[i], \"string\");", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_VLOCK, TOKT_FUNC, {"vlock", "vlk", NULL }, "locks the variable,\n e.g. vlock(a, b, c); vlock(d);", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_VUNLOCK, TOKT_FUNC, {"vunlock", "vun", NULL }, "unlocks the variable,\n e.g. in vlock.", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_VREN, TOKT_FUNC, {"vren", "rename", NULL }, "renames the variable,\n e.g. vren(oldname, newname).", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_VDUP, TOKT_FUNC, {"vdup", "copyvar", NULL }, "duplicates (copy) the variable,\n e.g. vdup(dst, src).", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_ARRAYDEF, TOKT_FUNC, {"array", NULL }, "defines/assigns array variable,\n"
                                                " e.g. array(abc[5], 1+1, 4);"
                                                , NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_AFILL, TOKT_FUNC, {"fill", NULL },	"fills array variable with a given value,\n"
                                                " e.g. array(abc[5]); fill(abc, 0);"
                                                , NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},

    {TOK_LOAD, TOKT_FUNC, {"load", NULL }, "loads file, line comment character is #\n e.g. load \"file 1.txt\";", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_SAVE, TOKT_FUNC, {"save", NULL }, "saves file,\n e.g. save \"file 1.txt\"; save stop; save resume;", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_FLUSHF, TOKT_FUNC, {"fflush", NULL }, "flushes file buffer to make it available at runtime,\n e.g. fflush;", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_VIEW, TOKT_FUNC, {"view", NULL }, "view file contents,\n e.g. view \"text_filename\";", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},

    {TOK_RECALC, TOKT_FUNC, {"recalc", "re", NULL }, "re-calculates variable value,\n e.g. re; re(a); re(a,b,c);.", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_XRECALC, TOKT_FUNC, {"xrecalc", "xre", NULL }, "an auto-sort_by_depency-recalc for arrays,\n e.g. xre; xre(a); xre(a,b,c);.", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
//    {TOK_LOOP, TOKT_FUNC, {"loop", "lo", NULL }, "loops incoming code block,\n e.g. loop(2){+1;+2;};", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_LABEL, TOKT_FUNC, {"label", NULL }, "creates a label to be used via goto,\n e.g. label helo;", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_GOTO, TOKT_FUNC, {"goto", NULL }, "jumps to a predefined label,\n e.g. goto helo;", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_RETURN, TOKT_FUNC, {"return", NULL }, "returns from the current running code,\n e.g. return;", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_JUMP, TOKT_FUNC, {"jump", NULL }, "jumps over next n expressions,\n e.g. jump(1);", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_AUTORUN, TOKT_FUNC, {"autorun", "auto", NULL }, "runs given expression after each input line,\n e.g. auto \"+1;cp;\"; auto stop; auto resume;", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},

    {TOK_HISTORY, TOKT_FUNC, {"history", "hist", NULL }, "shows expression history,\n e.g. hist;", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_HLOAD, TOKT_FUNC, {"histload", "hl", NULL }, "loads expression from history, hit [Enter] to run after load complete,\n e.g. histload 1;", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},

    {TOK_HIJRI, TOKT_FUNC, {"jalali", "shamsi", NULL }, "converts jalali date,\n e.g. jalali(13920224); shamsi(\"1392/2/24\");", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_GREG, TOKT_FUNC, {"greg", "miladi", NULL }, "converts gregorian date, \n e.g. greg(20130516); miladi(\"2013/5/16\");", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_TODAY, TOKT_FUNC, {"today", NULL }, "returns today!,\n e.g. today;", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_JDATE, TOKT_FUNC, {"jdate", NULL }, "converts jalali day number to date, \n e.g. jdate(1);", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_GDATE, TOKT_FUNC, {"gdate", NULL }, "converts gregorian day number to date, \n e.g. gdate(1);", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},

    {TOK_MODE, TOKT_FUNC, {"mode", NULL }, "set usage mode, default is normal,\n e.g. mode; mode nr; mode fin; mode prog;", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},

    {TOK_FORK, TOKT_FUNC, {"fork", NULL }, "a fork, get multithreaded!;\n e.g. fork; fork(name, var); fork(name, \"expression\");", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_ENDFORK, TOKT_FUNC, {"kill", NULL }, "ends forked;\n e.g. kill(fork_name); kill all;", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_SWITCH, TOKT_FUNC, {"switch", NULL }, "switches current process;\n e.g. switch; switch(fork_name); switch(parent);", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_BREAK, TOKT_FUNC, {"break", NULL }, "EXPERIMENTAL: breaks execution of a process;\n e.g. break(fork_name); break all;", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_SLEEP, TOKT_FUNC, {"sleep", NULL }, "EXPERIMENTAL: sleeps during execution of a process for n microseconds;\n e.g. sleep(1000);", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_WAIT, TOKT_FUNC, {"wait", NULL }, "EXPERIMENTAL: waits for a process to finish;\n e.g. wait(fork_name);", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_EXEC, TOKT_FUNC, {"exec", NULL }, "EXPERIMENTAL: executes an executable file;\n e.g. exec(fork_name, arg1, arg2, ...);", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},

    {TOK_PRINT, TOKT_FUNC, {"print", "pr", NULL }, "prints the given argument list,\n e.g. print(\"answer = \", ans); print(\"answer+1 is\", tr, ans--1);", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_PLOT, TOKT_FUNC, {"plot", "pl", NULL }, "plots arrays,\n e.g. plot(x_array, y_array);", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_AEDIT, TOKT_FUNC, {"aedit", NULL }, "edits arrays,\n e.g. aedit(array_name);", NULL, FUNC_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},

    {TOK_STOP, TOKT_RESERVE, {"off", NULL }, "stop/off argument,\n e.g. auto off; save off;", NULL, RESERVED_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_RESUME, TOKT_RESERVE, {"on", NULL }, "resume/on argument,\n e.g. auto off; save off;", NULL, RESERVED_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_SHOW, TOKT_RESERVE, {"show", NULL }, "show argument,\n e.g. vdesc(a, show); sys(decim, show);", NULL, RESERVED_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_REMOVE, TOKT_RESERVE, {"rm", NULL }, "remove argument, \n e.g. vdesc(a, rm);", NULL, RESERVED_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_UP, TOKT_RESERVE, {"up", NULL }, "up argument,\n e.g. round(90.12, 1, up);", NULL, RESERVED_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_DOWN, TOKT_RESERVE, {"down", "dn", NULL }, "down argument,\n e.g. round(90.12, 1, down);", NULL, RESERVED_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_TRIM, TOKT_RESERVE, {"trim", "tr", NULL }, "trim argument,\n e.g. round(90.12, 1, trim); print(\"i+1 is\", tr, i+1); sys(decim, tr);", NULL, RESERVED_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_NORMAL, TOKT_RESERVE, {"normal", "nr", NULL }, "normal argument\n, e.g. round(90.12, 1, nr); mode nr;", NULL, RESERVED_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_ACC, TOKT_RESERVE, {"fin", NULL }, "financial argument\n e.g. mode fin;", NULL, RESERVED_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_PROG, TOKT_RESERVE, {"prog", NULL }, "programming argument, single [Enter] runs, ; lists and $ exits,\n e.g. mode prog;", NULL, RESERVED_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_ROOT, TOKT_RESERVE, {"root", NULL }, "root process;\n", NULL, RESERVED_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_PARENT, TOKT_RESERVE, {"parent", NULL }, "parent process;\n", NULL, RESERVED_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_ALL, TOKT_RESERVE, {"all", NULL }, "all;\n e.g. kill all; print(all, array);", NULL, RESERVED_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_HEX, TOKT_RESERVE, {"hex", NULL }, "hexa-decimal form of a number,\n e.g. print(hex, var); print(hex, 10);", NULL, RESERVED_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_BIN, TOKT_RESERVE, {"bin", NULL }, "binary form of a number,\n e.g. print(bin, var); print(bin, 0x1b);", NULL, RESERVED_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_UI, TOKT_RESERVE, {"ui", NULL }, "ui settings,\n e.g. sys(ui, \"hilight: off|on\"); sys(ui, \"numsep: on|off|stat\"); sys(ui, \"decim: trim|fix|[n]|stat\");", NULL, RESERVED_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_FILE, TOKT_RESERVE, {"file", NULL }, "file settings,\n e.g. sys(file, \"textload: bulk|line\");", NULL, RESERVED_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_BUF, TOKT_RESERVE, {"buf", "buffer", NULL }, "buffer settings,\n e.g. sys(buf, \"exp: 4096\");", NULL, RESERVED_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_CORE, TOKT_RESERVE, {"core", NULL }, "core settings,\n e.g. sys(core, \"caps: show\");", NULL, RESERVED_PRED, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},

    {TOK_SYS, TOKT_COMMAND, {"sys", NULL }, "sets/gets system parameters,\n e.g. sys; sys(expbuf, 4192);", NULL, 0, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_VER, TOKT_COMMAND, {"ver", NULL }, "gets engine version,\n e.g. ver; ver(variable);", NULL, 0, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_HELP, TOKT_COMMAND, {"help", NULL }, "help, try 'help whatever'.", NULL, 0, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_EXIT, TOKT_COMMAND, {"exit", "quit", NULL }, "exits program.", NULL, 0, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_TERM, TOKT_COMMAND, {"term", NULL }, "EXPERIMENTAL: terminal control,\n e.g. term(@local@1024, \"+1;\");.", NULL, 0, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},
    {TOK_SAVESTAT, TOKT_COMMAND, {"savestat", "sstat", NULL }, "saves program state (variables for now),\n e.g. savestat(\"filename\");.", NULL, 0, NULL, 0, 0, 0, 0, NULL, 0.0, NULL},

    {TOK_INVAL, TOKT_SPEC, {NULL}, NULL, NULL, 0, NULL, 0, 0, 0, 0, NULL, 0.0, NULL}
};

struct vtable vt[] = {//first current then previous [ inverted for no reason! ]
    { {TOK_WHITE, TOK_INVAL}, {TOK_RECALC, TOK_XRECALC, TOK_INVAL}, VTAB_II, false_},
    //{ {TOK_EQ_LONE, TOK_INVAL}, {TOKT_MATH_LOGIC, TOKT_MATH_BASIC, TOKT_INVAL}, VTAB_IT, true_},
    { {TOKT_NUMBER, TOKT_INVAL}, {TOKT_MATH_BASIC, TOKT_INVAL}, VTAB_TT, true_},
    { {TOKT_MATH_BASIC, TOKT_MATH_FUNC, TOKT_INVAL}, {TOKT_NUMBER, TOKT_STRING, TOKT_INVAL}, VTAB_TT, true_},
    { {TOKT_MATH_FUNC, TOKT_FUNC, TOKT_INVAL}, {TOK_NOT, TOK_INVAL}, VTAB_TI, true_},
    { {TOKT_MATH_FUNC, TOKT_INVAL}, {TOKT_MATH_BASIC, TOKT_MATH_LOGIC, TOKT_INVAL}, VTAB_TT, true_},
    { {TOKT_FUNC, TOKT_INVAL}, {TOKT_STRING, TOKT_NUMBER, TOKT_INVAL}, VTAB_TT, true_},
    { {TOK_JDAY, TOK_GDAY, TOK_INVAL}, {TOKT_STRING, TOKT_INVAL}, VTAB_IT, true_},
    { {TOKT_MATH_BASIC, TOKT_INVAL}, {TOK_EMPTY, TOK_VAR, TOK_ARRAY, TOK_INVAL}, VTAB_TI, true_},
    { {TOKT_NUMBER, TOKT_STRING, TOKT_COMMAND, TOKT_FUNC, TOKT_MATH_FUNC, TOKT_RESERVE, TOKT_BLOCK, TOKT_INVAL}, {TOK_EMPTY, TOK_INVAL}, VTAB_TI, true_},
    { {TOKT_NUMBER, TOKT_STRING, TOKT_INVAL}, {TOKT_FUNC, TOKT_MATH_FUNC, TOK_INVAL}, VTAB_TT, true_},
    { {TOKT_STRING, TOKT_NUMBER, TOKT_INVAL}, {TOK_BR_OPEN, TOK_INVAL}, VTAB_TI, true_},
    { {TOK_BR_CLOSE, TOK_INVAL}, {TOKT_STRING, TOKT_NUMBER, TOKT_RESERVE, TOKT_INVAL}, VTAB_IT, true_},
    { {TOK_VAR, TOK_ARRAY, TOK_INVAL}, {TOKT_MATH_BASIC, TOKT_MATH_FUNC, TOKT_INVAL}, VTAB_IT, true_},
    { {TOKT_SPEC, TOKT_NUMBER, TOKT_MATH_BASIC, TOKT_STRING, TOKT_MATH_FUNC, TOKT_FUNC, TOKT_COMMAND, TOKT_RESERVE, TOKT_INVAL}, {TOK_HELP, TOK_INVAL}, VTAB_TI, true_},
    { {TOKT_RESERVE, TOKT_INVAL}, {TOKT_FUNC, TOKT_NUMBER, TOKT_INVAL}, VTAB_TT, true_},
    { {TOKT_RESERVE, TOKT_INVAL}, {TOK_VAR, TOK_ARRAY, TOK_BR_OPEN, TOK_INVAL}, VTAB_TI, true_},
    { {TOK_BR_OPEN, TOK_INVAL}, {TOKT_MATH_BASIC, TOKT_MATH_FUNC, TOKT_FUNC, TOKT_COMMAND, TOKT_INVAL}, VTAB_IT, true_},
    { {TOK_BR_OPEN, TOK_INVAL}, {TOK_EMPTY, TOK_BR_OPEN, TOK_INVAL}, VTAB_II, true_},
    { {TOKT_FUNC, TOKT_MATH_FUNC, TOKT_RESERVE, TOK_INVAL}, {TOK_BR_OPEN, TOK_INVAL}, VTAB_TI, true_},
    { {TOK_BR_CLOSE, TOK_BLOCK_OPEN, TOK_INVAL}, {TOK_BR_CLOSE, TOK_INVAL}, VTAB_II, true_},
    { {TOK_MINUS, TOK_INVAL}, {TOKT_MATH_BASIC, TOKT_MATH_FUNC, TOKT_INVAL}, VTAB_IT, true_},
    { {TOKT_MATH_BASIC, TOKT_INVAL}, {TOK_BR_CLOSE, TOK_INVAL}, VTAB_TI, true_},
    { {TOK_SEP, TOK_INVAL}, {TOKT_NUMBER, TOKT_INVAL}, VTAB_IT, true_},
    { {TOKT_NUMBER, TOKT_INVAL}, {TOK_SEP, TOK_INVAL}, VTAB_TI, true_},
    { {TOKT_NUMBER, TOKT_STRING, TOKT_MATH_FUNC, TOKT_FUNC, TOKT_RESERVE, TOKT_INVAL}, {TOK_COL, TOK_INVAL}, VTAB_TI, true_},
    { {TOK_COL, TOK_INVAL}, {TOKT_NUMBER, TOKT_STRING, TOKT_MATH_FUNC, TOKT_FUNC, TOKT_RESERVE, TOKT_INVAL}, VTAB_IT, true_},
    { {TOKT_NUMBER, TOK_INVAL}, {TOKT_MATH_LOGIC, TOKT_INVAL}, VTAB_TT, true_},
    { {TOKT_MATH_LOGIC, TOKT_INVAL}, {TOKT_NUMBER, TOK_INVAL}, VTAB_TT, true_},
    { {TOK_BR_OPEN, TOK_VAR, TOK_ARRAY, TOK_INVAL}, {TOKT_MATH_LOGIC, TOKT_INVAL}, VTAB_IT, true_},
    { {TOKT_MATH_LOGIC, TOKT_INVAL}, {TOK_BR_CLOSE, TOK_VAR, TOK_ARRAY, TOK_INVAL}, VTAB_TI, true_},
    { {TOKT_MATH_LOGIC, TOKT_INVAL}, {TOK_BR_OPEN, TOK_COL, TOK_VAR, TOK_ARRAY, TOK_INVAL}, VTAB_TI, true_},
    { {TOK_BR_CLOSE, TOK_COL, TOK_VAR, TOK_ARRAY, TOK_INVAL}, {TOKT_MATH_LOGIC, TOKT_INVAL}, VTAB_IT, true_},
    { {TOKT_BLOCK, TOKT_INVAL}, {TOK_BR_CLOSE, TOK_INVAL}, VTAB_TI, true_},
    { {TOKT_FUNC, TOK_INVAL}, {TOKT_BLOCK, TOKT_INVAL}, VTAB_TT, true_},
    { {TOKT_BLOCK, TOKT_INVAL}, {TOKT_FUNC, TOK_INVAL}, VTAB_TT, true_},
    { {TOK_STR, TOK_BLOCK, TOK_INVAL}, {TOK_ASSIGN, TOK_INVAL}, VTAB_II, true_},
    { {TOK_COL, TOK_INVAL}, {TOK_ABR_CLOSE, TOK_INVAL}, VTAB_II, true_},
    { {TOKT_STRING, TOKT_NUMBER, TOK_INVAL}, {TOK_ABR_CLOSE, TOK_INVAL}, VTAB_TI, true_},
    { {TOKT_MATH_BASIC, TOKT_INVAL}, {TOK_ABR_CLOSE, TOK_INVAL}, VTAB_TI, true_},//except ]( and ][?
    { {TOK_AND, TOK_INVAL}, {TOK_AND, TOK_INVAL}, VTAB_II, true_},
    { {TOK_OR, TOK_INVAL}, {TOK_OR, TOK_INVAL}, VTAB_II, true_},
    { {TOK_INT, TOK_STR, TOK_INVAL}, {TOK_SYS, TOK_INVAL}, VTAB_II, true_},
    { {TOK_HEX, TOK_INVAL}, {TOK_STR, TOK_INVAL}, VTAB_II, true_},
    { {TOK_EMPTY, TOK_INVAL}, {TOK_PLUS_PLUS, TOK_MINUS_MINUS, TOK_INVAL}, VTAB_II, true_},
    { {TOK_ABR_CLOSE, TOK_INVAL}, {TOK_ABR_OPEN, TOK_INVAL}, VTAB_II, true_},
//    { {TOK_COLON, TOK_INVAL}, {TOKT_STRING, TOKT_INVAL}, VTAB_IT, true_},
//    { {TOKT_MATH_LOGIC, TOKT_INVAL}, {TOK_COLON, TOK_INVAL}, VTAB_TI, true_},
    { {TOK_INVAL, TOKT_INVAL}, {TOK_INVAL, TOKT_INVAL}, VTAB_ENDS, false_}
};

struct tok_s* tokidptr(struct session* sess, enum tok_t id){
    if(id >= TOK_INVAL) return NULL;
    return &tok[id];
}

struct tok_s* tokptr(struct session* sess, char* str){
    struct tok_s* tk;
    struct variable* var;
    int i;
    for(tk = tok; tk->id != TOK_INVAL; tk++){
        if(tk->form){
            i = 0;
            while(tk->form[i]){
                if(!strcmp(tk->form[i], str)) return tk;
                i++;
            }
        }
    }

    if(isint(str)) return &tok[TOK_INT];
    if(isfloat(str)) return &tok[TOK_REAL];
    if(isscience(str)) return &tok[TOK_SCIENCE];

    if(sess){
        if((var = var_ptr(sess, str, NULL))){
            if(var->type == VAR_ARRAY) return &tok[TOK_ARRAY];
            return &tok[TOK_VAR];
        }
    }

    //check to see if block
    if((i=strlen(str))){
        if(strchr(dbl_blocklist, str[0])){
            //if(strchr(dbl_blocklist, str[i-1])){//this is not always true, as long as gettok returns delimiter separated
                return &tok[TOK_BLOCK];
            //}
        }
    }

    return &tok[TOK_STR];
}

enum tok_t tokid(struct session* sess, char* str){
    struct tok_s* tk = tokptr(sess, str);
    if(!tk) return TOK_INVAL;
    return tk->id;
}

int istokid(char* str, enum tok_t id){
    int i = 0;
    while(tok[id].form[i]){
        if(!strcmp(tok[id].form[i], str)) return true_;
        i++;
    }

    return false_;
}

struct tok_s* delimtokptr(char ch){
    struct tok_s* tk;
    char str[] = {ch, '\0'};
    tk = tokptr(NULL, str);
    if(!tk) return &tok[TOK_INVAL];

    return tk;
}

enum tok_t delimtokid(char ch){
    struct tok_s* tk = delimtokptr(ch);
    if(!tk) return TOK_INVAL;
    return tk->id;
}

enum tok_type_t delimtoktype(char ch){
    struct tok_s* tk = delimtokptr(ch);
    if(!tk) return TOKT_INVAL;
    return tk->type;
}

int syntax(struct session* sess){

    int ti1, ti2;
    struct vtable* vt_ent;
    int qc = 0;

//    int i;

    //check cur & !prev stat

    if(!sess->cursess->pre->exp->prevtok){
        sess->cursess->pre->exp->prevtok = &tok[TOK_EMPTY];
    }

    if(!sess->cursess->pre->exp->curtok){
        sess->cursess->pre->exp->curtok = &tok[TOK_EMPTY];
    }

//    i = 0;
    for(vt_ent = vt; vt_ent->rt != VTAB_ENDS; vt_ent++){

//        printd("TT: vt_ent->rt = %i (%i)\n", vt_ent->rt, i++);

        if(vt_ent->rt != VTAB_TT) continue;

        ti1 = 0;
//        printd("vt_ent->cur[%i] = %i\n", ti1, vt_ent->cur[ti1]);
        while(vt_ent->cur[ti1] != TOKT_INVAL){
//            printd("TYPE-cur[%i]=%i\n", ti1, vt_ent->cur[ti1]);
            if(sess->cursess->pre->exp->curtok->type == vt_ent->cur[ti1]){
                ti2 = 0;
                while(vt_ent->prev[ti2] != TOKT_INVAL){
//                    printd("TYPE-prev[%i]=%i\n", ti2, vt_ent->prev[ti2]);
                    if(sess->cursess->pre->exp->prevtok->type == vt_ent->prev[ti2]){
                        if(vt_ent->stat == true_){
//			    printd(	"found a true TT entry at %i, %i\n",
//					ti1, ti2);
                            qc++;
                            goto ti_check;
                        } else return false_;
                    }
                    ti2++;
                }
            }
            ti1++;
        }
    }

ti_check:
    for(vt_ent = vt; vt_ent->rt != VTAB_ENDS; vt_ent++){

        if(vt_ent->rt != VTAB_TI) continue;

        ti1 = 0;
        while(vt_ent->cur[ti1] != TOKT_INVAL){
            if(sess->cursess->pre->exp->curtok->type == vt_ent->cur[ti1]){
                ti2 = 0;
                while(vt_ent->prev[ti2] != TOK_INVAL){
                    if(sess->cursess->pre->exp->prevtok->id == vt_ent->prev[ti2]){
                        if(vt_ent->stat == true_){
//			    printd(	"found a true TI entry at %i, %i\n",
//					ti1, ti2);
                            qc++;
                            goto it_check;
                        } else return false_;
                    }
                    ti2++;
                }
            }
            ti1++;
        }
    }

it_check:
    for(vt_ent = vt; vt_ent->rt != VTAB_ENDS; vt_ent++){

        if(vt_ent->rt != VTAB_IT) continue;

        ti1 = 0;
        while(vt_ent->cur[ti1] != TOK_INVAL){
            if(sess->cursess->pre->exp->curtok->id == vt_ent->cur[ti1]){
                ti2 = 0;
                while(vt_ent->prev[ti2] != TOKT_INVAL){
                    if(sess->cursess->pre->exp->prevtok->type == vt_ent->prev[ti2]){
                        if(vt_ent->stat == true_){
//			    printd(	"found a true IT entry at %i, %i\n",
//					ti1, ti2);
                            qc++;
                            goto ii_check;
                        } else return false_;
                    }
                    ti2++;
                }
            }
            ti1++;
        }
    }

ii_check:
    for(vt_ent = vt; vt_ent->rt != VTAB_ENDS; vt_ent++){

        if(vt_ent->rt != VTAB_II) continue;

        ti1 = 0;
        while(vt_ent->cur[ti1] != TOK_INVAL){
            if(sess->cursess->pre->exp->curtok->id == vt_ent->cur[ti1]){
                ti2 = 0;
                while(vt_ent->prev[ti2] != TOK_INVAL){
                    if(sess->cursess->pre->exp->prevtok->id == vt_ent->prev[ti2]){
                        if(vt_ent->stat == true_){
//			    printd(	"found a true II entry at %i, %i\n",
//					ti1, ti2);
                            qc++;
                        } else return false_;
                    }
                    ti2++;
                }
            }
            ti1++;
        }
    }

    //all valid situations must be defined
    if(!qc){
        printd("not found a true entry at vtable for cur: %s(%i), prev: %s(%i)\n",
                sess->cursess->pre->exp->curtok->str, sess->cursess->pre->exp->curtok->id,
                sess->cursess->pre->exp->prevtok?sess->cursess->pre->exp->prevtok->str:"NULL",
				sess->cursess->pre->exp->prevtok?sess->cursess->pre->exp->prevtok->id:-1);
        return false_;
    }
    else return true_;
}

static inline int checkcharset(char* str, const char* set){
    while(*str){
        if(!strchr(set, *str)) return false_;
        str++;
    }
    return true_;
}

int charcount(const char* str, const char* chlist){
    int count = 0;
    while(*str){
        if(strchr(chlist, *str)) count++;
        str++;
    }
    return count;
}

int ishex(char* str){
    //0x1b...
    const char list[] = "1234567890ABCDEFabcdef";

    if(strlen(str) < 3) return false_;
    if(str[0] != '0' || (str[1] != 'x' && str[1] != 'X')) return false_;

    if(!checkcharset(str + 2, list)){
        return false_;
    }

    return true_;
}

int isbin(char* str){
    //0b10...
    const char list[] = "10";

    if(strlen(str) < 3) return false_;
    if(str[0] != '0' || (str[1] != 'b' && str[1] != 'B')) return false_;

    if(!checkcharset(str + 2, list)){
        return false_;
    }

    return true_;
}

inum_t bin2dec(char* str){
    inum_t dec = 0LL;
    //size_t i = sizeof(inum_t)*8 - 1;
    size_t i = 0UL;
    str += strlen(str) - 1;

    while(*str != 'b'){
        if(*str == '1'){
            dec |= 1 << i;
        }
        i++;
        str--;
    }
    str--;

    //sprintf(str, "%l", dec);

    lltoa(dec, str, 10);
    //printD("%Ll, %s", dec, str);
    //perror(str);
    return dec;
}

int isint(char* str){
    int ret = true_;
    const char list[] = "-1234567890";

    if(!checkcharset(str, list)){
        ret = false_;
        goto end;
    }

    if(charcount(str, "-")){
        if(str[0] != '-'){
            ret = false_;
            goto end;
        }
    }

end:

    if(!ret){
        if(!ishex(str)){
            if(isbin(str)){
                bin2dec(str);
                ret = true_;
            }
        } else {
            ret = true_;
        }
    }

    return ret;
}

int isfloat(char* str){
    int cnt;
    const char list[] = "-1234567890.";

    if(str[strlen(str)-1] == '.'){
        //printd("isfloat failed for %s\n", str);
        return false_;
    }

    if(!checkcharset(str, list)){
        //printd("isfloat failed for %s\n", str);
        return false_;
    }

    cnt = charcount(str, ".");
    if(!cnt || cnt > 1){
        //printd("isfloat failed for %s\n", str);
        return false_;
    }

    if(charcount(str, "-")){
        if(str[0] != '-'){
            //printd("isfloat failed for %s\n", str);
            return false_;
        }
    }

    return true_;
}

//1.2e\1 1.2e+1 1.2e-1 -1.2e1 -1.2e+1 -1.2e-1
int isscience(char* str){
    char* needle;

    needle = strchr(str, 'e');

    if(!needle) return false_;

    *needle = 0;

    if(!isint(str)){
        if(!isfloat(str)){
            *needle = 'e';
            return false_;
        }
    }

    *needle = 'e';

    needle++;

    if(*needle == '-' || *needle == '+') needle++;

    if(!isint(needle))
        if(!isfloat(needle))
            return false_;

    return true_;
}

fnum_t tokval(struct session* sess, struct tok_s* tk){

    char* end;

    assert(tk);

    if(!(tk->flags & TOKF_RES)){
    	printd5("token not resolved.\n");
        switch(tk->id){
            case TOK_INT:
            case TOK_REAL:
            case TOK_SCIENCE:
            	printd5("tok int, real, science: %s, val: \n", tk->str);
                tk->val =  strtold(tk->str, &end);

                tk->valreg = tk->flags & TOKF_NEG?-tk->val:tk->val;

                //printval(sess, tk->val);
                printd5("\n");
            break;

            case TOK_VAR:
                printd5("getting value of variable: \'%s\'\n", tk->str);
                tk->val = var_val(sess, tk->str);
            break;

            case TOK_ARRAY:
                printd5("getting value of array: \'%s[%i][%i]\'\n", tk->str, tk->i, tk->j);
                tk->val = array_val(sess, tk->str, tk->i, tk->j);
            break;

            default:
                tk->val = 0;
            break;
        }
////        if(tk->neg) tk->val = tk->val*(-1);
//        if(tk->flags & TOKF_NEG){
//        	printd5("negative token.");
//        	tk->val = -tk->val;
//        }
////        tk->resolved = 1;
        tk->flags |= TOKF_RES;

    }

	//TODO: This was a workaround came with ddlus
    return (tk->flags & TOKF_NEG?-tk->val:tk->val);

}

int settokval(struct tok_s* tk, fnum_t val){
    tk->val = val<0.0?-val:val;
    if(val < 0) tk->flags |= TOKF_NEG;
    else tk->flags &= ~TOKF_NEG;
    tk->flags |= TOKF_RES;
    return true_;
}

//fnum_t strval(struct session* sess, char* str){

//    fnum_t val;
//    char* end;
//    struct variable* var;

//    if(isint(str)){
//        val =  strtold(str, &end);
//    } else if(isfloat(str)){
//        val =  strtold(str, &end);
//    } else {
//        var = var_ptr(sess, str, NULL);
//        if(var){
//            return var->val;
//        } else {
//        	val = (fnum_t)0;
//        }
//    }
//    return val;
//}

void show_group(struct session* sess, enum tok_type_t t){

    struct tok_s* tk;
    int i;
    int j;
    char* buf; size_t bufsize; size_t pos;

    buf = sess->cursess->outbuf;
    bufsize = sess->cursess->outbufsize;
    pos = 0;

    //printo("  ");

    snprintf(buf, bufsize, "  ");
    pos = strlen(buf);

    j = 0;
    for(tk = tok; tk->id != TOK_INVAL; tk++){
        if(tk->type == t){
            i = 0;
            while(tk->form[i]){
                //printo("%s", tk->form[i]);
                snprintf(buf + pos, bufsize - pos, "%s", tk->form[i]);
                pos += strlen(tk->form[i]);
                j++;
                i++;
                if(tk->form[i]){

                    //printo(", ");
                    snprintf(buf + pos, bufsize - pos, ", ");
                    pos += 2;

                    if(!(j%8)) {
                        //printo("\n  ");
                        snprintf(buf + pos, bufsize - pos, "\n  ");
                        pos += 3;
                        j = 0;
                    }
                }
            }
            //printo("; ");
            snprintf(buf + pos, bufsize - pos, "; ");
            pos += 2;
            if(!(j%8)) {
//                printo("\n  ");
                snprintf(buf + pos, bufsize - pos, "\n  ");
                pos += 3;
                j = 0;
            }
        }
    }
    //printo("\n");
    textout_(buf);
}

void show_tok_desc(struct session* sess, struct tok_s* reftk){

    int i;
    struct variable* var;
    struct tok_s* tk;
    char* buf;
    size_t bufsize;
    size_t pos;

    for(tk = tok; tk->id != TOK_INVAL; tk++){
        if(tk->id == reftk->id){
            if(tk->desc){

                printo("%s"PARAEND, tk->desc);

                if(reftk->id == TOK_VAR){
                    var = var_ptr(sess, reftk->str, NULL);
                    if(var){
                        printo(" %s"PARAEND, var->desc?var->desc:"description not set, try 'vdesc'.");
                    }
                } else if(reftk->id == TOK_ARRAY){
                    var = array_ptr(sess, reftk->str, reftk->i, reftk->j, NULL);
                    if(var){
                        printo(" %s"PARAEND, var->desc?var->desc:"description not set, try 'adesc'.");
                    }
                }

                printo("valid forms: ");
                i = 0;
                pos = 0;
                bufsize = sess->cursess->outbufsize;
                buf = sess->cursess->outbuf;
                while(tk->form[i]){
//                    printo("%s", tk->form[i]);
                    snprintf(buf + pos, bufsize - pos, "%s", tk->form[i]);
                    pos += strlen(tk->form[i]);
                    i++;
                    if(tk->form[i]){
                        //printo(", ");
                        snprintf(buf + pos, bufsize - pos, ", ");
                        pos += 2;
                    } else {
                        ///printo(";\n");
                        snprintf(buf + pos, bufsize - pos, ";");
                        pos += 1;
                    }
                }
                textout_(buf);

            }
            else printo("no description available [%i].\n", tk->id);
            break;
        }
    }

    if(tk->id == TOK_INVAL){
        printo("no description found [%i].\n", tk->id);
    }

}
