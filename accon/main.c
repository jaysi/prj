#include <stdio.h>
#include <stdlib.h>

#define MAX_INPUT	1024
#define MAX_CMD		10
#include "../lib13/include/acc13.h"
#include "../lib13/include/str13.h"
#define printo printf
#define scani gets

#define ESCAPE	'\\'
#define PACK1	"\""
#define PACK2	"'"
#define DELIM	" "


char escape;
char* pack[3];
char delim[2];

enum cmd_code {
	CODE_EMPTY,
	CODE_HELP,
	CODE_EXIT,
	CODE_OPEN,
	CODE_CLOSE,
	CODE_GROUPADD,
	CODE_RMGROUP,
	CODE_GROUPSET,
	CODE_GROUPCHK,
    CODE_GROUPLIST,
	CODE_USERADD,
	CODE_RMUSER,
	CODE_USERSET,
	CODE_USERCHK,
    CODE_USERLIST,
    CODE_LOGIN,
    CODE_LOGOUT,
    CODE_PERM_USER_CHK,
    CODE_PERM_USER_ADD,
    CODE_PERM_USER_RM,
    CODE_PERM_GROUP_CHK,
    CODE_PERM_GROUP_ADD,
    CODE_PERM_GROUP_RM,
	CODE_INVAL
};

struct _cmd {
    enum cmd_code code;
    char* cmd[10];
    char* desc;
    char* syntax;
} cmd[] = {
	{
			CODE_EMPTY,
			{
            "", NULL
			},
			NULL,
			NULL,
	},
	{
			CODE_HELP,
			{
            "help", "?", NULL
			},
			"prints help message",
			"help"

	},
	{
			CODE_EXIT,
			{
            "quit", "exit", NULL
			},
			"exit the program",
			"exit"

	},
	{
			CODE_OPEN,
			{
            "open", NULL
			},
			"open database",
			"open \'filename\'"

	},
	{
			CODE_CLOSE,
			{
            "close", NULL
			},
			"close database",
			"close"

	},
	{
			CODE_GROUPADD,
			{
            "groupadd", "gadd", NULL
			},
			"add group(s)",
			"groupadd \'groupname1\' \'groupname2\' ..."

	},

	{
			CODE_RMGROUP,
			{
            "rmgroup", "grm", "grouprm", "rmg", "delgroup", "groupdel", "gdel", "delg", NULL
			},
			"removes group(s)",
			"rmgroup \'groupname1\' \'groupname2\' ..."

	},
	{
			CODE_GROUPSET,
			{
            "groupset", "gset", NULL
			},
			"sets group config",
			"groupset \'groupname\'"

	},
	{
			CODE_GROUPLIST,
			{
            "grouplist", "glist", NULL
			},
			"lists groups",
			"grouplist"

	},
	{
			CODE_USERADD,
			{
            "useradd", "uadd", NULL
			},
			"add user(s)",
			"useradd \'username1\' \'username2\' ..."

	},

	{
			CODE_RMUSER,
			{
            "rmuser", "urm", "userrm", "rmu", "deluser", "userdel", "udel", "delu", NULL
			},
			"removes user(s)",
			"rmuser \'username1\' \'username2\' ..."

	},
	{
			CODE_USERSET,
			{
            "userset", "uset", NULL
			},
			"sets user config",
			"userset \'username\'"

	},
	{
			CODE_USERLIST,
			{
            "userlist", "ulist", NULL
			},
			"lists users",
			"userlist"

	},
	{
			CODE_LOGIN,
			{
            "login", "lin", NULL
			},
			"login user",
			"login \'username1\' \'password1\' \'username2\' \'password2\' ..."

	},
	{
			CODE_LOGOUT,
			{
            "logout", "lout", NULL
			},
			"logout user",
			"logout \'username1\' \'username2\' ..."

	},
	{
		CODE_INVAL,
		{NULL},
		NULL,
		NULL
	}
};

enum cmd_code translate(char* str){
	struct _cmd* c;
	int i;
	for(c = cmd; c->cmd[0]; c++){
		i = 0;
		while(c->cmd[i]){
//            printo("[%i]:%s\n", i, c->cmd[i]);
			if(!strcmp(c->cmd[i], str)) return c->code;
			i++;
		}
	}
	return CODE_INVAL;
}

int prompt(char* init_str, char* input){

	printo("\n%s>", init_str);
	scani(input);

	return 0;
}

int help(){
	struct _cmd* c;
	int i;

	printo("\n-- access13 console help --\n");
	for(c = cmd; c->cmd[0]; c++){
		i = 0;
		while(c->cmd[i]){
			printo("\nforms: ");
			printo("%s, ", c->cmd[i]);
			i++;
		}
		printo("\ndescription: %s", c->desc);
		printo("\nsyntax: %s\n", c->syntax);
	}
	return 0;
}

int main(int argc, char* argv[])
{
	char input[MAX_INPUT];
	int n;
	char** ary;

	escape = ESCAPE;
	strcpy(delim, DELIM);
    pack[0] = PACK1;
    pack[1] = PACK2;
    pack[2] = NULL;

    printo("-- access13 console --\n");
show_prompt:
	prompt("main", input);
	printo("translating %s...\n", input);

    n = s13_exparts(input, delim, pack, escape);

    ary = s13_exmem(n);

    s13_explode(input, delim, pack, escape, ary);

	switch(translate(ary[0])){
	case CODE_HELP:
	help();
	break;
	case CODE_EXIT:
		return 0;
		break;

	case CODE_OPEN:
		break;
	case CODE_CLOSE:
		break;
	case CODE_GROUPADD:
		break;
	case CODE_RMGROUP:
		break;
	case CODE_GROUPSET:
		break;
	case CODE_GROUPCHK:
		break;
    case CODE_GROUPLIST:
		break;
	case CODE_USERADD:
		break;
	case CODE_RMUSER:
		break;
	case CODE_USERSET:
		break;
	case CODE_USERCHK:
		break;
    case CODE_USERLIST:
		break;
    case CODE_LOGIN:
		break;
    case CODE_LOGOUT:
		break;

	default:
		printo("unknown input %s\n", input);
		break;
	}

	s13_free_exmem(ary);

	goto show_prompt;

    return 0;
}
