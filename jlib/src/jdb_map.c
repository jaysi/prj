#include "jdb.h"

jdb_bent_t _jdb_max_nful(struct jdb_handle * h, jdb_blk_t btype)
{
	switch (btype) {
	case JDB_BTYPE_EMPTY:
		return 0;
	case JDB_BTYPE_MAP:
		return h->hdr.map_bent;
	case JDB_BTYPE_TYPE_DEF:
		return h->hdr.typedef_bent;
	case JDB_BTYPE_TABLE_DEF:
		return JDB_BENT_INVAL;
	case JDB_BTYPE_COL_TYPEDEF:
		return h->hdr.col_typedef_bent;
	case JDB_BTYPE_CELLDEF:
		return h->hdr.celldef_bent;
	case JDB_BTYPE_CELL_DATA_PTR:
		return h->hdr.dptr_bent;
	case JDB_BTYPE_INDEX:
		return h->hdr.index1_bent;
	default:
		_wdeb(L"ERROR: bad block type: 0x%x", btype);
		return 0;
	}
}

void _jdb_free_map_list(struct jdb_handle *h)
{
	_jdb_free_blk_list(h, &h->map_list);
}

int _jdb_create_map(struct jdb_handle *h)
{
	struct jdb_map *map;
	jdb_bid_t bid;
	
	/*
		the MACROs are made for working with tables, i'm not going to
		use them here for now.
	*/

	_jdb_lock_handle(h, JDB_LK_HDR, JDB_LK_WR);

	_wdeb_map(L"max_blocks = %u", h->hdr.max_blocks);

	//_jdb_enter_lock(h, JDB_LK_MAP_RD | JDB_LK_MAP_WR);

	if (h->map_list.first == NULL) {
		bid = 0UL;
	} else if ((bid = h->map_list.last->bid + h->hdr.map_bent + 1) >=
		   h->hdr.max_blocks) {
		_jdb_unlock_handle(h, JDB_LK_HDR, JDB_LK_WR);
		//_jdb_exit_lock(h, JDB_LK_MAP_RD | JDB_LK_MAP_WR);
		return -JE_LIMIT;
	}

	h->hdr.nblocks++;
	h->hdr.nmaps++;
	_jdb_set_handle_flag(h, JDB_HMODIF, 0);
	_jdb_unlock_handle(h, JDB_LK_HDR, JDB_LK_WR);

	_wdeb_map(L"map bid %u", bid);

	map = (struct jdb_map *)malloc(sizeof(struct jdb_map));
	if (!map) {

		_jdb_lock_handle(h, JDB_LK_HDR, JDB_LK_WR);
		h->hdr.nblocks--;
		h->hdr.nmaps--;
		_jdb_unset_handle_flag(h, JDB_HMODIF, 0);
		_jdb_unlock_handle(h);
		return -JE_MALOC;
	}
	
	map->blockbuf = _jdb_get_blockbuf(h, JDB_BTYPE_MAP, 0,
					JDB_CHANGE_BMAP_SIZE(h->hdr.map_bent));
	if (!map->blockbuf) {

		//undo code
		_jdb_lock_handle(h, JDB_LK_HDR, JDB_LK_WR);
		h->hdr.nblocks--;
		h->hdr.nmaps--;
		_jdb_unset_handle_flag(h, JDB_HMODIF, 0);
		_jdb_unlock_handle(h, JDB_LK_HDR, JDB_LK_WR);

		free(map);
		return -JE_MALOC;
	}
	
	map->hdr = (struct jdb_map_blk_hdr*)map->blockbuf;
	map->entry = (struct jdb_map_blk_entry*)(map->blockbuf +
						sizeof(struct jdb_map_blk_hdr));
	map->change_bmap = map->entry + 
			sizeof(struct jdb_map_blk_entry) * h->hdr.map_bent;
	
	map->next = NULL;
	memset((void *)map->hdr, '\0', sizeof(struct jdb_map_blk_hdr));
	memset((void *)map->entry, '\0',
	       sizeof(struct jdb_map_blk_entry) * h->hdr.map_bent);
	memset(map->change_bmap, 0xff, JDB_CHANGE_BMAP_SIZE(h->hdr.map_bent));

	map->hdr->type = JDB_BTYPE_MAP;

	map->bid = bid;
	map->write = 1;
	if (!h->map_list.first) {
		h->map_list.first = map;
		h->map_list.last = map;
		h->map_list.cnt = 1UL;
	} else {
		h->map_list.last->next = map;
		h->map_list.last = h->map_list.last->next;
		h->map_list.cnt++;
	}
	return 0;
}
/*
int _jdb_set_map_entry(struct jdb_handle *h,
		       struct jdb_map *map,
		       jdb_bent_t map_bent,
		       jdb_blk_t btype,
		       jdb_data_t dtype, jdb_tid_t tid, jdb_bent_t nful)
{

	if (map->entry[map_bent].blk_type == JDB_BTYPE_EMPTY &&
	    btype != JDB_BTYPE_EMPTY) {
		map->hdr->nset++;
	} else if (map->entry[map_bent].blk_type != JDB_BTYPE_EMPTY &&
		   btype == JDB_BTYPE_EMPTY) {
		map->hdr->nset--;
	}

	map->entry[map_bent].blk_type = btype;
	map->entry[map_bent].dtype = dtype;
	map->entry[map_bent].tid = tid;
	map->entry[map_bent].nful = nful;
	map->write++;

	return 0;
}
*/
static inline struct jdb_map* _jdb_get_map_ptr_by_bid(struct jdb_handle* h,
							jdb_bid_t bid){
	jdb_bid_t i;
	struct jdb_map* map;
	map = h->map_list.first;
	assert(map);
	_wdeb_find(L"_JDB_MAP_ORDER(bid(%u), h->hdr.map_bent(%u)) = %u",
		bid,h->hdr.map_bent, _JDB_MAP_ORDER(bid, h->hdr.map_bent));
		
	for(i = 0; i < _JDB_MAP_ORDER(bid, h->hdr.map_bent); i++){
		map = map->next;
		if(!map) break;
	}

	return map;
}

