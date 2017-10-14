#include <stdio.h>
#include <stdlib.h>

#define MAX_INPUT	1024
#define MAX_CMD		10
#include "../lib13/include/acc13.h"
#include "../lib13/include/str13.h"
#include "../lib13/include/db13.h"
//#define printf(fmt, args...) printf(fmt, ## args)
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
    CODE_USER_JOIN_GROUP,
    CODE_USER_LEAVE_GROUP,
    CODE_USER_GROUP_CHK,
    CODE_USER_GROUP_LIST,
    CODE_GROUP_USER_LIST,
    CODE_PERM_USER_CHK,
    CODE_PERM_USER_ADD,
    CODE_PERM_USER_RM,
    CODE_PERM_GROUP_CHK,
    CODE_PERM_GROUP_ADD,
    CODE_PERM_GROUP_RM,
    CODE_USER_ACCESS,
    CODE_DB_STAT,
	CODE_INVAL
};

struct _cmd {
    enum cmd_code code;
    char* cmd[10];
    char* desc;
    char* syntax;
    int(*func)(struct access13*, int, char**);
};

int printe13(error13_t err13, char* msg){
	printf("\nERROR (%i): %s (%s)\n", err13, e13_codemsg(err13), msg?msg:"-");
	return 0;
}

int prompt(char* init_str, char* input){

	printf("\n%s>", init_str);
	scani(input);

	return 0;
}

int do_open(struct access13* ac, int n, char** ary){
	error13_t ret;
	struct db13* db;
	if(n < 2){
        printf("bad usage, try help '%s'", ary[0]);
        return -1;
	}

	printf("openning %s...", ary[1]);

	db = m13_malloc(sizeof(struct db13));
	if(!db){
		printf("failed\n");
		printe13(e13_error(E13_NOMEM), "do_open()");
		return -1;
	}
	ret = db_init(db, DB_DRV_SQLITE);
	if(ret != E13_OK){
		printf("failed\n");
		printe13(ret, "do_open()");
		return -1;
	}
	ret = db_open(db, NULL, NULL, NULL, NULL, ary[1]);
	if(ret != E13_OK){
		printf("failed\n");
		printe13(ret, ary[1]);
		return -1;
	}

	ret = acc_init(ac, db, 0);
	if(ret != E13_OK){
		printf("failed\n");
		printe13(ret, ary[1]);
		return -1;
	}

	printf("done\n");

	return 0;

}

int do_close(struct access13* ac, int n, char** ary){
	error13_t ret;

	printf("closing...");

    ret = acc_destroy(ac);
	if(ret != E13_OK){
		printf("failed\n");
		printe13(ret, NULL);
		return -1;
	}

//	ret = db_close(ac->db);
//	if(ret != E13_OK){
//		printf("failed\n");
//		printe13(ret, NULL);
//		return -1;
//	}

	printf("done\n");

    return 0;
}

int do_groupadd(struct access13* ac, int n, char** ary){
    error13_t ret;

	if(n < 2){
        printf("bad usage, try help '%s'", ary[0]);
        return -1;
	}

	printf("adding %s...", ary[1]);

    ret = acc_group_add(ac, ary[1]);
	if(ret != E13_OK){
		printf("failed\n");
		printe13(ret, ary[1]);
		return -1;
	}

	printf("done\n");

    return 0;

}

int do_grouprm(struct access13* ac, int n, char** ary){
    error13_t ret;
    gid13_t id;

	if(n < 2){
        printf("bad usage, try help '%s'", ary[0]);
        return -1;
	}

    id = strtoul(ary[1], NULL, 10);

    printf("removing %s[%u]...", id==ULONG_MAX?ary[1]:"-", id==ULONG_MAX?GID13_INVAL:id);

    ret = acc_group_rm(ac, id==ULONG_MAX?ary[1]:NULL, id);
	if(ret != E13_OK){
		printf("failed\n");
		printe13(ret, ary[1]);
		return -1;
	}

	printf("done\n");

    return 0;

}

