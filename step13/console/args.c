#include "step13.h"

#include <ctype.h>

#define _deb1(f, a...)

static struct step13_command{
    const char* str;
    enum step13_cmd_code code;
    const char* usage;
    const char* desc;
    int args_needed;
} step13_cmd[] = {

    {"init", STEP13_CMD_INIT, "", "initializes the step13 database", 0},
    {"update", STEP13_CMD_UPDATE, "", "updates the step13 database", 0},
    {"clean", STEP13_CMD_CLEAN, "", "removes the step13 database", 0},
    {"add", STEP13_CMD_ADD, "", "adds path to monitor or tag/comment to the given path", 1},
    {"rm", STEP13_CMD_RM, "", "removes path from monitoring or tag/comment from the given path", 1},
    {"compare", STEP13_CMD_COMPARE, "", "n/a", 0},
    {"list", STEP13_CMD_LIST, "", "lists records, takes all, backup, tag, compare", 1},
    {"commit", STEP13_CMD_COMMIT, "", "n/a", 0},
    {"checkout", STEP13_CMD_CHECKOUT, "", "n/a", 0},
    {"query", STEP13_CMD_QUERY, "", "n/a", 0},
    {"version", STEP13_CMD_VERSION, "", "n/a", 0},

    {"path", STEP13_CMDARG_PATH, "", "n/a", 0},
    {"tag", STEP13_CMDARG_TAG, "", "n/a", 0},
    {"comment", STEP13_CMDARG_COMMENT, "", "n/a", 0},//needs 1 arg if adding comment
    {"all", STEP13_CMDARG_ALL, "", "n/a", 0},
    {"cmp", STEP13_CMDARG_COMPARE, "", "n/a", 0},
    {"bak", STEP13_CMDARG_BACKUP, "", "n/a", 0},
    {"dup", STEP13_CMDARG_DUPLICATE, "", "n/a", 0},
    {"error", STEP13_CMDARG_ERROR, "", "n/a", 0},
    {"warning", STEP13_CMDARG_WARNING, "", "n/a", 0},
    {"info", STEP13_CMDARG_INFO, "", "n/a", 0},
    {"debug", STEP13_CMDARG_DEBUG, "", "n/a", 0},
    {"low", STEP13_CMDARG_LOW, "", "n/a", 0},
    {"medium", STEP13_CMDARG_MEDIUM, "", "n/a", 0},
    {"full", STEP13_CMDARG_FULL, "", "n/a", 0},
    {"none", STEP13_CMDARG_NONE, "", "n/a", 0},

    {0, 0}

};

static struct step13_switch_string{
    char* str_short;
    char* str_long;
    enum step13_switch code;
    char* usage;
    char* desc;
} step13_switch_str[] = {
  {"-v", "--verbose", STEP13_SW_VERBOSE, " <level=none, error, warning, info, debug>", "verbose level"},
  {"-u", "--recursive", STEP13_SW_RECURSIVE, " ", "enable recursive operations"},
  {"-l", "--detail", STEP13_SW_DETAIL, " <level=none, low, medium, full>", "detail level on output"},
  {"-n", "--nobackup", STEP13_SW_NOBACKUP, " ", "disable backups"},
  {"-r", "--root", STEP13_SW_ROOT, " <root=.>", "set root directory"},
  {"-s", "--source", STEP13_SW_SOURCE, " <source list=.\';\'>", "set source(s)"},
  {"-p", "--path", STEP13_SW_PATH, " <path list=.\';\'>", "set path(es)"},
  {"-b", "--backupdir", STEP13_SW_BACKUPDIR, " <backup directory=.>", "set backup directory"},
  {"-t", "--type", STEP13_SW_TYPE, " <\'&|!\'\'<>=!\'type\'&|!\'>", "set requested type(s)"},
  {"-e", "--ctime", STEP13_SW_CTIME, " <\'&|!\'\'<>=!\'creat time\'&|!\'>", "set requested create time(s)"},
  {"-m", "--mtime", STEP13_SW_MTIME, " <\'&|!\'\'<>=!\'modif time\'&|!\'>", "set requested modif time(s)"},
  {"-i", "--size", STEP13_SW_SIZE, " <\'&|!\'\'<>=!\'size\'&|!\'>", "set requested size(s)"},
  {"-c", "--comment", STEP13_SW_COMMENT, " <\'&|!\'\'<>=!\'comment\'&|!\'>", "set requested comment(s)"},
  {"-g", "--tag", STEP13_SW_TAG, " <\'&|!\'\'<>=!\'tag\'&|!\'>", "set requested tag(s)"},
  {"-a", "--stat", STEP13_SW_STAT, " <\'&|!\'\'<>=!\'status\'&|!\'>", "set requested status"},
  {0, 0}
};