static inline struct jdb_map_blk_entry* _jdb_get_map_entry_ptr(
					struct jdb_handle* h, jdb_bid_t bid){
	struct jdb_map* map;
	
	if(!(map = _jdb_get_map_ptr_by_bid(h, bid))) return NULL;
	assert(_JDB_MAP_BENT(bid, h->hdr.map_bent) <= h->hdr.map_bent);
	return &map->entry[_JDB_MAP_BENT(bid, h->hdr.map_bent)];
}

jdb_dtype_t _jdb_get_block_dtype(struct jdb_handle* h, jdb_bid_t bid){
	struct jdb_map_blk_entry* m_ent;
	jdb_dtype_t dtype;
	
	m_ent = _jdb_get_map_entry_ptr(h, bid);
	if(!m_ent) dtype = JDB_TYPE_EMPTY;
	else dtype = m_ent->dtype;

	return dtype;
}

int _jdb_set_map_nful_by_bid(struct jdb_handle *h, jdb_bid_t bid,
			     jdb_bent_t nful)
{	
	jdb_bent_t map_bent;
	struct jdb_map *map;
	
	map = _jdb_get_map_ptr_by_bid(h, bid);
	if(!map){
		return -JE_NOTFOUND;
	}
	
	map_bent = bid - map->bid - 1;
	_wdeb_nful
	    (L"found bid @ map %u[%u]: %u, blk_type: %x, dtype: %x, nful: %u",
	     map->bid,
	     map_bent,
	     map->entry[map_bent].blk_type,
	     map->entry[map_bent].dtype,
	     map->entry[map_bent].nful);
	     	     
	map->entry[map_bent].nful = nful;
	
	_jdb_track_change(map->change_bmap, map_bent);
	
	map->write++;
	
	return 0;
}

int _jdb_inc_map_nful_by_bid(struct jdb_handle *h, jdb_bid_t bid, jdb_bent_t n)
{
	jdb_bent_t map_bent;
	struct jdb_map *map;
	
	map = _jdb_get_map_ptr_by_bid(h, bid);
	if(!map){
		_wdeb_inc(L"map ptr of bid %u not found", bid);
		return -JE_NOTFOUND;		
	}
	
	map_bent = bid - map->bid - 1;
	_wdeb_nful
	    (L"found bid @ map %u[%u]: %u, blk_type: %x, dtype: %x, nful: %u, adding by %u",
	     map->bid,
	     map_bent,
	     map->entry[map_bent].blk_type,
	     map->entry[map_bent].dtype,
	     map->entry[map_bent].nful, n);

	map->entry[map_bent].nful+=n;
	
	_jdb_track_change(map->change_bmap, map_bent);

	map->write++;
	return 0;
}