int do_groupset(struct access13* ac, int n, char** ary){
    error13_t ret;
    gid13_t id;
    int stt;

	if(n < 3){
        printf("bad usage, try help '%s'", ary[0]);
        return -1;
	}

    id = strtoul(ary[1], NULL, 10);

    if(ary[2][0]=='i' || ary[2][0]=='I') stt = ACC_GRP_STT_INACTIVE;
    if(ary[2][0]=='r' || ary[2][0]=='R') stt = ACC_GRP_STT_REMOVED;
    else stt = ACC_GRP_STT_ACTIVE;

    printf("setting status of %s[%u] to %s...",
		id==ULONG_MAX?ary[1]:"-",
		id==ULONG_MAX?GID13_INVAL:id,
		stt==ACC_GRP_STT_ACTIVE?"ACTIVE":(stt==ACC_GRP_STT_INACTIVE?"INACTIVE":"REMOVED")
		);

    ret = acc_group_set_stat(ac, id==ULONG_MAX?ary[1]:NULL, id, stt);
	if(ret != E13_OK){
		printf("failed\n");
		printe13(ret, ary[1]);
		return -1;
	}

	printf("done\n");

    return 0;

}

int do_groupchk(struct access13* ac, int n, char** ary){
    error13_t ret;
    gid13_t id;
    struct group13 group;

	if(n < 2){
        printf("bad usage, try help '%s'", ary[0]);
        return -1;
	}

    id = strtoul(ary[1], NULL, 10);

    printf("checking %s[%u]...", id==ULONG_MAX?ary[1]:"-", id==ULONG_MAX?GID13_INVAL:id);

    ret = acc_group_chk(ac, id==ULONG_MAX?ary[1]:NULL, id, &group);
	if(ret != E13_OK){
		printf("failed\n");
		printe13(ret, ary[1]);
		return -1;
	}

	printf("done\n");

	printf("-- group info --\n");
	printf("name: %s\n", group.name);
	printf("gid: %u\n", group.gid);
	printf("status: %s\n", group.stt==ACC_GRP_STT_INACTIVE?"INACTIVE":(group.stt==ACC_GRP_STT_REMOVED?"REMOVED":"ACTIVE"));
    printf("-- group info --\n");

    return 0;

}

int do_grouplist(struct access13* ac, int n, char** ary){

	gid13_t num, i;
	struct group13* grouplist, *group;
	error13_t ret;

	printf("getting group list...");

    ret = acc_group_list(ac, &num, &grouplist);
	if(ret != E13_OK){
		printf("failed\n");
		printe13(ret, ary[1]);
		return -1;
	}

	printf("done\n");

	if(!num || !grouplist){
		printf("-- empty --\n");
		return 0;
	}

	group = grouplist;

	printf("-- group list [%u] --\n", num);
	i = 0;
	while(group){
		printf("No.: %u\n", i++);
		printf("name: %s\n", group->name);
		printf("gid: %u\n", group->gid);
		printf("status: %s\n\n",
				group->stt==ACC_GRP_STT_INACTIVE?"INACTIVE":(group->stt==ACC_GRP_STT_REMOVED?"REMOVED":"ACTIVE"));

		group = group->next;
	}
	acc_group_list_free(grouplist);
	printf("-- group list --\n");

	return 0;

}

int do_useradd(struct access13* ac, int n, char** ary){
    error13_t ret;

	if(n < 3){
        printf("bad usage, try help '%s'", ary[0]);
        return -1;
	}

	printf("adding %s...", ary[1]);

    ret = acc_user_add(ac, ary[1], ary[2]);
	if(ret != E13_OK){
		printf("failed\n");
		printe13(ret, ary[1]);
		return -1;
	}

	printf("done\n");

    return 0;

}

