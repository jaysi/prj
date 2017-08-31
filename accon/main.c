#include <stdio.h>
#include <stdlib.h>

#include "../lib13/include/acc13.h"
#define printo printf

enum cmd_code {
	CODE_EMPTY,
	CODE_HELP,
	CODE_INVAL
};

struct _cmd {
	char* cmd[];
    enum cmd_code code;
} cmd[] = {
	{
		{
            "", NULL
		},
			CODE_EMPTY
	},
	{
		{
            "?", "help", NULL
		},
			CODE_HELP
	},
	{
		NULL,
		CODE_INVAL
	}
};

enum cmd_code translate(char* str){
	struct _cmd* c;
	char* s;
	for(c = cmd; c->cmd; c++){
		for(s = c->cmd; s; s++){
			if(!strcmp(s, str)) return c->code;
		}
	}
	return CODE_INVAL;
}

int main()
{
    printo("-- access13 console --\n");
    printo("")
    return 0;
}