int _jdb_dec_map_nful_by_bid(struct jdb_handle *h, jdb_bid_t bid, jdb_bent_t n)
{
	jdb_bent_t map_bent;
	struct jdb_map *map;
	map = _jdb_get_map_ptr_by_bid(h, bid);
	if(!map){
		return -JE_NOTFOUND;
	}
	map_bent = bid - map->bid - 1;
	_wdeb_nful
	    (L"found bid @ map %u[%u]: %u, blk_type: %x, dtype: %x, nful: %u",
	     map->bid,
	     map_bent,
	     map->entry[map_bent].blk_type,
	     map->entry[map_bent].dtype,
	     map->entry[map_bent].nful);
	     
	map->entry[map_bent].nful-=n;

	_jdb_track_change(map->change_bmap, map_bent);
	
	map->write++;
	return 0;
}

int _jdb_get_map_nful_by_bid(struct jdb_handle *h, jdb_bid_t bid, jdb_bent_t* nful)
{
	jdb_bent_t map_bent;
	struct jdb_map *map;
	map = _jdb_get_map_ptr_by_bid(h, bid);
	if(!map){
		return -JE_NOTFOUND;
	}
	map_bent = bid - map->bid - 1;
	_wdeb_nful
	    (L"found bid @ map %u[%u]: %u, blk_type: %x, dtype: %x, nful: %u",
	     map->bid,
	     map_bent,
	     map->entry[map_bent].blk_type,
	     map->entry[map_bent].dtype,
	     map->entry[map_bent].nful);	     
	*nful = map->entry[map_bent].nful;
	return 0;
}

jdb_bid_t _jdb_get_empty_map_entry(struct jdb_handle * h, jdb_blk_t btype,
				   jdb_data_t dtype, jdb_tid_t tid,
				   jdb_bent_t nful)
{
	struct jdb_map *map;
	jdb_bent_t bent;
	jdb_bid_t bid;
	
	for (map = h->map_list.first; map; map = map->next) {
		if (map->hdr->nset >= h->hdr.map_bent)
			continue;
after_new_created:
		for (bent = 0; bent < h->hdr.map_bent; bent++) {
			if (map->entry[bent].blk_type == JDB_ID_EMPTY) {
				map->entry[bent].blk_type = btype;
				map->entry[bent].dtype = dtype;
				map->entry[bent].tid = tid;
				map->entry[bent].nful = nful;
				
				_jdb_track_change(map->change_bmap, bent);
				
				map->write++;
				map->hdr->nset++;
								
				bid = _jdb_calc_bid(map->bid, bent);
				return bid;
			}
		}
	}
	
	if (_jdb_create_map(h) < 0) {
		_wdeb_map(L"_jdb_create_map() fails, tid: %u", tid);
		return JDB_ID_INVAL;
	}

	map = h->map_list.last;

	goto after_new_created;

	return bid;
}

/*

int _jdb_get_empty_map_entries(struct jdb_handle *h, jdb_blk_t * btype,
			       jdb_data_t * dtype, jdb_tid_t * tid,
			       jdb_bent_t * nful, jdb_bid_t * bid, jdb_bid_t n)
{

	struct jdb_map *map;
	jdb_bent_t bent;
	jdb_bid_t i;
	int ret;

	i = 0UL;

	for (map = h->map_list.first; map; map = map->next) {
		if (map->hdr->nset >= h->hdr.map_bent)
			continue;
		for (bent = 0; bent < h->hdr.map_bent; bent++) {
			if (map->entry[bent].blk_type == JDB_ID_EMPTY) {
				map->entry[bent].blk_type = btype[i];
				map->entry[bent].dtype = dtype[i];
				map->entry[bent].tid = tid[i];
				map->entry[bent].nful = nful[i];
				bid[i++] = _jdb_calc_bid(map->bid, bent);
				map->write++;
				map->hdr->nset++;
				if (i == n)
					return 0;
			}
		}
	}

	while (1) {

		if ((ret = _jdb_create_map(h)) < 0) {
			_wdeb_map(L"_jdb_create_map() fails, bid: %u", bid[i]);
			return ret;
		}

		map = h->map_list.last;

		for (bent = 0; bent < h->hdr.map_bent; bent++) {
			map->entry[bent].blk_type = btype[i];
			map->entry[bent].dtype = dtype[i];
			map->entry[bent].tid = tid[i];
			map->entry[bent].nful = nful[i];
			bid[i++] = _jdb_calc_bid(map->bid, bent);
			map->hdr->nset++;
			if (i == n)
				return 0;
		}
	}

	return -JE_UNK;
}

*/