int print_help_switches(struct step13* h){
	enum step13_switch sw;
	struct step13_switch_string* sw_str = step13_switch_str;

    for(sw = STEP13_SW_VERBOSE; sw < STEP13_SW_ERROR; sw++){
        lineout(h, STEP13_HELP, " +- %s or %s%s, %s", sw_str[sw].str_short,
                sw_str[sw].str_long, sw_str[sw].usage, sw_str[sw].desc);
    }

	return 0;
}

enum step13_cmd_code cmd_code(char* command){
    struct step13_command* cmd;

    if(!command) return 0;

    for(cmd = step13_cmd; cmd->str; cmd++){
        if(!strcmp(command, cmd->str)) return cmd->code;
    }

    return 0;
}

enum step13_switch detect_arg(char* arg){
    struct step13_switch_string* strs;

    if(!arg) return STEP13_SW_ERROR;

    for(strs = step13_switch_str; strs->str_short; strs++){
        if(!strcmp(arg, strs->str_short) || !strcmp(arg, strs->str_long))
            return strs->code;
    }

    return STEP13_SW_ERROR;
}

int extract_sw_args(struct step13* h, int argc, char *argv[], int* index){

    enum step13_switch sw;
    char sw_logic;
    register size_t arglen;// = strlen(argv[*index])-1;

    assert(*index < argc);

    arglen = strlen(argv[*index])-1;

    switch(argv[*index][arglen]){
        case '&':
            sw_logic = STEP13_LOGIC_AND;
            argv[*index][arglen] = '\0';
        break;
        case '|':
            sw_logic = STEP13_LOGIC_OR;
            argv[*index][arglen] = '\0';
        break;
        case '!':
            sw_logic = STEP13_LOGIC_NOT;
            argv[*index][arglen] = '\0';
        break;
        default:
            sw_logic = STEP13_LOGIC_NONE;
        break;
    }

    sw = detect_arg(argv[*index]);

    if(sw == STEP13_SW_ERROR){

        lineout(h, STEP13_ERROR, "Could not detect argument #%i [ %s ].", *index,
                argv[*index]);
        return -1;
    }

    h->sw_logic[sw] = sw_logic;

    if(h->switches & STEP13_SW(sw)){

        lineout(h, STEP13_ERROR, "the switch \'%s\' is already set, either by you or by default.",
                argv[*index]);
        return -1;

    }

    h->switches |= STEP13_SW(sw);

    (*index)++;

    switch(sw){

        case STEP13_SW_VERBOSE://verbose level

        if(*index < argc){

            if(argv[*index][0] != '-'){                

                if(!isdigit(argv[*index][0])){
                    switch(cmd_code(argv[*index])){
                        case STEP13_CMDARG_NONE:
                            h->vlevel = 0;
                        break;
                        case STEP13_CMDARG_ERROR:
                            h->vlevel = 1;
                        break;
                        case STEP13_CMDARG_WARNING:
                            h->vlevel = 2;
                        break;
                        case STEP13_CMDARG_INFO:
                            h->vlevel = 3;
                        break;
                        case STEP13_CMDARG_DEBUG:
                            h->vlevel = 4;
                        break;
                        default:
                            lineout(h, STEP13_ERROR, "could not understand argument \'%s\'.", argv[*index]);
                            return -1;
                        break;
                    }
                } else h->vlevel = atoi(argv[*index]);

            } else {

                h->vlevel = 1;
                (*index)--;

            }

        } else {

            h->vlevel = 1;

        }

        _deb1("set verbose level to \'%i\'.", h->vlevel);

        break;

        case STEP13_SW_RECURSIVE:

        (*index)--;

        h->switches |= STEP13_SW(STEP13_SW_RECURSIVE);

        _deb1("backup set to %s", BITSTATSTR(STEP13_SW(STEP13_SW_NOBACKUP), h->switches));

        break;

        case STEP13_SW_DETAIL://detail level

        if(*index < argc){

            if(argv[*index][0] != '-'){

                if(!isdigit(argv[*index][0])){
                    switch(cmd_code(argv[*index])){
                        case STEP13_CMDARG_NONE:
                            h->dlevel = 0;
                        break;
                        case STEP13_CMDARG_LOW:
                            h->dlevel = 1;
                        break;
                        case STEP13_CMDARG_MEDIUM:
                            h->dlevel = 5;
                        break;
                        case STEP13_CMDARG_FULL:
                            h->dlevel = 10;
                        break;
                        default:
                            lineout(h, STEP13_ERROR, "could not understand argument \'%s\'.", argv[*index]);
                            return -1;
                        break;
                    }
                } else h->dlevel = atoi(argv[*index]);

            } else {

                h->dlevel = 1;
                (*index)--;

            }

        } else {

            h->dlevel = 1;

        }

        _deb1("set detail level to \'%i\'.", h->dlevel);

        break;


        case STEP13_SW_NOBACKUP:

        (*index)--;

        h->switches |= STEP13_SW(STEP13_SW_NOBACKUP);

       _deb1("nobackup set to %s", BITSTATSTR(STEP13_SW(STEP13_SW_NOBACKUP), h->switches));

        break;

        case STEP13_SW_ROOT://path specifiers
        case STEP13_SW_BACKUPDIR:
        case STEP13_SW_PATH:
        case STEP13_SW_SOURCE:

        if(*index < argc){

            if(!p13_is_abs_path(argv[*index])){
                if(!p13_get_abs_path(argv[*index], h->sw_arg[sw])){
                    lineout(h, STEP13_ERROR, "Error getting absolute path of the given path (\'%s\').", argv[*index]);
                    return -1;
                }
            } else s13_strcpy(h->sw_arg[sw], argv[*index], MAXPATHNAME);

            set_default_dirs(h);

        } else {
            lineout(h, STEP13_ERROR, "You must specify a path using this switch.");
            return -1;
        }

        //_deb1("set root directory to \'%s\'.", h->sw_arg[STEP13_SW_ROOT]);

        break;

        default:

        if(*index < argc){

            if(h->sw_arg[sw]) free(h->sw_arg[sw]);

            if(!(h->sw_arg[sw] = (char*)malloc(strlen(argv[*index])+1))){
                lineout(h, STEP13_ERROR, "memmory allocation failed.");
                return -1;
            }

            snprintf(h->sw_arg[sw], MAXSWITCHARG, "%s", argv[*index]);

        } else {

            lineout(h, STEP13_ERROR, "You must specify another argument for this switch (\'%s\').", argv[(*index)-1]);
            return -1;

        }

        //_deb1("set switch \'%s\' to \'%s\', combination logic is 0x%x and switch id is %i.", argv[(*index)-1], argv[*index], sw_logic, sw);

        break;

    }

    (*index)++;

    return 0;
}

