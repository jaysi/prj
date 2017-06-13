#ifndef _JDB_BLOCK_H
#define _JDB_BLOCK_H

//#include "jdb_blocki.c"

//block types
#define JDB_BTYPE_EMPTY		0x00
#define JDB_BTYPE_MAP		0x01
#define JDB_BTYPE_TYPE_DEF	0x02
#define JDB_BTYPE_TABLE_DEF	0x03
#define JDB_BTYPE_COL_TYPEDEF	0x04
#define JDB_BTYPE_CELLDEF	0x05
#define JDB_BTYPE_CELL_DATA	0x06	/*not a real type, just used for
					  memmory allocations*/
#define JDB_BTYPE_CELL_DATA_VAR	0x07
#define JDB_BTYPE_CELL_DATA_FIX 0x08
#define JDB_BTYPE_CELL_DATA_PTR	0x09
#define JDB_BTYPE_INDEX		0x0a	/*index, see map_entry's dtype field*/
#define JDB_BTYPE_TABLE_FAV	0x0b	/*block-rating*/
#define JDB_BTYPE_SNAPSHOT	0x0c	/*snapshot block*/
#define JDB_BTYPE_VOID		0xff	/*void blocks type, mainly for
					   memmory allocations */

#define JDB_MAP_CMP_BTYPE	(0x01<<0)
#define JDB_MAP_CMP_DTYPE	(0x01<<1)
#define JDB_MAP_CMP_TID		(0x01<<2)
#define JDB_MAP_CMP_NFUL	(0x01<<3)	/* this flag is different from
						   other flags as long as the
						   map entry value must be less
						   than the requested value */

#define JDB_MIN_BLK_SIZE	1024
#define JDB_MAX_BLK_SIZE	65536
#define JDB_DEF_BLK_SIZE	4096

/*
	map blocks only address table_def, cell_main and index blocks
*/
struct jdb_map_blk_hdr {
	uchar type;
	uchar flags;

	jdb_bent_t nset;	//number of entries set
	uint32_t crc32;		//crc of the block including header
}__attribute__((packed));

struct jdb_map_blk_entry {
	jdb_blk_t blk_type;
	/*
	   dtype:
	   in CELL_DATA_*       block data type
	   in INDEX             index type
	 */
	jdb_data_t dtype;
	jdb_bent_t nful;
	jdb_tid_t tid;
	/*
	   the tid field is not needed in GLOBAL blocks like snapshots,
	   thus, it means:
			total bytes used in block: snapshot
	*/	
	/*
	   jdb_bid_t bid;
	   don't need this, there is a formula to find the bid if there is
	   a map entry for each block, that is:
	   assuming if map_order = 0 then map_bid = 0;
	   map_bid = map_order*(map_entry_capacity + 1);
	   bid = map_bid + blk_entry_inside_the_map;
	 */
}__attribute__((packed));

struct jdb_map {
	void* blockbuf;
	void* change_bmap;
	jdb_bid_t bid;
	uint32_t write;
	struct jdb_map_blk_hdr* hdr;
	struct jdb_map_blk_entry *entry;	
	struct jdb_map *next;
};

struct jdb_map_list {
	jdb_bid_t cnt;
	struct jdb_map *first;
	struct jdb_map *last;
};

struct jdb_table;

struct jdb_typedef_blk_hdr {
	uchar type;
	uchar flags;

	uint32_t crc32;
}__attribute__((packed));

struct jdb_typedef_blk_entry {
	uchar flags;
	uchar type_id;		//jdb_data_t
	uchar base;		//jdb_data_t too!
	uint32_t len;		//chunksize if var flag set
}__attribute__((packed));

struct jdb_typedef_blk {
	void* blockbuf;
	uint32_t write;
	jdb_bid_t bid;
	struct jdb_table *table;
	struct jdb_typedef_blk_hdr* hdr;
	struct jdb_typedef_blk_entry *entry;
	struct jdb_typedef_blk *next;
};