int _jdb_rm_map_bid_entries(struct jdb_handle *h, jdb_bid_t * bid, jdb_bid_t n)
{
	struct jdb_map *map;
	jdb_bid_t i, removed = 0UL;
	jdb_bent_t map_bent;
	
	for (i = 0; i < n; i++) {
	
		map = _jdb_get_map_ptr_by_bid(h, bid[i]);
		if(!map) continue;
		
		map_bent = bid[i] - map->bid - 1;
		map->entry[map_bent].blk_type = JDB_BTYPE_EMPTY;
		bid[i] = JDB_ID_INVAL;
		removed++;
		map->hdr->nset--;
		
		_jdb_track_change(map->change_bmap, map_bent);
		
		map->write++;
	}
	
	_jdb_lock_handle(h, JDB_LK_HDR, JDB_LK_WR);
	h->hdr.nblocks -= removed;
	_jdb_set_handle_flag(h, JDB_HMODIF, 0);
	_jdb_unlock_handle(h, JDB_LK_HDR, JDB_LK_WR);

	if (removed == n)
		return 0;
	return -JE_NOTFOUND;
}

int _jdb_rm_map_bid_entry(struct jdb_handle *h, jdb_bid_t bid)
{
	struct jdb_map *map;	
	jdb_bent_t map_bent;
	
	map = _jdb_get_map_ptr_by_bid(h, bid);
	if(!map){
		return -JE_NOTFOUND;
	}
	map_bent = bid - map->bid - 1;
	map->entry[map_bent].blk_type = JDB_BTYPE_EMPTY;
	map->hdr->nset--;
	map->write++;
	
	_jdb_track_change(map->change_bmap, bent);
	
	_jdb_lock_handle(h, JDB_LK_HDR, JDB_LK_WR);
	h->hdr.nblocks--;
	_jdb_set_handle_flag(h, JDB_HMODIF, 0);
	_jdb_unlock_handle(h, JDB_LK_HDR, JDB_LK_WR);
	
	return 0;	
}

jdb_bid_t _jdb_find_first_map_match(struct jdb_handle * h, jdb_blk_t btype,
				    jdb_data_t dtype, jdb_tid_t tid,
				    jdb_bent_t nful, uchar cmp_flags)
{
	struct jdb_map *map;
	jdb_bent_t i;
	int match_needed = 0, matches;

	if (cmp_flags & JDB_MAP_CMP_BTYPE) {
		match_needed++;
	}
	if (cmp_flags & JDB_MAP_CMP_DTYPE) {
		match_needed++;
	}
	if (cmp_flags & JDB_MAP_CMP_TID) {
		match_needed++;
	}

	if (cmp_flags & JDB_MAP_CMP_NFUL) {
		match_needed++;
	}
	
	_wdeb_find(L"searching for B:0x%02x, D:0x%02x, T:%u, N:%u, match needed = %u",
			btype, dtype, tid, nful, match_needed);

	for (map = h->map_list.first; map; map = map->next) {
		for (i = 0; i < h->hdr.map_bent; i++) {
			matches = 0;
			if (cmp_flags & JDB_MAP_CMP_BTYPE) {
				if (map->entry[i].blk_type != btype)
					continue;
				matches++;
				_wdeb_find(L"set matches to %u [BTYPE=0x%02x] @ #%u:%u",
					matches, btype, map->bid, i);
			}
			if (cmp_flags & JDB_MAP_CMP_DTYPE) {
				if (map->entry[i].dtype != dtype)
					continue;
				matches++;
				_wdeb_find(L"set matches to %u [DTYPE=0x%02x] @ #%u:%u", 
					matches, dtype, map->bid, i);
			}
			if (cmp_flags & JDB_MAP_CMP_TID) {
				if (map->entry[i].tid != tid)
					continue;
				matches++;
				_wdeb_find(L"set matches to %u [TID=%u] @ #%u:%u",
					matches, tid, map->bid, i);
			}

			if (cmp_flags & JDB_MAP_CMP_NFUL) {
				if (map->entry[i].nful >= nful)
					continue;
				matches++;
				_wdeb_find(L"set matches to %u [NFUL=<%u] @ #%u:%u",
					matches, nful, map->bid, i);
			}

			if (matches == match_needed){
				bid = _jdb_calc_bid(map->bid, i);
				return bid;
			}
		}
	}
	_wdeb_find(L"returning INVAL");
	return JDB_ID_INVAL;
}

