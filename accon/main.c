#include <stdio.h>
#include <stdlib.h>

#define MAX_INPUT	1024
#define MAX_CMD		10
#include "../lib13/include/acc13.h"
#include "../lib13/include/str13.h"
#include "../lib13/include/db13.h"
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
			"groupset \'groupname\' \'stat=(A)ctive|(I)nactive|(R)emoved\'"

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

struct _cmd* translate_s(char* str){
	struct _cmd* c;
	int i;
	for(c = cmd; c->cmd[0]; c++){
		i = 0;
		while(c->cmd[i]){
//            printo("[%i]:%s\n", i, c->cmd[i]);
			if(!strcmp(c->cmd[i], str)) return c;
			i++;
		}
	}
	return NULL;
}

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

int printe13(error13_t err13, char* msg){
	printo("\nERROR: %s (%s)\n", e13_codemsg(err13), msg?msg:"-");
	return 0;
}

int prompt(char* init_str, char* input){

	printo("\n%s>", init_str);
	scani(input);

	return 0;
}

int help(int n, char** ary){
	struct _cmd* c;
	int i;

	if(n > 1){
		c = translate_s(ary[1]);

		i = 0;
		while(c->cmd[i]){
			printo("\nforms: ");
			printo("%s, ", c->cmd[i]);
			i++;
		}

		printo("\ndescription: %s", c->desc);
		printo("\nsyntax: %s\n", c->syntax);

		return 0;
	}

	printo("\n-- access13 console help --\n");
	for(c = cmd; c->cmd[0]; c++){
		i = 0;
		printo("\nforms: ");
		while(c->cmd[i]){
			printo("%s, ", c->cmd[i]);
			i++;
		}
		printo("\ndescription: %s", c->desc);
		printo("\nsyntax: %s\n", c->syntax);
	}

	return 0;
}

int do_open(struct access13* ac, int n, char** ary){
	error13_t ret;
	struct db13* db;
	if(n < 2){
        printo("bad usage, try help '%s'", ary[0]);
        return -1;
	}

	printo("openning %s...", ary[1]);

	db = m13_malloc(sizeof(struct db13));
	if(!db){
		printo("failed\n");
		printe13(e13_error(E13_NOMEM), "do_open()");
		return -1;
	}
	ret = db_init(db, DB_DRV_SQLITE);
	if(ret != E13_OK){
		printo("failed\n");
		printe13(ret, "do_open()");
		return -1;
	}
	ret = db_open(db, NULL, NULL, NULL, NULL, ary[1]);
	if(ret != E13_OK){
		printo("failed\n");
		printe13(ret, ary[1]);
		return -1;
	}

	ret = acc_init(ac, db, 0);
	if(ret != E13_OK){
		printo("failed\n");
		printe13(ret, ary[1]);
		return -1;
	}

	printo("done\n");

	return 0;

}

int do_close(struct access13* ac, int n, char** ary){
	error13_t ret;

	printo("closing...");

    ret = acc_destroy(ac);
	if(ret != E13_OK){
		printo("failed\n");
		printe13(ret, NULL);
		return -1;
	}

	printo("done\n");

    return 0;
}

int do_groupadd(struct access13* ac, int n, char** ary){
    error13_t ret;

	if(n < 2){
        printo("bad usage, try help '%s'", ary[0]);
        return -1;
	}

	printo("adding %s...", ary[1]);

    ret = acc_group_add(ac, ary[1]);
	if(ret != E13_OK){
		printo("failed\n");
		printe13(ret, ary[1]);
		return -1;
	}

	printo("done\n");

    return 0;

}

int do_grouprm(struct access13* ac, int n, char** ary){
    error13_t ret;
    gid13_t id;

	if(n < 2){
        printo("bad usage, try help '%s'", ary[0]);
        return -1;
	}

    id = strtoul(ary[1], NULL, 10);

    printo("removing %s[%u]...", id==ULONG_MAX?ary[1]:"-", id==ULONG_MAX?GID13_INVAL:id);

    ret = acc_group_rm(ac, id==ULONG_MAX?ary[1]:NULL, id);
	if(ret != E13_OK){
		printo("failed\n");
		printe13(ret, ary[1]);
		return -1;
	}

	printo("done\n");

    return 0;

}

