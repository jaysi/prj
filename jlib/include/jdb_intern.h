#ifndef _JDB_INTERN_H
#define _JDB_INTERN_H

#include "hardio.h"
#include "sha256.h"
#include "aes.h"
#include "jer.h"
#include "debug.h"
#include "jhash.h"
#include "jbits.h"
#include "debug.h"
#include "jtime.h"
#include "jdb_types.h"
#include "jdb_intern.h"
#include "jdb_diff.h"
#include "jdb_list.h"
#include "jdb_lock.h"
#include "jdb_mem.h"
#include "jtable.h"
#include "jcs.h"
#include "jdb_debug.h"
#include "jcrypt.h"
#define _J_USE_PTHREAD
#include "jcompat.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>

#define JDB_HDR_SIZE	1024

/*
struct JDBConstHdr{
	uint16_t magic;
	uchar type;
	uchar ver;
};
*/

struct jdb_handle;

#define PWHASHSIZE	32//sha256

struct jdb_hdr {

	/*
		the RTC marked elements are run-time constant and do not need
		locking on run-time, handle refrence count is needed, anyway...
		
		others may be locked through the LK_HDR
	*/

	//fixed
	uint16_t magic;
	uchar type;
	uint32_t ver;

	//configuration
	jdb_hflag_t flags;//RTC
	uchar crc_type;//RTC
	uchar crypt_type;//RTC

	uchar wr_retries;
	uchar rd_retries;

	jdb_bsize_t blocksize;	/*must be multiplier of 2^10 (1024) */ //RTC

	//limits
	jdb_bid_t max_blocks;//RTC

	//block entries
	jdb_bent_t map_bent;//RTC
	jdb_bent_t typedef_bent;//RTC
	jdb_bent_t col_typedef_bent;//RTC
	jdb_bent_t celldef_bent;//RTC
	jdb_bent_t index1_bent;//RTC
	jdb_bent_t dptr_bent;//RTC
	jdb_bent_t fav_bent;//RTC
	jdb_bent_t index0_bent;//RTC
	jdb_bent_t cb_size;//crypt block size, (AES/DES3)_BSIZE, //RTC
	
	jdb_bid_t map_list_buck;
	jdb_bid_t list_buck;
	jdb_bid_t fav_load;

	//status
	jdb_bid_t nblocks;	//number of blocks
	jdb_bid_t nmaps;	//number of map blocks
	jdb_tid_t ntables;
	uint64_t chid;		//change id
	
	uchar pwhash[PWHASHSIZE];	/*sha256 */ //RTC
	
	//PAD
		
	uint32_t crc32;		//header crc
}__attribute__((packed));

/*			Table-Definition			*/

struct jdb_table {
	jdb_bid_t table_def_bid;

	jdb_bid_t nwrblk;
	jdb_bid_t map_chg_list_size, map_chg_ptr;
	jdb_bid_t* map_chg_list;
	
	jmx_t lock_mx;
	jcond_t lock_cond;
	unsigned int lock_wr;
	unsigned int lock_rd;
		
	struct jdb_table_def_blk main;
	struct jdb_col_typedef_list col_typedef_list;
	struct jdb_typedef_list typedef_list;
	struct jdb_celldef_list celldef_list;
	struct jdb_cell_list cell_list;
	struct jdb_index1_list index1_list;
	struct jdb_index2_list index2_list;
	struct jdb_cell_data_ptr_list data_ptr_list;
	struct jdb_cell_data_list data_list;
	struct jdb_fav_blk_list fav_list;
	struct jdb_table *next;
};

struct jdb_table_list {
	jdb_tid_t cnt;
	struct jdb_table *first;
	struct jdb_table *last;
};

struct jdb_wr_fifo_entry{
	jdb_bid_t nblocks;
	jdb_bid_t* bid_list;
	uint32_t bufsize;
	uchar* hdrbuf;
	uchar* buf;
	struct jdb_wr_fifo_entry* next;
};

struct jdb_wr_fifo{
	uint32_t cnt;
	struct jdb_wr_fifo_entry* first, *last;
};

/*
	init / cleanup
*/
int _jdb_cleanup_handle(struct jdb_handle *h);