struct jdb_typedef_list {
	jdb_bid_t cnt;
	struct jdb_typedef_blk *first;
	struct jdb_typedef_blk *last;
};

struct jdb_celldef_blk_hdr {
	uchar type;
	uchar flags;

	uint32_t crc32;
}__attribute__((packed));

struct jdb_celldef_blk_entry {
	struct jdb_celldef_blk* parent;		//pointer to the celldef_blk's write, so i can set it directly on change
	uint32_t row;
	uint32_t col;
	uint32_t data_crc32;
	uchar data_type;
	uint32_t datalen;	//real len, will be converted to the 16byte-chunk'ed len on pack/unpack
	/*
	   data chain entry pointer
	 */
	jdb_bid_t bid_entry;
	jdb_bent_t bent;
	uchar last_access_d[3];
	uchar last_access_t[3];
}__attribute__((packed));

struct jdb_celldef_blk {
	void* blockbuf;
	uint32_t write;
	jdb_bid_t bid;
	struct jdb_table *table;
	struct jdb_celldef_blk_hdr* hdr;
	struct jdb_celldef_blk_entry *entry;
	struct jdb_celldef_blk *next;
};

struct jdb_celldef_list {
	jdb_bid_t cnt;
	struct jdb_celldef_blk *first;
	struct jdb_celldef_blk *last;
};

/*
	var. len. data chunks size is defined in table data type definition in len field
*/

#define JDB_VDATA_CHUNK_MAX	JDB_MAX_TYPE_SIZE
#define JDB_VDATA_CHUNK_MIN	32
#define JDB_VDATA_CHUNK_DEF	64

/*
	data can't be bigger than blocksize - data_blk_hdr size
*/

struct jdb_cell_data_blk_hdr {
	uchar type;
	uchar flags;
	uchar dtype;//already shown in map, put here for performance issues
	uint32_t crc32;
}__attribute__((packed));

struct jdb_cell_data_blk {
	void* blockbuf;
	uint32_t write;
	jdb_bid_t bid;
	uint16_t entsize;
	uint16_t base_len;
	jdb_bent_t bmapsize;
	jdb_bent_t nent;
	jdb_data_t data_type;
	jdb_data_t base_type;
	jdb_bent_t maxent;

	struct jdb_table *table;
	struct jdb_cell_data_blk_hdr* hdr;
	uchar* bitmap;
	uchar* datapool;
	struct jdb_cell_data_blk *next;
};

struct jdb_cell_data_list{
	jdb_bid_t cnt;
	struct jdb_cell_data_blk* first;
	struct jdb_cell_data_blk* last;
};

struct jdb_cell_data_ptr_blk_hdr {
	uchar type;
	uchar flags;
	//uchar data_type; not needed, shown in map entry       
	uint32_t crc32;
}__attribute__((packed));

struct jdb_cell_data_ptr_blk_entry {
	struct jdb_cell_data_ptr_blk* parent;
	jdb_bid_t bid;
	jdb_bent_t bent;/*begin, if DATA_EXT, this is the first part of the
			  the number of continues blocks allocated to
			  the cell, in this case it can't be INVAL*/
	jdb_bent_t nent;/*continues chunks inside the same block,
			  INVAL if the entire block allocated,
			  on the case of the DATA_EXT, this is the second part
			  of the number of continues blocks allocated to
			  the cell, in this case it can't be INVAL*/
	jdb_bid_t nextdptrbid;
	jdb_bent_t nextdptrbent;
	struct jdb_cell_data_ptr_blk_entry* next;
}__attribute__((packed));

struct jdb_cell_data_ptr_blk{
	void* blockbuf;
	uint32_t write;
	jdb_bid_t bid;
	jdb_bent_t nent;
	struct jdb_cell_data_ptr_blk_hdr* hdr;
	struct jdb_cell_data_ptr_blk_entry* entry;
	struct jdb_cell_data_ptr_blk* next;
	
};