int do_groupset(struct access13* ac, int n, char** ary){
    error13_t ret;
    gid13_t id;
    int stt;

	if(n < 3){
        printo("bad usage, try help '%s'", ary[0]);
        return -1;
	}

    id = strtoul(ary[1], NULL, 10);

    if(ary[2][0]=='i' || ary[2][0]=='I') stt = ACC_GRP_STT_INACTIVE;
    if(ary[2][0]=='r' || ary[2][0]=='R') stt = ACC_GRP_STT_REMOVED;
    else stt = ACC_GRP_STT_ACTIVE;

    printo("setting status of %s[%u] to %s...",
		id==ULONG_MAX?ary[1]:"-",
		id==ULONG_MAX?GID13_INVAL:id,
		stt==ACC_GRP_STT_ACTIVE?"ACTIVE":(stt==ACC_GRP_STT_INACTIVE?"INACTIVE":"REMOVED")
		);

    ret = acc_group_set_stat(ac, id==ULONG_MAX?ary[1]:NULL, id, stt);
	if(ret != E13_OK){
		printo("failed\n");
		printe13(ret, ary[1]);
		return -1;
	}

	printo("done\n");

    return 0;

}

int do_groupchk(struct access13* ac, int n, char** ary){
    error13_t ret;
    gid13_t id;
    struct group13 group;

	if(n < 2){
        printo("bad usage, try help '%s'", ary[0]);
        return -1;
	}

    id = strtoul(ary[1], NULL, 10);

    printo("checking %s[%u]...", id==ULONG_MAX?ary[1]:"-", id==ULONG_MAX?GID13_INVAL:id);

    ret = acc_group_chk(ac, id==ULONG_MAX?ary[1]:NULL, id, &group);
	if(ret != E13_OK){
		printo("failed\n");
		printe13(ret, ary[1]);
		return -1;
	}

	printo("done\n");

	printo("-- group info --\n");
	printo("name: %s\n", group.name);
	printo("gid: %u\n", group.gid);
	printo("status: %s\n", group.stt==ACC_GRP_STT_INACTIVE?"INACTIVE":(group.stt==ACC_GRP_STT_REMOVED?"REMOVED":"ACTIVE"));
    printo("-- group info --\n");

    return 0;

}

int do_grouplist(struct access13* ac, int n, char** ary){

	gid13_t num, i;
	struct group13* grouplist, *group;
	error13_t ret;

	printo("getting group list...");

    ret = acc_group_list(ac, &num, &grouplist);
	if(ret != E13_OK){
		printo("failed\n");
		printe13(ret, ary[1]);
		return -1;
	}

	printo("done\n");

	if(!num || !grouplist){
		printo("-- empty --\n");
		return 0;
	}

	group = grouplist;

	printo("-- group list [%u] --\n", num);
	i = 0;
	while(group){
		printo("No.: %u\n", i++);
		printo("name: %s\n", group->name);
		printo("gid: %u\n", group->gid);
		printo("status: %s\n\n",
				group->stt==ACC_GRP_STT_INACTIVE?"INACTIVE":(group->stt==ACC_GRP_STT_REMOVED?"REMOVED":"ACTIVE"));

		group = group->next;
	}
	printo("-- group list --\n");

	return 0;

}

int main(int argc, char* argv[])
{
	char input[MAX_INPUT];
	int n;
	char** ary;
	struct access13 ac;

	escape = ESCAPE;
	strcpy(delim, DELIM);
    pack[0] = PACK1;
    pack[1] = PACK2;
    pack[2] = NULL;

    printo("-- access13 console --\n");
show_prompt:
	prompt("main", input);

    n = s13_exparts(input, delim, pack, escape);

    ary = s13_exmem(n);

    s13_explode(input, delim, pack, escape, ary);

	switch(translate(ary[0])){
	case CODE_HELP:
	help(n, ary);
	break;
	case CODE_EXIT:
		return 0;
		break;

	case CODE_OPEN:
		do_open(&ac, n, ary);
		break;
	case CODE_CLOSE:
		do_close(&ac, n, ary);
		break;
	case CODE_GROUPADD:
		do_groupadd(&ac, n, ary);
		break;
	case CODE_RMGROUP:
		do_grouprm(&ac, n, ary);
		break;
	case CODE_GROUPSET:
		do_groupset(&ac, n, ary);
		break;
	case CODE_GROUPCHK:
		do_groupchk(&ac, n, ary);
		break;
    case CODE_GROUPLIST:
    	do_grouplist(&ac, n, ary);
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
