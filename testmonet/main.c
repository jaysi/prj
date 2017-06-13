#include <stdio.h>

#include "main.h"

int svr();
int clt();

int main(void)
{
    char choice[16];

choice_label:
    printo("[c]lient / [s]erver / [e]xit:");
    scani("%s", choice);

    if(!strcasecmp(choice, "client") || !strcasecmp(choice, "c")) {
        return clt();
    } else if(!strcasecmp(choice, "server") || !strcasecmp(choice, "s")) {
        return svr();
    } else if(!strcasecmp(choice, "exit") || !strcasecmp(choice, "x") || !strcasecmp(choice, "e")) {
        return 0;
    } else {
        printo("use one of the choices provided.\n");
        goto choice_label;
    }

    return 0;
}

