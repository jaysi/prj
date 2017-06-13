#include "include/ilink_intern.h"

//#define _deb_sel _NullMsg
//#define _deb_rm _NullMsg
//#define _getkey _NullMsg//_GetKey

#define ilink_poll_lock(list)    MACRO( if((list)->mx) th13_mutex_lock( (list)->mx ); )
#define _assert_fdcount _AssertMacro

#ifdef ILINK_POLL_USE_SELECT
#include "ilink_poll_select.c"
#endif // ILINK_POLL_USE_SELECT

#ifdef ILINK_POLL_USE_POLL
#include "ilink_poll_poll.c"
#endif

#ifdef ILINK_POLL_USE_EPOLL
#include "ilink_poll_epoll.c"
#endif
