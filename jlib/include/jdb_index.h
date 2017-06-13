#ifndef _JDB_INDEX_H
#define _JDB_INDEX_H

#include "jdb_types.h"

/*				indexes					*/

//index types, set as map_entry's dtype field, length same as jdb_data_t
#define JDB_ITYPE0	0x00
#define JDB_ITYPE1	0x01
#define JDB_ITYPE2	0x02

/*
	index type 0 (GLOBAL-UNIQUE), hash -> id
	Usage:
		TableNameHash -> TableID	TID_INX
		Allows table renaming.
*/

struct jdb_index_blk_hdr{

	uchar type;
	uchar flags;

	uint32_t crc32;		//crc of the block including header
}__attribute__((packed));

struct jdb_index0_blk_entry{
	uint32_t hash;
	uint32_t tid;//JDB_ID_INVAL for empty entries
}__attribute__((packed));

struct jdb_index0_blk{
	void* blockbuf;
	uint32_t write;
	jdb_bid_t bid;
	struct jdb_index_blk_hdr* hdr;
	struct jdb_index0_blk_entry* entry;
	struct jdb_index0_blk* next;
};

struct jdb_index0_blk_list{
	uint32_t cnt;
	struct jdb_index0_blk* first;
	struct jdb_index0_blk* last;

};

/*
	index type 1 (LOCAL-UNIQUE), hash -> row, col
	Usage:
		FullDataHash -> Row, Column
		
		Allows fast data search
		limitations are support for exact data match only
*/

struct jdb_index1_blk_entry {
	uint64_t hash64;	
	uint32_t row;
	uint32_t col;
}__attribute__((packed));

struct jdb_index1 {
	void* blockbuf;
	uint32_t write;

	struct jdb_table *table;
	struct jdb_index_blk_hdr* hdr;
	struct jdb_index1_blk_entry *entry;
	struct jdb_index1 *next;
};

struct jdb_index1_list {
	jdb_bid_t cnt;
	struct jdb_index1 *first;
	struct jdb_index1 *last;
};

/*
	index type 2 (LOCAL-NON_UNIQUE), hash -> row, col
	Usage:
		DataHash -> Row, Column
		
		Allows fast data search even for parts of data in a cell
		May return multiple results, Same record may not exist in
		a table, thus insertion is SLOW due to needed searches
*/

struct jdb_index2_blk_entry {
	uint32_t hash32;	
	uint32_t row;
	uint32_t col;
}__attribute__((packed));

struct jdb_index2 {
	void* blockbuf;
	uint32_t write;

	struct jdb_table *table;
	struct jdb_index_blk_hdr* hdr;
	struct jdb_index2_blk_entry *entry;
	struct jdb_index2 *next;
};

struct jdb_index2_list {
	jdb_bid_t cnt;
	struct jdb_index2 *first;
	struct jdb_index2 *last;
};

#else

#endif