/*
	packing, they set/check crc too!
*/
int _jdb_pack_hdr(struct jdb_hdr *hdr, uchar * buf);
int _jdb_unpack_hdr(struct jdb_hdr *hdr, uchar * buf);
int _jdb_pack_map(struct jdb_handle* h, struct jdb_map *blk, uchar * buf);
int _jdb_unpack_map(struct jdb_handle* h, struct jdb_map *blk, uchar * buf);
int _jdb_pack_typedef(struct jdb_handle* h, struct jdb_typedef_blk *blk, uchar * buf);
int _jdb_unpack_typedef(struct jdb_handle* h, struct jdb_typedef_blk *blk, uchar * buf);
int _jdb_pack_celldef(struct jdb_handle* h, struct jdb_celldef_blk *blk, uchar * buf);
int _jdb_unpack_celldef(struct jdb_handle* h, struct jdb_celldef_blk *blk, uchar * buf);
int _jdb_pack_table_def(struct jdb_handle* h, struct jdb_table_def_blk *blk, uchar * buf);
int _jdb_unpack_table_def(struct jdb_handle* h, struct jdb_table_def_blk *blk, uchar * buf);
int _jdb_pack_col_typedef(struct jdb_handle* h, struct jdb_col_typedef *blk, uchar * buf);
int _jdb_unpack_col_typedef(struct jdb_handle* h, struct jdb_col_typedef *blk, uchar * buf);
int _jdb_pack_data(	struct jdb_handle* h, struct jdb_cell_data_blk *blk,
			uchar * buf);
int _jdb_unpack_data(	struct jdb_handle* h, struct jdb_table* table,
			struct jdb_cell_data_blk *blk, jdb_data_t type_id,
			uchar * buf);
int _jdb_unpack_data_ptr(struct jdb_handle* h, struct jdb_cell_data_ptr_blk *blk,
			uchar* buf);
int _jdb_pack_data_ptr(struct jdb_handle* h, struct jdb_cell_data_ptr_blk *blk,
			uchar* buf);
int _jdb_unpack_fav(struct jdb_handle* h, struct jdb_fav_blk *blk,
			uchar * buf);
int _jdb_pack_fav(struct jdb_handle* h, struct jdb_fav_blk *blk,
			uchar * buf);

/*
	hashing
*/
uint32_t _jdb_hash(unsigned char *key, size_t key_len);
uchar _jdb_pearson(unsigned char *key, size_t len);
#define _jdb_crc32(key, size) _jdb_hash(key, size)
#define _jdb_key32(key, size) _jdb_hash(key, size)

/*
	read/write
*/
#define JDB_WR_SHUTDOWN	(0x01<<0)	//the db is shutting down, do not use AIO and you may change the buffers' contents
#define JDB_WR_NONEW	(0x01<<1)	//do not allocate new memmory (i.e. you may change the buffer contents and it will be valid till the write is complete.

int _jdb_read_hdr(struct jdb_handle *h);
int _jdb_write_hdr(struct jdb_handle *h);

/*
	access control
*/
void _jdb_fill_date(uchar d[3], uchar t[3]);

/*
	handle
*/
void _jdb_unset_handle_flag(struct jdb_handle *h, uint16_t flag, int lock);
void _jdb_set_handle_flag(struct jdb_handle *h, uint16_t flag, int lock);
uint16_t _jdb_get_handle_flag(struct jdb_handle *h, int lock);
uint64_t _jdb_get_chid(struct jdb_handle *h, int lock);

/*
	header
*/
int _jdb_hdr_to_buf(struct jdb_handle* h, uchar** buf, int alloc);


/*
	crypto
*/
int _jdb_decrypt(struct jdb_handle *h, uchar * src, uchar * dst,
		 size_t bufsize);
int _jdb_encrypt(struct jdb_handle *h, uchar * src, uchar * dst,
		 size_t bufsize);

/*
	allocation
*/
//#define _jdb_blk_off(n, hsize, bsize) ((n)==0?hsize:((hsize) + (((n)-1)*(bsize))))

  /*
     jdb_bid_t bid;
     don't need this, there is a formula to find the bid if there is
     a map entry for each block, that is:
     assuming if map_order = 0 then map_bid = 0;
     map_bid = map_order*(map_entry_capacity + 1);
     bid = map_bid + blk_entry_inside_the_map;
   */

//#define _jdb_calc_bid(map_no, map_bent_capacity, map_bent) ((map_no*(map_bent_capacity+1))+map_bent)
jdb_bid_t _jdb_find_table_def_bid(struct jdb_handle *h, uint32_t key);

/*
	type-size calculators/extractors
*/
size_t _jdb_base_dtype_size(uchar dtype);
size_t _jdb_data_len(struct jdb_handle *h, struct jdb_table *table,
		      uchar dtype, uchar * base_type, size_t * base_len);
size_t _jdb_typedef_len(struct jdb_handle *h, struct jdb_table *table,
			jdb_data_t type_id, jdb_data_t * base_type,
			size_t * base_len);

/*
	mapping
*/

int _jdb_free_map_list(struct jdb_handle *h);
jdb_bent_t _jdb_max_nful(struct jdb_handle *h, jdb_blk_t btype);
int _jdb_create_map(struct jdb_handle *h);
int _jdb_set_map_entry(struct jdb_handle *h,
		       struct jdb_map *map,
		       jdb_bent_t map_bent,
		       jdb_blk_t btype,
		       jdb_data_t dtype, jdb_tid_t tid, jdb_bent_t nful);
