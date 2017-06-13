#include "step13.h"

//#define _deb3 _DebugMsg
#define _deb1(f, a...)
#define _deb2(f, a...)
#define _deb3(f, a...)
#define _deb4(f, a...)

int help(struct step13* h, char* progpath){

    //enum step13_switch sw;
    //struct step13_switch_string* sw_str = step13_switch_str;

    lineout(h, STEP13_HELP,"Welcome to %s; a synchronization, journalling and tracking program.", PROGTITLE);
    lineout(h, STEP13_HELP, "Usage:");
    lineout(h, STEP13_HELP,"%s <command> <switches>", PROGNAME);
    lineout(h, STEP13_HELP, "Commands are:");
    lineout(h, STEP13_HELP, " +- init");
    lineout(h, STEP13_HELP, " +- update");
    lineout(h, STEP13_HELP, " +- clean");
    lineout(h, STEP13_HELP, " +- add");
    lineout(h, STEP13_HELP, " +- rm");
    lineout(h, STEP13_HELP, " +- compare");
    lineout(h, STEP13_HELP, " +- list <list command>");
    lineout(h, STEP13_HELP, " |        +- all");
    lineout(h, STEP13_HELP, " |        +- cmp");
    lineout(h, STEP13_HELP, " |        +- bak");
    lineout(h, STEP13_HELP, " |        +- tag");
    lineout(h, STEP13_HELP, " |        \\- dup");
    lineout(h, STEP13_HELP, " +- commit");
    lineout(h, STEP13_HELP, " +- checkout");
    lineout(h, STEP13_HELP, " +- query\n");
    lineout(h, STEP13_HELP, " \\- version\n");

    lineout(h, STEP13_HELP, "\n -- Press any key to see the switches -- ");
    getchar();

    lineout(h, STEP13_HELP, "Switches are:");

    print_help_switches(h);//this one has to be in args.c

    lineout(h, STEP13_HELP, "\n *  Example: %s list all -r /home/myhome -s /home/home1 -g hello;goodbye -m 1389-3-12-12-0-0 -i 10mb", PROGNAME);
    lineout(h, STEP13_HELP, "\n-- [ written by Babak Akhondi, < jaysi13@gmail.com > ]");
    return 0;
}

int version(struct step13* h){
    char* target = "Unknown";
    char* exectype = "Release";

#ifdef linux
    target = "Linux";
#elif defined(__WINNT)
    target = "Windows";
#endif

#ifndef NDEBUG
    exectype = "Debug";
#endif

    lineout(h, STEP13_HELP, "version \"%s\", compiled for %s, %s executable",
            VERSION, target, exectype);
    return 0;
}

char* clean_float(double num, char* __clfl_buf){
    int i;

    snprintf(__clfl_buf, MAXNUM, "%f", num);

    i = strlen(__clfl_buf)-1;

    while(__clfl_buf[i] == '0') i--;

    if(__clfl_buf[i] != '.') __clfl_buf[i+1] = '\0';
    else __clfl_buf[i] = '\0';

    return __clfl_buf;
}

char* sep_val(double num, char* __clfl_buf){

    int ilen, iseplen, i;
    char tmp[MAXNUM];
    char* end;

    snprintf(tmp, MAXNUM, "%s", clean_float(num, __clfl_buf));
    end = strstr(tmp, ".");

    if(!end){
        ilen = strlen(tmp);
        iseplen = ilen + (ilen / 3) - (!(ilen%3)?1:0);// + (!(ilen%2)?1:0);
    } else {
        ilen = end - tmp;
        iseplen = ilen + (ilen / 3) - (!(ilen%3)?1:0);// + (!(ilen%2)?1:0);
        memcpy(__clfl_buf + iseplen, end, strlen(end)+1);
    }

//	printf("ilen=%i, iseplen=%i\n", ilen, iseplen);

    i = 0;

    while(iseplen){

        __clfl_buf[iseplen--] = tmp[ilen--];

        if( i == 3 ){
            __clfl_buf[iseplen--] = '\'';
            i = 0;
        }

        i++;

    }

    return __clfl_buf;
}
char* get_size(size_t size, char* __clfl_buf){

    _deb2("isize: %u", size);    

    char tmp[MAXSIZEBUF];

    if(size < KB){
        snprintf(__clfl_buf, MAXSIZEBUF, "%s B ", sep_val(size, __clfl_buf));
    } else if(size >= KB && size < MB){
        snprintf(__clfl_buf, MAXSIZEBUF, "%s KB (", sep_val(size/KB, __clfl_buf));
    } else if (size >= MB && size < GB){
        snprintf(__clfl_buf, MAXSIZEBUF, "%s MB (", sep_val(size/MB, __clfl_buf));
    } else if (size >= GB){
        snprintf(__clfl_buf, MAXSIZEBUF, "%s GB (", sep_val(size/GB, __clfl_buf));
    }

    if(size >= KB){
        strcat(__clfl_buf, sep_val(size, tmp));
        strcat(__clfl_buf, " B)");
    }

    _deb2("size: %s", __clfl_buf);

    return __clfl_buf;
}