struct jdb_cell_data_ptr_list{

	jdb_bid_t cnt;
	struct jdb_cell_data_ptr_blk* first;
	struct jdb_cell_data_ptr_blk* last;

};

#define JDB_CELL_DATA_NOTLOADED	0
#define JDB_CELL_DATA_UPTODATE	1
#define JDB_CELL_DATA_MODIFIED	2

struct jdb_cell {
	uint32_t write;
	int data_stat;
	struct jdb_table *table;
	struct jdb_celldef_blk_entry *celldef;
	struct jdb_cell_data_ptr_blk_entry *dptr;/*data pointers, they point to the
						chunks inside the lists of data blocks
						the pointer is array and allocated as
						buckets
						*/
	struct jdb_cell* next;
	uchar *data;
};

struct jdb_cell_list {
	jdb_bid_t cnt;
	struct jdb_cell *first;
	struct jdb_cell *last;
};

struct jdb_table_def_blk_hdr {
	uchar type;
	uchar flags;

	uint16_t indexes;
	jdb_tid_t tid;
	uint16_t namelen;	//jcs len
	uint32_t nrows;
	uint32_t ncols;
	uint64_t ncells;
	jdb_bid_t ncol_typedef;	//number of col_typedef entries
	uint32_t crc32;
}__attribute__((packed));

struct jdb_table_def_blk {
	void* blockbuf;
	uint32_t write;
	struct jdb_table *table;
	struct jdb_table_def_blk_hdr* hdr;
	wchar_t *name;
};

//#define JDB_COL_TYPEDEF_TYPE_COL_TYPE        0x01

struct jdb_col_typedef_blk_hdr {
	uchar type;
	uchar flags;

	uint32_t crc32;
}__attribute__((packed));

struct jdb_col_typedef_blk_entry {
	uchar type_id;
	uint32_t col;
}__attribute__((packed));

struct jdb_col_typedef {
	void* blockbuf;
	uint32_t write;
	jdb_bid_t bid;
	struct jdb_table *table;
	struct jdb_col_typedef_blk_hdr* hdr;
	struct jdb_col_typedef_blk_entry *entry;
	struct jdb_col_typedef *next;
}__attribute__((packed));

struct jdb_col_typedef_list {
	jdb_bid_t cnt;
	struct jdb_col_typedef *first;
	struct jdb_col_typedef *last;
};

struct jdb_fav_blk_hdr{
	uchar type;
	uchar flags;
	uint32_t crc32;
}__attribute__((packed));

struct jdb_fav_blk_entry{
	jdb_bid_t bid;
	uint32_t nhits;
}__attribute__((packed));

struct jdb_fav_blk{
	void* blockbuf;
	uint32_t write;
	//struct jdb_table* table;
	jdb_bid_t bid;
	struct jdb_fav_blk_hdr* hdr;
	struct jdb_fav_blk_entry* entry;
	struct jdb_fav_blk* next;
};

struct jdb_fav_blk_list{
	jdb_bid_t cnt;
	struct jdb_fav_blk* first;
	struct jdb_fav_blk* last;
};

/*				MACROs					*/

#define _jdb_assign_blk_element_offset(blk, hdr_size, entry_size, nentry)	\
	(blk)->hdr = __typeof((blk)->hdr)((blk)->blockbuf);			\
	(blk)->entry = __typeof((blk)->entry)((blk)->blockbuf + (hdr_size));	\
	(blk)->change_bmap = (blk)->blockbuf + (hdr_size) + ((nentry)*(entry_size));