int do_userrm(struct access13* ac, int n, char** ary){
    error13_t ret;
    uid13_t id;

	if(n < 2){
        printf("bad usage, try help '%s'", ary[0]);
        return -1;
	}

    id = strtoul(ary[1], NULL, 10);

    printf("removing %s[%u]...", id==ULONG_MAX?ary[1]:"-", id==ULONG_MAX?UID13_INVAL:id);

    ret = acc_user_rm(ac, id==ULONG_MAX?ary[1]:NULL, id);
	if(ret != E13_OK){
		printf("failed\n");
		printe13(ret, ary[1]);
		return -1;
	}

	printf("done\n");

    return 0;

}

int do_userset(struct access13* ac, int n, char** ary){
    error13_t ret;
    gid13_t id;
    int stt;
    char* sttstr;

	if(n < 3){
        printf("bad usage, try help '%s'", ary[0]);
        return -1;
	}

    id = strtoul(ary[1], NULL, 10);

    switch(ary[2][0]){
	case 'i':
	case 'I':
		stt = ACC_USR_STT_INACTIVE;
		sttstr = "INACTIVE";
		break;
	case 'r':
	case 'R':
		stt = ACC_USR_STT_REMOVED;
		sttstr = "REMOVED";
		break;
	case 'l':
	case 'L':
		stt = ACC_USR_STT_LOGIN;
		sttstr = "LOGGED IN";
		break;
	case 'o':
	case 'O':
		stt = ACC_USR_STT_LOGOUT;
		sttstr = "LOGGED OUT";
		break;
//	case 'a':
//	case 'A':
//		stt = ACC_USR_STT_ACTIVE;
//		sttstr = "ACTIVE";
//		break;
	default:
		sttstr = "ACTIVE";
//		printe13(e13_error(E13_NOTFOUND), ary[2]);
		break;

    }

    printf("setting status of %s[%u] to %s...",
		id==ULONG_MAX?ary[1]:"-",
		id==ULONG_MAX?GID13_INVAL:id,
		sttstr
		);

    ret = acc_user_set_stat(ac, id==ULONG_MAX?ary[1]:NULL, id, stt);
	if(ret != E13_OK){
		printf("failed\n");
		printe13(ret, ary[1]);
		return -1;
	}

	printf("done\n");

    return 0;

}

int do_userchk(struct access13* ac, int n, char** ary){
    error13_t ret;
    uid13_t id;
    struct user13 user;
    char* sttstr;
    int t13[D13_ITEMS];

	if(n < 2){
        printf("bad usage, try help '%s'", ary[0]);
        return -1;
	}

    id = strtoul(ary[1], NULL, 10);

    printf("checking %s[%u]...", id==ULONG_MAX?ary[1]:"-", id==ULONG_MAX?UID13_INVAL:id);

    ret = acc_user_chk(ac, id==ULONG_MAX?ary[1]:NULL, id, &user);
	if(ret != E13_OK){
		printf("failed\n");
		printe13(ret, ary[1]);
		return -1;
	}

	printf("done\n");

	switch(user.stt){
	case ACC_USR_STT_INACTIVE:
		sttstr = "ACTIVE";
		break;
	case ACC_USR_STT_REMOVED:
		sttstr = "REMOVED";
		break;
	case ACC_USR_STT_LOGIN:
		sttstr = "LOGGED IN";
		break;
	case ACC_USR_STT_LOGOUT:
		sttstr = "LOGGED OUT";
		break;
//	case ACC_USR_STT_ACTIVE:
//		sttstr = "ACTIVE";
//		break;
	default:
		sttstr = "ACTIVE";
		break;
	}

	printf("-- user info --\n");
	printf("name: %s\n", user.name);
	printf("uid: %u\n", user.uid);
	printf("status: %s\n", sttstr);

	d13s_get_jtime(user.lastlogin, t13);
	printf("last login: %u/%u/%u - %u:%u:%u (%u)\n", t13[D13_ITEMS_YEAR], t13[D13_ITEMS_MON], t13[D13_ITEMS_DAY], t13[D13_ITEMS_HOUR], t13[D13_ITEMS_MIN], t13[D13_ITEMS_SEC], user.lastlogin);

	d13s_get_jtime(user.lastlogout, t13);
	printf("last logout: %u/%u/%u - %u:%u:%u (%u)\n",
			t13[D13_ITEMS_YEAR],
			t13[D13_ITEMS_MON],
			t13[D13_ITEMS_DAY],
			t13[D13_ITEMS_HOUR],
			t13[D13_ITEMS_MIN],
			t13[D13_ITEMS_SEC],
			user.lastlogout);

    printf("-- user info --\n");

    return 0;

}

