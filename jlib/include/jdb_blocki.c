#include "jdb.h"

static inline jdb_bent_t _jdb_get_bent_by_btype(struct jdb_handle* h, jdb_btype_t btype, jdb_btype_t itype){
	
	switch(btype){
		
	case JDB_BTYPE_EMPTY:
		return 0;
	case JDB_BTYPE_MAP:
		return h->hdr.map_bent;
	case JDB_BTYPE_TYPE_DEF:
		return h->hdr.typedef_bent;
	case JDB_BTYPE_TABLE_DEF:
		return JDB_ID_INVAL;
	case JDB_BTYPE_COL_TYPEDEF:
		return h->hdr.col_typedef_bent;
	case JDB_BTYPE_CELLDEF:
		return h->hdr.celldef_bent;
	case JDB_BTYPE_CELL_DATA:
		return JDB_ID_INVAL;
	case JDB_BTYPE_CELL_DATA_VAR:
		return JDB_ID_INVAL;
	case JDB_BTYPE_CELL_DATA_FIX:
		return JDB_ID_INVAL;
	case JDB_BTYPE_CELL_DATA_PTR:
		return h->hdr.dptr_bent;
	case JDB_BTYPE_INDEX:
		switch(itype){
			case JDB_ITYPE0:
				return h->hdr.index0_bent;
			case JDB_ITYPE1:
				return h->hdr.index1_bent;
			default:
				break;
		}
		return JDB_ID_INVAL;		
	case JDB_BTYPE_TABLE_FAV:
		return h->hdr.fav_bent;
	case JDB_BTYPE_SNAPSHOT:
		return JDB_ID_INVAL;
	case JDB_BTYPE_VOID:
		return JDB_ID_INVAL;
	default:
		break;
	}
	return JDB_BENT_INVAL;
}