int fill_switch_where_string(struct step13* h, char* colname, enum step13_switch sw, char type){
    size_t bufsize = SQLWHEREBUFCHUNK;
    size_t fillsize = 0;
    char* buf;
    char* str;
    int i;

    while(h->sw_arg[sw][i]){

        switch(h->sw_arg[sw][i]){

            case '&':
            str = " AND ";
            break;

            case '|':
            str = " OR ";
            break;

            case '!':
                switch(type){
                    case 't':
                    str = " NOT LIKE ";
                    break;
                    default:
                    str = " != ";
                    break;
                }

            break;

            case '=':
                switch(type){
                    case 't':
                    str = " LIKE ";
                    break;
                    default:
                    str = " = ";
                    break;
                }
            break;

            case '>':

            default:
            break;
        }

        fillsize+=strlen(str);
        if(bufsize < fillsize + 1){
            bufsize += SQLWHEREBUFCHUNK;
            buf = (char*)realloc(buf, bufsize);
        }

        strcat(buf, str);

        i++;

    }

    return 0;

}

int switch_to_sql_where(struct step13* h){

    size_t bufsize = SQLWHEREBUFCHUNK;
    size_t fillsize = 0;
    char* str;
    char* buf = (char*)malloc(bufsize);
    int i;

    enum step13_switch sw;

    str = " WHERE (";
    strcpy(buf, str);
    fillsize += strlen(str);

    /*
     * where-able switches are, type, ctime, mtime, size, comment, tag, stat
     * an example of a valid logical string is:
     *  --tag| =hell&!nohell --comment =this_is_hell , becomes:
     *  WHERE( (tag LIKE hell AND tag NOT LIKE nohell) OR (comment LIKE this_is_hell) )
     */

    if(h->switches & STEP13_SW(STEP13_SW_TYPE)){

        if(fill_switch_where_string(h, "type", STEP13_SW_TYPE, 't') < 0) return -1;

    }

    if(h->switches & STEP13_SW(STEP13_SW_CTIME)){

        if(fill_switch_where_string(h, "ctime", STEP13_SW_CTIME, 'i') < 0) return -1;

    }

    if(h->switches & STEP13_SW(STEP13_SW_MTIME)){

        if(fill_switch_where_string(h, "mtime", STEP13_SW_MTIME, 'i') < 0) return -1;

    }

    if(h->switches & STEP13_SW(STEP13_SW_SIZE)){

        if(fill_switch_where_string(h, "size", STEP13_SW_SIZE, 'i') < 0) return -1;

    }

    if(h->switches & STEP13_SW(STEP13_SW_COMMENT)){

        if(fill_switch_where_string(h, "comment", STEP13_SW_COMMENT, 't') < 0) return -1;

    }

    if(h->switches & STEP13_SW(STEP13_SW_TAG)){

        if(fill_switch_where_string(h, "tags", STEP13_SW_TAG, 't') < 0) return -1;

    }

    if(h->switches & STEP13_SW(STEP13_SW_STAT)){

        if(fill_switch_where_string(h, "stat", STEP13_SW_STAT, 't') < 0) return -1;

    }

    str = ");";
    fillsize += strlen(str);
    if(bufsize < fillsize + 1){
        bufsize += SQLWHEREBUFCHUNK;
        buf = realloc(buf, bufsize);
    }
    strcat(buf, str);

    return 0;
}