int _jdb_list_map_match(struct jdb_handle *h, jdb_blk_t btype, jdb_data_t dtype,
			jdb_tid_t tid, jdb_bent_t nful, uchar cmp_flags,
			jdb_bid_t ** bid, jdb_bid_t * n)
{
	struct jdb_map *map;
	jdb_bent_t i;
	int match_needed = 0, matches;
	int buck_cnt = 0;	

	*n = 0UL;
	*bid = NULL;

	if (cmp_flags & JDB_MAP_CMP_BTYPE) {
		match_needed++;
	}
	if (cmp_flags & JDB_MAP_CMP_DTYPE) {
		match_needed++;
	}
	if (cmp_flags & JDB_MAP_CMP_TID) {
		match_needed++;
	}
	if (cmp_flags & JDB_MAP_CMP_NFUL) {
		match_needed++;
	}
	for (map = h->map_list.first; map; map = map->next) {
		for (i = 0; i < h->hdr.map_bent; i++) {
			matches = 0;
			if (cmp_flags & JDB_MAP_CMP_BTYPE) {
				if (map->entry[i].blk_type != btype)
					continue;
				matches++;
			}
			if (cmp_flags & JDB_MAP_CMP_DTYPE) {
				if (map->entry[i].dtype != dtype)
					continue;
				matches++;
			}
			if (cmp_flags & JDB_MAP_CMP_TID) {
				if (map->entry[i].tid != tid)
					continue;
				matches++;
			}
			if (cmp_flags & JDB_MAP_CMP_NFUL) {
				if (map->entry[i].nful >= nful)
					continue;
				matches++;
			}
			if (matches == match_needed) {
				if (!(*bid)) {
					*bid = (jdb_bid_t *)
					    malloc(sizeof(jdb_bid_t) *
						   h->hdr.map_list_buck);
					if (!(*bid)){
						return -JE_MALOC;
					}
					buck_cnt = 0;
				} else {
					if (buck_cnt == h->hdr.map_list_buck) {
						*bid = realloc(*bid,
							sizeof(jdb_bid_t)*(h->hdr.map_list_buck*
						    		(((*n)+h->hdr.map_list_buck)/h->hdr.map_list_buck)));
						if (!(*bid)){
							return -JE_MALOC;
						}
						
						
						buck_cnt = 0;
					}
				}
				(*bid)[(*n)++] = _jdb_calc_bid(map->bid, i);
				buck_cnt++;
			}
		}
	}
	_wdeb_list_map(L"buck cnt is %i, *n = %u", buck_cnt, *n);
	return 0;
}