int do_userlist(struct access13* ac, int n, char** ary){

	gid13_t num, i;
	struct user13* userlist, *user;
	error13_t ret;
	char* sttstr;
	int t13[D13_ITEMS];

	printf("getting user list...");

    ret = acc_user_list(ac, &num, &userlist);
	if(ret != E13_OK){
		printf("failed\n");
		printe13(ret, ary[1]);
		return -1;
	}

	printf("done\n");

	if(!num || !userlist){
		printf("-- empty --\n");
		return 0;
	}

	user = userlist;

	printf("-- user list [%u] --\n", num);
	i = 0;
	while(user){
		printf("No.: %u\n", i++);
		printf("name: %s\n", user->name);
		printf("gid: %u\n", user->uid);

		switch(user->stt){
		case ACC_USR_STT_INACTIVE:
			sttstr = "ACTIVE";
			break;
		case ACC_USR_STT_REMOVED:
			sttstr = "REMOVED";
			break;
		case ACC_USR_STT_LOGIN:
			sttstr = "LOGGED IN";
			break;
		case ACC_USR_STT_LOGOUT:
			sttstr = "LOGGED OUT";
			break;
//		case ACC_USR_STT_ACTIVE:
//			sttstr = "ACTIVE";
//			break;
		default:
			sttstr = "ACTIVE";
			break;
		}

		printf("status: %s\n\n", sttstr);

	d13s_get_jtime(user->lastlogin, t13);
	printf("last login: %u/%u/%u - %u:%u:%u (%u)\n",
			t13[D13_ITEMS_YEAR],
			t13[D13_ITEMS_MON],
			t13[D13_ITEMS_DAY],
			t13[D13_ITEMS_HOUR],
			t13[D13_ITEMS_MIN],
			t13[D13_ITEMS_SEC],
			user->lastlogin);

	d13s_get_jtime(user->lastlogout, t13);
	printf("last logout: %u/%u/%u - %u:%u:%u (%u)\n",
			t13[D13_ITEMS_YEAR],
			t13[D13_ITEMS_MON],
			t13[D13_ITEMS_DAY],
			t13[D13_ITEMS_HOUR],
			t13[D13_ITEMS_MIN],
			t13[D13_ITEMS_SEC],
			user->lastlogout);

		user = user->next;
	}
	acc_user_list_free(userlist);
	printf("-- user list --\n");

	return 0;

}

int do_login(struct access13* ac, int n, char** ary){

	error13_t ret;
	uid13_t uid;

	if(n < 3){
        printf("bad usage, try help '%s'", ary[0]);
        return -1;
	}

	printf("logging in %s...", ary[1]);

	ret = acc_user_login(ac, ary[1], ary[2], &uid);

	if(ret != E13_OK){
		printf("failed\n");
		printe13(ret, ary[1]);
		return -1;
	}

	printf("done [uid = %u]...\n", uid);

	return 0;
}

int do_logout(struct access13* ac, int n, char** ary){

	error13_t ret;
	uid13_t id;

	if(n < 2){
        printf("bad usage, try help '%s'", ary[0]);
        return -1;
	}

    id = strtoul(ary[1], NULL, 10);

    printf("logging out %s[%u]...", id==ULONG_MAX?ary[1]:"-", id==ULONG_MAX?UID13_INVAL:id);

	ret = acc_user_logout(ac, id==ULONG_MAX?ary[1]:NULL, id);

	if(ret != E13_OK){
		printf("failed\n");
		printe13(ret, ary[1]);
		return -1;
	}

	printf("done [uid = %u]...\n", id);

	return 0;

}

