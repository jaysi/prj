#ifndef _JDB_LOCK_H
#define _JDB_LOCK_H
	
#define _jdb_lock_bid_list(h, bid, n, lock)
#define _jdb_relock_bid_list(h, bid, n, lock)
#define _jdb_unlock_bid_list(h, bid, n, lock)

#define JDB_LK_RD 0
#define JDB_LK_WR 1

/*
	handle locks
	first 8 bits for read access
*/

#define JDB_LK_NONE	0x0000	//used for refrence counting

#define JDB_LK_MAP	(0x0001<<0)
#define JDB_LK_HDR	(0x0001<<1)
#define JDB_LK_TABLE	(0x0001<<2)
#define JDB_LK_INDEX0	(0x0001<<3)
#define JDB_LK_SNAP	(0x0001<<4)
#define JDB_LK_CONF	(0x0001<<5)
#define JDB_LK_BASE	(0x0001<<6)
#define JDB_LK_UNDO	(0x0001<<7)

#define JDB_LK_ALL	0xffff

int _jdb_init_handle_lock(struct jdb_handle *handle);
int _jdb_destroy_handle_lock(struct jdb_handle *handle);
void _jdb_unset_handle_flag(struct jdb_handle *h, uint16_t flag, int lock);
void _jdb_set_handle_flag(struct jdb_handle *h, uint16_t flag, int lock);
jdb_hflag_t _jdb_get_handle_flag(struct jdb_handle *h, int lock);
void _jdb_lock_handle(struct jdb_handle* h, jdb_hlock_t lock, int lock_type);
void _jdb_unlock_handle(struct jdb_handle* h, jdb_hlock_t lock, int lock_type);
int _jdb_init_table_lock(struct jdb_table *table);
int _jdb_destroy_table_lock(struct jdb_table *table);
int _jdb_lock_table(	struct jdb_handle* h, wchar_t* tname,
			int lock, struct jdb_table** table
		);
int _jdb_unlock_table(	struct jdb_handle* h, wchar_t* tname,
			int lock, struct jdb_table** table
		);


#else

#endif