#define _jdb_create_blk(h, table, blk, blk_type, hdr_size, entry_size, nentry, list, ret)\
	(blk)->blockbuf = _jdb_get_blockbuf(h, blk_type,0, JDB_CHANGE_BMAP_SIZE(nentry));\
	if(!(blk)->blockbuf){\
		*ret = -JE_MALOC;\
	} else {\
		_jdb_assign_blk_element_offset(blk, hdr_size, entry_size, nentry);\
		(blk)->bid =_jdb_get_empty_map_entry(h, blk_type, 0, (table)->main.hdr.tid, 0);\
		if((blk)->bid == JDB_ID_INVAL){\
			_jdb_release_blockbuf((blk)->blockbuf);\
			*ret = -JE_LIMIT;\
		} else {\
			_jdb_lock_handle(h, JDB_LK_HDR, JDB_LK_WR);\
			(h)->hdr.nblocks++;\
			_jdb_set_handle_flag(h, JDB_HMODIF, 0);\
			_jdb_unlock_handle(h, JDB_LK_HDR, JDB_LK_WR);\
			memset((void*)(blk)->hdr, 0, hdr_size);\
			memset((void*)(blk)->entry, 0xff, (entry_size)*(nentry));\
			memset((blk)->change_bmap, 0xff, JDB_CHANGE_BMAP_SIZE(nentry));\
			(blk)->next = NULL;\
			(blk)->write = 0UL;\
			(blk)->hdr->type = blk_type;\
			_JDB_SET_WR(h, blk, (blk)->bid, table, 1);\
			if (!(list)->first) {\
				(list)->first = blk;\
				(list)->last = blk;\
				(list)->cnt = 1UL;\
			} else {\
				(list)->last->next = blk;\
				(list)->last = blk;\
				(list)->cnt++;\
			}\
			*ret = 0;\
		}\
	}

#define _jdb_create_blk_parented(h, table, blk, blk_type, hdr_size, entry_size, nentry, list, ret)\
	(blk)->blockbuf = _jdb_get_blockbuf(h, blk_type,0, JDB_CHANGE_BMAP_SIZE(nentry));\
	if(!(blk)->blockbuf){\
		*ret = -JE_MALOC;\
	} else {\
		_jdb_assign_blk_element_offset(blk, hdr_size, entry_size, nentry);\
		(blk)->bid =_jdb_get_empty_map_entry(h, blk_type, 0, (table)->main.hdr.tid, 0);\
		if((blk)->bid == JDB_ID_INVAL){\
			_jdb_release_blockbuf((blk)->blockbuf);\
			*ret = -JE_LIMIT;\
		} else {\
			_jdb_lock_handle(h, JDB_LK_HDR, JDB_LK_WR);\
			(h)->hdr.nblocks++;\
			_jdb_set_handle_flag(h, JDB_HMODIF, 0);\
			_jdb_unlock_handle(h, JDB_LK_HDR, JDB_LK_WR);\
			memset((void*)(blk)->hdr, 0, hdr_size);\
			memset((void*)(blk)->entry, 0xff, (entry_size)*(nentry));\
			memset((blk)->change_bmap, 0xff, JDB_CHANGE_BMAP_SIZE(nentry));\
			(blk)->next = NULL;\
			(blk)->write = 0UL;\
			(blk)->hdr->type = blk_type;\
			_JDB_SET_WR(h, blk, (blk)->bid, table, 1);\
			for (int _j = 0; _j < nentry; _j++) {\
				(blk)->entry[_j].parent = blk;\
			}\
			if (!(list)->first) {\
				(list)->first = blk;\
				(list)->last = blk;\
				(list)->cnt = 1UL;\
			} else {\
				(list)->last->next = blk;\
				(list)->last = blk;\
				(list)->cnt++;\
			}\
			*ret = 0;\
		}\
	}
	
#define _jdb_free_blk_list(h, list)\
	__typeof((list)->first) del;\
	while ((list)->first) {\
		del = (list)->first;\
		list.first = (list)->first->next;\
		_jdb_release_blockbuf(h, del->blockbuf);\
		free(del);\
	}\
	(list)->first = NULL;
	