int show_row(void* handle){

    char ti[MAXTIME];
    char ti2[MAXTIME*2], ti3[MAXTIME*2];
    char buf[1024];
    char* path, *rroot;
    char* tary[6];
    char sizebuf[MAXSIZEBUF];
    time_t t;
    struct step13* h = handle;

    switch(sqlite3_step(h->stmt)){
        case SQLITE_ROW:
        _deb1("got another row!");
        break;
        case SQLITE_DONE:
        _deb1("done query");
        return 1;
        break;
        default:
        return -1;
        break;
    }

    h->qsize += (size_t)sqlite3_column_int(h->stmt, STEP13_COLID_SIZE);
    if(!strcmp((char*)sqlite3_column_text(h->stmt, STEP13_COLID_TYPE), P13_TYPE_DIR)) h->dirs++;
    else if(!strcmp((char*)sqlite3_column_text(h->stmt, STEP13_COLID_TYPE), P13_TYPE_FILE)) h->files++;
    else h->others++;

    if(h->dlevel > 1){

        //ctime
        t = (time_t)sqlite3_column_int(h->stmt, STEP13_COLID_CTIME);
        _deb4("ct = %i", (int)t);
        d13_time13(&t, ti);
        s13_explode(ti, "-", 0, 0, tary);

        snprintf(ti2, MAXTIME*2, "%s/%s/%s %s:%s:%s",
                 tary[0], tary[1], tary[2], tary[3], tary[4], tary[5]);

        //mtime
        t = (time_t)sqlite3_column_int(h->stmt, STEP13_COLID_MTIME);
        _deb4("mt = %i", (int)t);
        d13_time13(&t, ti);
        s13_explode(ti, "-", 0, 0, tary);

        snprintf(ti3, MAXTIME*2, "%s/%s/%s %s:%s:%s",
                 tary[0], tary[1], tary[2], tary[3], tary[4], tary[5]);
    }

    path = (char*)sqlite3_column_text(h->stmt, STEP13_COLID_PATH);

    if(h->dlevel < 2){
        rroot = p13_get_unix_path(h->sw_arg[STEP13_SW_ROOT]);
        if(!strcmp(path, rroot)){
            strcpy(path, THIS_DIR);
        } else if(!memcmp(path, rroot, strlen(rroot))){
            path += strlen(rroot);
            if(!memcmp(path, DIR_SEP, strlen(DIR_SEP))) path += strlen(DIR_SEP);
        }
    }

    //lineout(JOTA_DEBUG, "path: %s", path);

    if(!h->dlevel){
        snprintf(buf, 1024, "%s%s", h->list_sign, path);
    } else if(h->dlevel == 1) {
        snprintf(buf, 1024, "%s%s -> %s", h->list_sign, path, sqlite3_column_text(h->stmt, STEP13_COLID_STAT));
    } else {
        snprintf(buf, 1024, "%sName:%s\n%sType:%s\n%sCreation Time:%s\n%sModification Time:%s\n%sSize:%s\n%sComment:%s\n%sTags:%s\n%sLevel:%i\n%sStatus:%s\n",
                 h->list_sign,
               path,//name
                 h->list_sign,
               sqlite3_column_text(h->stmt, STEP13_COLID_TYPE),//type
                 h->list_sign,
               ti2,//creat time
                 h->list_sign,
               ti3,//modif time
                 h->list_sign,
               get_size((size_t)sqlite3_column_int(h->stmt, STEP13_COLID_SIZE), sizebuf),//size
                 h->list_sign,
               sqlite3_column_text(h->stmt, STEP13_COLID_COMMENT),//comment
                 h->list_sign,
               sqlite3_column_text(h->stmt, STEP13_COLID_TAG),//tags
                 h->list_sign,
               sqlite3_column_int(h->stmt, STEP13_COLID_LEVEL),//level
                 h->list_sign,
               sqlite3_column_text(h->stmt, STEP13_COLID_STAT)//stat
               );
    }

    lineout(h, STEP13_OUT, buf);

    return 0;//to continue the loop
}

int show_tags(void* handle){

    struct step13* h = handle;

    switch(sqlite3_step(h->stmt)){
        case SQLITE_ROW:
        _deb1("got another row!");
        break;
        case SQLITE_DONE:
        _deb1("done query");
        return 1;//to finish the loop
        break;
        default:
        return -1;
        break;
    }

    printf("%s%s\n", h->list_sign, (char*)sqlite3_column_text(h->stmt, 0));

    return 0;

}

