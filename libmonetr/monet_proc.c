#include "monet_internal.h"

//this is heart of the monet-stacked-io

void* _monet_proc_thread(void* arg){

    struct _monet_poll_thread_arg* args = arg;
    struct monet* mn = args->mn;
    struct monet_fifo_entry* entry;

    while(1){
        th13_sem_wait(mn->wait_sem);
        th13_mutex_lock(&mn->req_fifo.mx);
        entry = mn->req_fifo->first;
        mn->req_fifo->first = mn->req_fifo->first->next;
        mn->req_fifo.n--;
        th13_mutex_unlock(&mn->req_fifo.mx);
        //process entry

    }

    return NULL;
}