int _jdb_set_map_nful_by_bid(struct jdb_handle *h, jdb_bid_t bid,
			     jdb_bent_t nful);
int _jdb_inc_map_nful(struct jdb_handle *h, struct jdb_map *map,
		      jdb_bent_t map_bent);
int _jdb_dec_map_nful(struct jdb_handle *h, struct jdb_map *map,
		      jdb_bent_t map_bent);
int _jdb_get_map_nful_by_bid(struct jdb_handle *h, jdb_bid_t bid, jdb_bent_t* nful);		      
jdb_bid_t _jdb_get_empty_map_entry(struct jdb_handle *h, jdb_blk_t btype,
				   jdb_data_t dtype, jdb_tid_t tid,
				   jdb_bent_t nful);
/*
int _jdb_get_empty_map_entries(struct jdb_handle *h, jdb_blk_t * btype,
			       jdb_data_t * dtype, jdb_tid_t * tid,
			       jdb_bent_t * nful, jdb_bid_t * bid, jdb_bid_t n);
*/
int _jdb_rm_map_bid_entries(struct jdb_handle *h, jdb_bid_t * bid, jdb_bid_t n);
int _jdb_write_map(struct jdb_handle *h);
int _jdb_load_map(struct jdb_handle *h);
int _jdb_inc_map_nful_by_bid(struct jdb_handle *h, jdb_bid_t bid, jdb_bent_t n);
int _jdb_dec_map_nful_by_bid(struct jdb_handle *h, jdb_bid_t bid, jdb_bent_t n);
int _jdb_list_map_match(struct jdb_handle *h, jdb_blk_t btype,
			jdb_data_t dtype, jdb_tid_t tid, jdb_bent_t nful,
			uchar cmp_flags, jdb_bid_t ** bid, jdb_bid_t * n);
jdb_bid_t _jdb_find_first_map_match(struct jdb_handle *h, jdb_blk_t btype,
				    jdb_data_t dtype, jdb_tid_t tid,
				    jdb_bent_t nful, uchar cmp_flags);
//struct jdb_map_blk_entry* _jdb_get_map_entry_ptr(struct jdb_handle* h, jdb_bid_t bid);
int _jdb_map_chg_proc(struct jdb_handle* h, jdb_bid_t bid, struct jdb_table* table, jdb_bid_t map_bid);
jdb_dtype_t _jdb_get_block_dtype(struct jdb_handle* h, jdb_bid_t bid);
int _jdb_rm_map_entries_by_tid(struct jdb_handle *h, jdb_tid_t tid);
int _jdb_rm_map_bid_entry(struct jdb_handle *h, jdb_bid_t bid);

/*
	types
*/
int _jdb_load_col_typedef(struct jdb_handle *h, struct jdb_table *table);
int _jdb_find_typedef(struct jdb_handle *h,
			    struct jdb_table *table, uchar type_id,
			    struct jdb_typedef_blk **blk,
			    struct jdb_typedef_blk_entry **entry);
int _jdb_load_typedef(struct jdb_handle *h, struct jdb_table *table);
void _jdb_free_typedef_list(struct jdb_table *table);
void _jdb_free_col_typedef_list(struct jdb_table *table);			    

/*
	data and data pointers
*/

int _jdb_create_dptr_chain(struct jdb_handle* h, struct jdb_table* table,
				struct jdb_cell_data_ptr_blk_entry** list,
				jdb_bid_t needed,
				jdb_bid_t* first_bid, jdb_bent_t* first_bent);
int _jdb_load_dptr_chain(	struct jdb_handle* h, struct jdb_table* table,
				struct jdb_cell* cell,
				struct jdb_cell_data_ptr_blk_entry** list);

int _jdb_load_all_cell_data(struct jdb_handle* h, struct jdb_table* table);

int _jdb_alloc_cell_data(struct jdb_handle *h, struct jdb_table *table,
		     struct jdb_cell *cell, jdb_data_t dtype,
		     uchar* data, size_t datalen, int first);

void _jdb_free_vdata_list(struct jdb_handle* h, struct jdb_table *table);
void _jdb_free_data_list(struct jdb_handle* h, struct jdb_table *table);
void _jdb_free_cell_list(struct jdb_handle* h, struct jdb_table* table);
void _jdb_free_celldef_list(struct jdb_handle* h, struct jdb_table *table);
int _jdb_free_data_ptr_list(struct jdb_handle* h, struct jdb_table *table);
int _jdb_load_cell_data(struct jdb_handle* h, struct jdb_table* table, struct jdb_cell* cell);
int _jdb_add_fdata(struct jdb_handle *h, struct jdb_table *table, uchar dtype,
			uchar * data, jdb_bid_t* databid, jdb_bent_t* databent);
