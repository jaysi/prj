#include "jdb.h"

/*	header and handle	*/

uint64_t _jdb_get_chid(struct jdb_handle *h, int lock)
{
	uint64_t chid;
	if (lock)
		_jdb_lock_handle(h);
	chid = h->hdr.chid++;	
	if (lock)
		_jdb_unlock_handle(h);
		
	return chid;
}

int _jdb_hdr_to_buf(struct jdb_handle* h, uchar** buf, int alloc){
	
	if(alloc){
		*buf = (uchar*)malloc(JDB_HDR_SIZE);
		if(!(*buf)) return -JE_MALOC;
	}
	
	_jdb_lock_handle(h, JDB_LK_HDR, JDB_LK_WR);

	if (!(_jdb_get_handle_flag(h, 0) & JDB_HMODIF)) {
		_jdb_unlock_handle(h, JDB_LK_HDR, JDB_LK_WR);
		if(alloc) free(*buf);
		return -JE_EMPTY;
	}

	_jdb_pack_hdr(&h->hdr, *buf);

	_jdb_encrypt(h, *buf, *buf, JDB_HDR_SIZE);

	_jdb_unset_handle_flag(h, JDB_HMODIF, 0);

	_jdb_unlock_handle(h, JDB_LK_HDR, JDB_LK_WR);

	return 0;
}
/*
int _jdb_write_hdr(struct jdb_handle *h)
{
	uchar buf[JDB_HDR_SIZE];
	int ret;

	_jdb_lock_handle(h);

	if (!(_jdb_get_handle_flag(h, 0) & JDB_HMODIF)) {
		_jdb_unlock_handle(h);		
		return 0;
	}
	
	_wdeb_wr(L"writing header...");

	_jdb_pack_hdr(&h->hdr, buf);

	_jdb_encrypt(h, buf, buf, JDB_HDR_SIZE);

	_jdb_unset_handle_flag(h, JDB_HMODIF, 0);

	_jdb_unlock_handle(h);

	ret = _jdb_seek_write(h, buf, JDB_HDR_SIZE, 0);

	if (ret) {
		_jdb_set_handle_flag(h, JDB_HMODIF, 1);
	}

	return ret;
}
*/
int _jdb_read_hdr(struct jdb_handle *h)
{
	uchar buf[JDB_HDR_SIZE];
	int ret;
	
	//Locking is not needed, the header is read ONCE at the startup

	ret = _jdb_seek_read(h, buf, JDB_HDR_SIZE, 0);

	if (ret < 0){
		_wdeb(L"failed to read, %i", ret);
		return ret;
	}

	_jdb_decrypt(h, buf, buf, JDB_HDR_SIZE);

	_jdb_lock_handle(h, JDB_LK_HDR, JDB_LK_WR);

	ret = _jdb_unpack_hdr(&h->hdr, buf);
	_jdb_unset_handle_flag(h, JDB_HMODIF, 0);

	_jdb_unlock_handle(h, JDB_LK_HDR, JDB_LK_WR);

	if (ret < 0)
		_jdb_set_handle_flag(h, JDB_HMODIF, 1);

	return ret;
}
