#ifndef _JDB_MEM_H
#define _JDB_MEM_H

#define JDB_MEM_DONT_LOCK_H		(0x01<<0)

#define _jdb_get_blockbuf(h, btype, flags, extra) malloc(h->hdr.blocksize + (extra))
#define _jdb_release_blockbuf(h, buf, flags) free(buf);

#else

#endif
