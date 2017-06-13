#include "include/ilink_intern.h"

#define _deb1 _NullMsg

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

error13_t ilink_get_info(struct infolink* link,
                              struct infolink_inf* info){

    return E13_IMPLEMENT;
};


error13_t _ilink_init_pcap_sock(struct infolink *link){
    return e13_error(E13_IMPLEMENT);
}

error13_t _ilink_init_sock(struct infolink *link,
                           char* port,
                           int family,
                           int socktype) {
     //    ilink_sock_t link->sock, new_fd;
     // listen on sock_fd, new connection on new_fd
     struct addrinfo hints, *servinfo, *p;
     #ifndef WIN32
         struct sigaction sa;
         int yes=1;
     #else
         char yes=1;
     #endif
         int rv;

         link->e = (struct e13*)malloc(sizeof(struct e13));
         if(!link->e) return e13_error(E13_NOMEM);

         if(e13_init(link->e, E13_MAX_WARN_DEF, E13_MAX_ESTR_DEF, LIB13_ILINK) != E13_OK){
             free(link->e);
             link->e = NULL;
             return e13_error(E13_ERROR);
         }

//         if(!accept_callback){
//             return e13_uerror(link->e, E13_MISUSE, "s", "need callback");
//         }

     #ifdef _WIN32
         WSADATA wsaData;   // if this doesn't work
         //WSAData wsaData; // then try this instead

         // MAKEWORD(1,1) for Winsock 1.1, MAKEWORD(2,0) for Winsock 2.0:

         if (WSAStartup(MAKEWORD(2,0), &wsaData) != 0) {
             _deb1("%s", strerror(errno));
             return e13_ierror(link->e, E13_SYSE, "s", "wsastartup()");
         }

     #endif

         memset(&hints, 0, sizeof hints);
         hints.ai_family = family;
         hints.ai_socktype = socktype;
         hints.ai_flags = AI_PASSIVE; // use my IP

         if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
             _deb1("%s", strerror(errno));
             return e13_ierror(link->e, E13_SYSE, "s", gai_strerror(rv));
         }

         // loop through all the results and bind to the first we can
         for(p = servinfo; p != NULL; p = p->ai_next) {
             if ((link->sock = socket(p->ai_family, p->ai_socktype,
                     p->ai_protocol)) == -1) {
                 e13_warn(link->e, E13_SYSE, "s", "socket()");
                 _deb1("%s", strerror(errno));
                 continue;
             }


             if (setsockopt(link->sock, SOL_SOCKET, SO_REUSEADDR, &yes,
                     sizeof(int)) == -1) {
                 freeaddrinfo(servinfo); // all done with this structure
                 _deb1("%s", strerror(errno));
                 return e13_ierror(link->e, E13_SYSE, "s", "setsockopt()");
             }

             if (bind(link->sock, p->ai_addr, p->ai_addrlen) == -1) {
                 close(link->sock);
                 e13_warn(link->e, E13_SYSE, "s", "bind()");
                 _deb1("%s", strerror(errno));
                 continue;
             }

             break;
         }

         if (p == NULL)  {
             freeaddrinfo(servinfo); // all done with this structure
             _deb1("%s", strerror(errno));
             return e13_ierror(link->e, E13_SYSE, "s", "bind()");
         }

         freeaddrinfo(servinfo); // all done with this structure

         _deb1("OK");

         return E13_OK;
}