int do_db_stat(struct access13* ac, int n, char** ary){
	error13_t ret;

	printf("getting db status...");

	printf("\n%s-%s\n", db_isinit(ac->db)?"init":"not init", db_isopen(ac->db)?"open":"not open");

	printf("done\n");

    return 0;
}


struct _cmd cmd[] = {
	{
			CODE_EMPTY,
			{
            "", NULL
			},
			NULL,
			NULL,
			NULL,
	},
	{
			CODE_HELP,
			{
            "help", "?", NULL
			},
			"prints help message",
			"help",
			NULL
	},
	{
			CODE_EXIT,
			{
            "quit", "exit", NULL
			},
			"exit the program",
			"exit",
			NULL

	},
	{
			CODE_OPEN,
			{
            "open", NULL
			},
			"open database",
			"open \'filename\'",
			&do_open
	},
	{
			CODE_CLOSE,
			{
            "close", NULL
			},
			"close database",
			"close",
			&do_close

	},
	{
			CODE_GROUPADD,
			{
            "groupadd", "gadd", NULL
			},
			"add group(s)",
			"groupadd \'groupname1\' \'groupname2\' ...",
			&do_groupadd

	},

	{
			CODE_RMGROUP,
			{
            "rmgroup", "grm", "grouprm", "rmg", "delgroup", "groupdel", "gdel", "delg", NULL
			},
			"removes group(s)",
			"rmgroup \'groupname1\' \'groupname2\' ...",
			&do_grouprm

	},
	{
			CODE_GROUPSET,
			{
            "groupset", "gset", NULL
			},
			"sets group config",
			"groupset \'groupname\' \'stat=(A)ctive|(I)nactive|(R)emoved\'",
			&do_groupset

	},
	{
			CODE_GROUPLIST,
			{
            "grouplist", "glist", NULL
			},
			"lists groups",
			"grouplist",
			&do_grouplist

	},
	{
			CODE_USERADD,
			{
            "useradd", "uadd", NULL
			},
			"add user(s)",
			"useradd \'username1\' \'username2\' ...",
			&do_useradd

	},

