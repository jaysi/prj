#ifndef _JDB_DIFF_H
#define _JDB_DIFF_H

#include "jbits.h"

#define JDB_CHANGE_BMAP_SIZE(nentry) ( (nentry) % 8 )?( ( (nentry) / 8 ) + 1 ):( (nentry) / 8 )

#define _jdb_alloc_change_bmap(nentry) malloc(JDB_CHANGE_BMAP_SIZE(nentry))

#define _jdb_track_change(change_bmap, entryi) set_bit(change_bmap, entryi)
#define _jdb_untrack_change(change_bmap, entryi) unset_bit(change_bmap, entryi)
#define _jdb_test_change(change_bmap, entryi) test_bit(change_bmap, entryi)

#else

#endif