int parse_switch(struct step13 *h, int argc, char *argv[], int base){

    int i = base;

    while(i < argc){

        if(extract_sw_args(h, argc, argv, &i) < 0) return -1;

    }

    //if(switch_to_sql_where(h) < 0) return -1;

    return 0;

}

int parse_args(struct step13 *h, int argc, char *argv[]){

    if(argc < 2){
        help(h, argv[0]);
        return 0;
    }

    int argvi = 1;

    switch( (h->cmd = cmd_code(argv[argvi])) ){

        case STEP13_CMD_ADD:
        case STEP13_CMD_RM:
        break;

        case STEP13_CMD_LIST:

            switch( (h->cmd_arg = cmd_code(argv[++argvi])) ){

            case STEP13_CMDARG_ALL:
            case STEP13_CMDARG_BACKUP:
            case STEP13_CMDARG_TAG:            
            case STEP13_CMDARG_COMPARE:
            break;

            default:
                lineout(h, STEP13_ERROR, "Unknown command argument\'%s\'.", argv[argvi]);
                return -1;
            break;

            }

        break;

        case STEP13_CMD_INIT:
        case STEP13_CMD_UPDATE:        
        case STEP13_CMD_CLEAN:        
        case STEP13_CMD_COMPARE:        
        case STEP13_CMD_COMMIT:        
        case STEP13_CMD_CHECKOUT:
        break;

        case STEP13_CMD_QUERY:
            snprintf(h->sql, MAXSQL, argv[++argvi]);
        break;

        case STEP13_CMD_VERSION:
        break;

        default:
            lineout(h, STEP13_ERROR, "Unknown command \'%s\'.", argv[argvi]);
            return -1;
        break;

    }

    argvi++;

    if(parse_switch(h, argc, argv, argvi) < 0) return -1;

    return 0;
}