int _jdb_load_map(struct jdb_handle *h)
{

	int ret;
	jdb_bid_t bid = 0UL;
	jdb_bid_t nmaps = h->hdr.nmaps;
	struct jdb_map *map;
	while (nmaps) {
		map = (struct jdb_map *)malloc(sizeof(struct jdb_map));
		
		map->blockbuf = _jdb_get_blockbuf(h, JDB_BTYPE_MAP, 0,
					JDB_CHANGE_BMAP_SIZE(h->hdr.map_bent));
		if(!map->blockbuf){
			return -JE_MALOC;
		}
		
		map->hdr = (struct jdb_map_blk_hdr*)map->blockbuf;
		map->entry = (struct jdb_map_blk_entry*)(map->blockbuf +
				sizeof(struct jdb_map_blk_hdr));
		map->change_bmap = map->entry +
			(sizeof(struct jdb_map_blk_entry)*h->hdr.map_bent);
		
		map->bid = bid;
		map->next = NULL;
		map->write = 0;

		if ((ret = _jdb_read_map_blk(h, map)) < 0) {
			_jdb_release_blockbuf(h, map->blockbuf);
			free(map);
			_jdb_cleanup_handle(h);
			return ret;
		}
		
		memset(	map->change_bmap , '\0',
			JDB_CHANGE_BMAP_SIZE(h->hdr.map_bent));

		nmaps--;
		bid += h->hdr.map_bent + 1;

		if (!h->map_list.first) {
			h->map_list.first = map;
			h->map_list.last = map;
			h->map_list.cnt = 1UL;
		} else {
			h->map_list.last->next = map;
			h->map_list.last = map;
			h->map_list.cnt++;
		}
	}

	return 0;
}
/*
int _jdb_write_map(struct jdb_handle *h)
{
	int ret = 0, fret;
	struct jdb_map *map;
	
	//_jdb_enter_lock(h, JDB_LK_MAP_WR | JDB_LK_MAP_RD);
	
	for (map = h->map_list.first; map; map = map->next) {
		if (map->write) {
			fret = _jdb_write_map_blk(h, map);
			if (fret < 0) {
				ret = fret;
			} else {
				map->write = 0;
			}
		}
	}
	
	//_jdb_exit_lock(h, JDB_LK_MAP_WR | JDB_LK_MAP_RD);

	return ret;
}
*/
int _jdb_map_chg_proc(struct jdb_handle* h, jdb_bid_t bid, struct jdb_table* table, jdb_bid_t map_bid){
	int cntr;
	int exists = 0;
	
	if(h->hdr.flags & JDB_O_WR_THREAD){
		for(cntr = 0; cntr < table->map_chg_ptr; cntr++){
			if(table->map_chg_list[cntr] == map_bid){
				_wdeb_tm(L"found m_c_l[ %u ] = %u", cntr, map_bid);
				exists = 1;
				break;
			}						
		}
		if(cntr == table->map_chg_list_size){
			_wdeb_tm(L"cntr[ %u ] = m_c_l_size %u", cntr,
					table->map_chg_list_size);
			table->map_chg_list_size += JDB_MAP_CHG_LIST_BUCK;
			table->map_chg_list = realloc(table->map_chg_list,
						table->map_chg_list_size);
			if(!table->map_chg_list){
				return -JE_MALOC;
			}
			table->map_chg_ptr = table->map_chg_list_size -
						JDB_MAP_CHG_LIST_BUCK + 1;
			table->map_chg_list[table->map_chg_ptr-1] = map_bid;
			_wdeb_tm(L"m_c_l_size now %u, set pos %u to %u",
				table->map_chg_list_size,
				table->map_chg_list_size -
					JDB_MAP_CHG_LIST_BUCK, map_bid);
		} else {
			if(!exists){
				_wdeb_tm(L"cntr[ %u ] set to %u",cntr,map_bid);
				table->map_chg_list[cntr] = map_bid;
				table->map_chg_ptr++;
			}
		}	
	}
	return 0;
}

int _jdb_rm_map_entries_by_tid(struct jdb_handle *h, jdb_tid_t tid)
{
	struct jdb_map *map;
	jdb_bent_t i;
	jdb_bid_t removed = 0;

	for (map = h->map_list.first; map; map = map->next) {
		for (i = 0; i < h->hdr.map_bent; i++){
			if (map->entry[i].tid == tid){
				map->entry[i].blk_type = JDB_BTYPE_EMPTY;
				map->hdr->nset--;

				_jdb_track_change(map->change_bmap, i);		

				map->write++;
				removed++;
			}
		}
	}

	_jdb_lock_handle(h, JDB_LK_HDR, JDB_LK_WR);

	_wdeb_rm(L"removed = %u", removed);

	h->hdr.nblocks-=removed;
	//h->hdr.ndata_ptrs++;
	_jdb_set_handle_flag(h, JDB_HMODIF, 0);
	_jdb_unlock_handle(h, JDB_LK_HDR, JDB_LK_WR);
	
	return 0;
}