#define _jdb_rm_blk(h, list, bid){\
	__typeof((list)->first) del;\
	__typeof((list)->first) prev;\
	del = (list)->first;\
	while(del){\
		if(del->bid == bid){\
			if(del->bid == (list)->first->bid){\
				(list)->first = (list)->first->next;\
				break;\
			} else if(del->bid == (list)->last->bid){\
				prev->next = NULL;\
				(list)->last = prev;\
				break;\
			} else {\
				prev->next = del->next;\
				break;\
			}\
		}\
		prev = del;\
		del = del->next;\
	}\
	if(del){\
		_jdb_release_blockbuf(h, del->blockbuf);\
		free(del);\
	}
	
/*
				CALCULATORS
	these are macros to calculate needed allocation space for header
	and entries of a block
*/
#define _jdb_calc_n_cbs(cb_size, entry_size)\
	( (entry_size) % (cb_size) )?( (entry_size ) / (cb_size) + 1 ):( (entry_size ) / (cb_size) ) 

#define _jdb_calc_blk_nentry(blk_size, hdr_size, entry_size)		\
	( ( (blk_size) - (hdr_size) ) / (entry_size) )
	
#define _jdb_calc_blk_entry_offset(hdr_size, entry_size, i, start_off)	\
	( (start_off) + (hdr_size) + ( (i) * (entry_size) ) )
	
#define _jdb_calc_blk_entry_i(entry, first, entry_size)\
	( ( (entry) - (first) ) / (entry_size) )

int _jdb_write_block(struct jdb_handle *h, uchar * block, jdb_bid_t bid,
		     uchar flags);
int _jdb_read_block(struct jdb_handle *h, uchar * block, jdb_bid_t bid);
int _jdb_write_map_blk(struct jdb_handle *h, struct jdb_map *map);
int _jdb_read_map_blk(struct jdb_handle *h, struct jdb_map *map);
int _jdb_seek_write(struct jdb_handle *h, uchar * buf, size_t len,
		    off_t offset);
int _jdb_seek_read(struct jdb_handle *h, uchar * buf, size_t len, off_t offset);
int _jdb_write_typedef_blk(struct jdb_handle *h, struct jdb_typedef_blk *blk);
int _jdb_read_typedef_blk(struct jdb_handle *h, struct jdb_typedef_blk *blk);
int _jdb_write_col_typedef_blk(struct jdb_handle *h, struct jdb_col_typedef *blk);
int _jdb_read_col_typedef_blk(struct jdb_handle *h, struct jdb_col_typedef *blk);
int _jdb_write_celldef_blk(struct jdb_handle *h, struct jdb_celldef_blk *blk);
int _jdb_read_celldef_blk(struct jdb_handle *h, struct jdb_celldef_blk *blk);
int _jdb_read_table_def_blk(struct jdb_handle *h, struct jdb_table *table);
int _jdb_write_table_def_blk(struct jdb_handle *h, struct jdb_table *table);
int _jdb_write_data_blk(struct jdb_handle *h, struct jdb_cell_data_blk *blk);
int _jdb_read_data_blk(	struct jdb_handle *h, struct jdb_table* table,
			struct jdb_cell_data_blk *blk, jdb_data_t type_id);
int _jdb_write_fav_blk(struct jdb_handle *h, struct jdb_fav_blk *blk);
int _jdb_read_fav_blk(struct jdb_handle *h, struct jdb_fav_blk *blk);
int _jdb_write_data_ptr_blk(struct jdb_handle *h, struct jdb_cell_data_ptr_blk *blk);
int _jdb_read_data_ptr_blk(struct jdb_handle *h, struct jdb_cell_data_ptr_blk *blk);

int _jdb_calc_data(jdb_bsize_t blocksize, uint16_t entsize,
		    jdb_bent_t * bent, jdb_bent_t * bmapsize);		


#else

#endif