int _jdb_rm_data_seg(struct jdb_handle *h, struct jdb_table *table,
	      jdb_bid_t bid, jdb_bent_t bent, jdb_bent_t nent);															
int _jdb_rm_dptr_chain(		struct jdb_handle* h, struct jdb_table* table,
				struct jdb_cell* cell,
				jdb_bid_t bid, jdb_bent_t bent);
int _jdb_load_data_ptr(struct jdb_handle *h, struct jdb_table *table);
						     

/*
	search
*/
int _jdb_table_handle(struct jdb_handle* h, wchar_t* name, struct jdb_table** table);


/*
	threads
*/
int _jdb_init_wr_thread(struct jdb_handle* h);
int _jdb_request_wr_thread_exit(struct jdb_handle* h);
int _jdb_request_table_write(struct jdb_handle* h, struct jdb_table* table);


int _jdb_rm_fav(struct jdb_handle *h, struct jdb_table* table, jdb_bid_t bid);
int _jdb_typedef_flags_by_ptr(struct jdb_handle* h, struct jdb_table *table, jdb_data_t type_id, uchar* flags);
int _jdb_inc_fav(struct jdb_handle *h, struct jdb_table* table,
		    jdb_bid_t bid);
int _jdb_load_celldef(struct jdb_handle *h, struct jdb_table *table);
int _jdb_load_fav(struct jdb_handle *h, struct jdb_table *table);
int _jdb_load_fav_blocks(struct jdb_handle* h, struct jdb_table* table);
void _jdb_free_fav_list(struct jdb_table *table);
int _jdb_load_cells(struct jdb_handle *h, struct jdb_table *table, int loaddata);

int _jdb_changelog_reg(struct jdb_handle* h, struct jdb_changlog_rec* rec);
int _jdb_changelog_reg_end(struct jdb_handle* h, struct jdb_changlog_rec* rec,
				int ret);			
int _jdb_changelog_assm_argbuf(uchar** buf, size_t* bufsize, uchar nargs,
				size_t* argsize, ...);
int _jdb_changelog_assm_rec(	struct jdb_changelog_rec* rec, uint64_t chid,
				uchar cmd, int ret, uchar nargs,
				size_t* argsize, ...);

int _jdb_jrnl_recover(struct jdb_handle* h);
int _jdb_jrnl_open(struct jdb_handle *h, wchar_t * filename, uint16_t flags);

int _jdb_init_handle_lock(struct jdb_handle *handle);
int _jdb_destroy_handle_lock(struct jdb_handle *handle);
void _jdb_lock_handle(struct jdb_handle* h, jdb_hlock_t lock, int lock_type);
void _jdb_unlock_handle(struct jdb_handle* h, jdb_hlock_t lock, int lock_type);
int _jdb_init_table_lock(struct jdb_table *table);
int _jdb_destroy_table_lock(struct jdb_table *table);
int _jdb_lock_table(	struct jdb_handle* h, wchar_t* tname,
			int lock, struct jdb_table** table);
int _jdb_unlock_table(	struct jdb_handle* h, wchar_t* tname,
			int lock, struct jdb_table** table);

uint64_t _jdb_get_chid(struct jdb_handle *h, int lock);


/*
	MACROs
*/

#define _jdb_blk_off(n, hsize, bsize) ((hsize) + ((n)*(bsize)))

#define _jdb_calc_bid(M_map_bid, M_map_bent) (M_map_bid + M_map_bent + 1)

#define _JDB_MAP_ORDER(bid, map_bent) ((bid)/((map_bent)+1))
#define _JDB_MAP_BID(bid, map_bent) (_JDB_MAP_ORDER(bid, map_bent)*((map_bent)+1))
#define _JDB_MAP_BENT(bid, map_bent) (((bid)%((map_bent)+1))-1)

#define JDB_MAP_CHG_LIST_BUCK	10

#define _JDB_SET_WR(h, blk, bid, table, map_chg_flag)\
	_wdeb(L"_JDB_SET_WR->bid= %u (IS different with map bid), chg_flag = %i, blk->write = %u", bid, map_chg_flag, (blk)->write);\
	if((blk)->write){\
		(blk)->write++;\
		if(map_chg_flag) _jdb_map_chg_proc(h, bid, table, _JDB_MAP_BID(bid, h->hdr.map_bent));\
	} else {\
		(blk)->write = 1;\
		table->nwrblk++;\
		if(map_chg_flag) _jdb_map_chg_proc(h, bid, table, _JDB_MAP_BID(bid, h->hdr.map_bent));\
	}


#endif
