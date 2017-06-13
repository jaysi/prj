#ifndef ILINK_TYPES_H
#define ILINK_TYPES_H

#include "../../lib13/include/lib13.h"

//#ifndef __WIN32
//#include <unistd.h>
//#include <sys/socket.h>
//#include <arpa/inet.h>
//#include <netinet/in.h>
//#include <netdb.h>
//#else
//#include <ws2tcpip.h>
//#include <winsock2.h>
//#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

#ifdef linux
#include <error.h>
#include <sys/wait.h>
#endif

#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#ifndef _WIN32
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#else
#include <w32api.h>
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501 //at least works on xp!
#include <winsock2.h>
#include <ws2tcpip.h>
#include <ws2spi.h>
#endif

#ifndef _WIN32
typedef int ilink_sock_t;
#else
typedef SOCKET ilink_sock_t;
#endif

typedef uint32_t ilink_pkt_id_t;
typedef uint32_t ilink_datalink_t;
typedef uint32_t ilink_datalen_t;
typedef uint32_t ilink_transport_t;
typedef uint8_t ilink_handshake_t;
typedef char ilink_data_t;
typedef uint32_t ilink_timeout_t;//in milliseconds
typedef uint8_t ilink_poll_flag_t;
typedef uint16_t ilink_pkt_type_t;
typedef uint8_t ilink_ppkt_flag_t;
typedef uint16_t ilink_flag_t;
typedef uchar ilink_udata_t;

#endif //ILINK_TYPES_H
