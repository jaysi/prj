#ifndef UI_H
#define UI_H

#include <errno.h>

struct step13;

#define STEP13_OUT        0
#define STEP13_ERROR      1
#define STEP13_WARNING    2
#define STEP13_INFO       3
#define STEP13_HELP       4
#define STEP13_DEBUG      5

#define lineout(h, code, fmt, args...) {\
        switch(code){\
            case STEP13_ERROR:\
            if((h)->vlevel){\
                printf("#E[ ");\
                printf(fmt, ## args);\
            }\
            break;\
            case STEP13_WARNING:\
            if((h)->vlevel > 1){\
                printf("!W[ ");\
                printf(fmt, ## args);\
            }\
            break;\
            case STEP13_INFO:\
            if((h)->vlevel > 2){\
                printf(":I[ ");\
                printf(fmt, ## args);\
            }\
            break;\
            case STEP13_DEBUG:\
            if((h)->vlevel > 3){\
                printf("?D{ ");\
                printf(fmt, ## args);\
            }\
            break;\
            case STEP13_OUT:\
            if((h)->vlevel > 2){\
                printf(fmt, ## args);\
            }\
            break;\
            default:\
            printf(fmt, ## args);\
            break;\
        }\
        switch(code){\
            case STEP13_ERROR:\
            if((h)->vlevel){\
                printf(" ]\n");\
                if(errno) perror("#S");\
            }\
            break;\
            case STEP13_WARNING:\
            if((h)->vlevel > 1){\
                printf(" ]\n");\
            }\
            break;\
            case STEP13_INFO:\
            if((h)->vlevel > 2){\
                printf(" ]\n");\
            }\
            break;\
            case STEP13_DEBUG:\
            if((h)->vlevel > 3){\
                printf(" }\n");\
            }\
            break;\
            case STEP13_HELP:\
            printf("\n");\
            break;\
            case STEP13_OUT:\
            if((h)->vlevel > 2){\
                printf("\n");\
            }\
            break;\
            default:\
            break;\
        }\
    };

#ifdef __cplusplus
    extern "C" {
#endif

int help(struct step13* h, char* progpath);
enum step13_cmd_code cmd_code(char* str);
int get_main_list(struct step13* h);
char* get_size(size_t size, char* sizebuf);

int show_row(void* handle);
int show_tags(void* handle);

#ifdef __cplusplus
    }
#endif

#endif // UI_H