	{
			CODE_RMUSER,
			{
            "rmuser", "urm", "userrm", "rmu", "deluser", "userdel", "udel", "delu", NULL
			},
			"removes user(s)",
			"rmuser \'username1\' \'username2\' ...",
			&do_userrm

	},
	{
			CODE_USERSET,
			{
            "userset", "uset", NULL
			},
			"sets user config",
			"userset \'username\' \'stat=(A)ctive|(I)nactive|(R)emoved|(L)oggedIn|(L)oggedOut\'",
			&do_userset

	},
	{
			CODE_USERLIST,
			{
            "userlist", "ulist", NULL
			},
			"lists users",
			"userlist",
			&do_userlist

	},
	{
			CODE_LOGIN,
			{
            "login", "lin", NULL
			},
			"login user",
			"login \'username1\' \'password1\' \'username2\' \'password2\' ...",
			&do_login

	},
	{
			CODE_LOGOUT,
			{
            "logout", "lout", NULL
			},
			"logout user",
			"logout \'username1\' \'username2\' ...",
			&do_logout

	},
	{
			CODE_USER_JOIN_GROUP,
			{
            "groupjoin", "gjoin", NULL
			},
			"joins user to group(s)",
			"groupjoin \'username\' \'group1\' \'group2\' ...",
			NULL

	},
	{
			CODE_USER_LEAVE_GROUP,
			{
            "groupleave", "gleave", NULL
			},
			"leave user from group(s)",
			"groupleave \'username\' \'group1\' \'group2\' ...",
			NULL

	},
	{
			CODE_USER_GROUP_CHK,
			{
            "groupcheck", "gcheck", "gchk", NULL
			},
			"checks user membership in group(s)",
			"groupcheck \'username\' \'group1\' \'group2\' ...",
			NULL

	},
	{
			CODE_USER_GROUP_LIST,
			{
            "usergrouplist", "uglist", NULL
			},
			"list user group(s)",
			"usergrouplist \'username\' ...",
			NULL

	},
	{
			CODE_GROUP_USER_LIST,
			{
            "groupuserlist", "gulist", NULL
			},
			"list users in a group",
			"groupuserlist \'groupname\' ...",
			NULL
	},
	{
			CODE_PERM_GROUP_ADD,
			{
            "permgroupadd", "pgadd", NULL
			},
			"add group permission",
			"permgroupadd \'groupname\' \'object_id\' \'perm=(R)ead|(W)rite|e(X)ecute\'",
			NULL

	},
	{
			CODE_PERM_GROUP_RM,
			{
            "permgrouprm", "pgrm", NULL
			},
			"remove group permission",
			"permgrouprm \'groupname\' \'object_id\' \'perm=(R)ead|(W)rite|e(X)ecute\'",
			NULL

	},
	{
			CODE_PERM_GROUP_CHK,
			{
            "permgroupcheck", "pgchk", NULL
			},
			"check group permission",
			"permgroupcheck \'groupname\' \'object_id\' \'perm=(R)ead|(W)rite|e(X)ecute|(L)ist\'",
			NULL

	},
	{
			CODE_USER_ACCESS,
			{
            "access", "acc", NULL
			},
			"check user access",
			"access \'username\' \'object_id\' \'perm=(R)ead|(W)rite|e(X)ecute\'",
			NULL

	},
	{
			CODE_DB_STAT,
			{
            "dbstat", NULL
			},
			"check db status",
			"dbstat",
			&do_db_stat

	},
	{
		CODE_INVAL,
		{NULL},
		NULL,
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
//            printf("[%i]:%s\n", i, c->cmd[i]);
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
//            printf("[%i]:%s\n", i, c->cmd[i]);
			if(!strcmp(c->cmd[i], str)) return c->code;
			i++;
		}
	}
	return CODE_INVAL;
}

int help(struct access13* ac, int n, char** ary){
	struct _cmd* c;
	int i;

	if(n > 1){
		c = translate_s(ary[1]);

		i = 0;
		while(c->cmd[i]){
			printf("\nforms: ");
			printf("%s, ", c->cmd[i]);
			i++;
		}

		printf("\ndescription: %s", c->desc);
		printf("\nsyntax: %s\n", c->syntax);

		return 0;
	}

	printf("\n-- access13 console help --\n");
	for(c = cmd; c->cmd[0]; c++){
		i = 0;
		printf("\nforms: ");
		while(c->cmd[i]){
			printf("%s, ", c->cmd[i]);
			i++;
		}
	}

	return 0;
}

int main(int argc, char* argv[])
{
	char input[MAX_INPUT];
	int n;
	char** ary;
	struct access13 ac;
	struct _cmd* c;

	escape = ESCAPE;
	strcpy(delim, DELIM);
    pack[0] = PACK1;
    pack[1] = PACK2;
    pack[2] = NULL;

    printf("-- access13 console --\n");

show_prompt:
	prompt("access", input);

    n = s13_exparts(input, delim, pack, escape);

    ary = s13_exmem(n);

    s13_explode(input, delim, pack, escape, ary);

	if(!(c = translate_s(ary[0]))){
		printe13(e13_error(E13_SYNTAX), ary[0]);
	} else {

		switch(c->code){
		case CODE_HELP:
			help(&ac, n, ary);
		break;
		case CODE_EXIT:
			return 0;
			break;
		default:
			if(!c->func){
				printe13(e13_error(E13_IMPLEMENT), ary[0]);
			} else {
				c->func(&ac, n, ary);
			}
			break;
		}
	}

	s13_free_exmem(ary);

	goto show_prompt;

    return 0;
}
