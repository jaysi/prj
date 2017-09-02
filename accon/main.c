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
	CODE_INVAL
};

struct _cmd {
    enum cmd_code code;
    char* cmd[10];
    char* desc;
    char* usage;
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
            "?", "help", NULL
			},
			"prints help message",
			"type it!"

	},
	{
			CODE_EXIT,
			{
            "quit", "exit", NULL
			},
			"exit the program",
			"type it!"

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
		printo("\nusage: %s\n", c->usage);
	}
	return 0;
}

int main(int argc, char* argv[])
{
	char input[MAX_INPUT];
	char cmd[MAX_CMD];
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
	default:
		printo("unknown input %s\n", input);
		break;
	}

	goto show_prompt;

    return 0;
}
