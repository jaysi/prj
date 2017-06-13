#include "jdb.h"

#define _wdeb_lk	_wdeb

int _jdb_init_handle_lock(struct jdb_handle *handle){

#ifndef NDEBUG
	jmxattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_setkind_np(&attr, PTHREAD_MUTEX_ERRORCHECK_NP);
#endif

#ifndef NDEBUG	
	_init_mx(&handle->lock_mx, &attr);
#else
	_init_mx(&handle->lock_mx, NULL);
#endif

	_init_cond(&handle->lock_cond, NULL);

	handle->rd_lock = 0;
	handle->wr_lock = 0;
	handle->rd_ref = 0;
	handle->wr_ref = 0;
}

int _jdb_destroy_handle_lock(struct jdb_handle *handle){
	_destroy_mx(&handle->lock_mx);
	_destroy_cond(&handle->lock_cond);
}

void _jdb_unset_handle_flag(struct jdb_handle *h, uint16_t flag, int lock){
	if (lock)
		_jdb_lock_handle(h);
	h->flags &= ~flag;
	if (lock)
		_jdb_unlock_handle(h);
}

void _jdb_set_handle_flag(struct jdb_handle *h, uint16_t flag, int lock)
{
	if (lock)
		_jdb_lock_handle(h);
	h->flags |= flag;
	if (lock)
		_jdb_unlock_handle(h);
}

jdb_hflag_t _jdb_get_handle_flag(struct jdb_handle *h, int lock)
{
	uint16_t flags;
	if (lock)
		_jdb_lock_handle(h);
	flags = h->flags;
	if (lock)
		_jdb_unlock_handle(h);
	return flags;
}

void _jdb_lock_handle(struct jdb_handle* h, jdb_hlock_t lock, int lock_type){
	_lock_mx(&h->lock_mx);
	
	if(lock_type == JDB_LK_WR){
		h->wr_ref++;
	}
	
	if(lock_type == JDB_LK_RD){
		h->rd_ref++;
	}	
	
	if(lock == JDB_LK_NONE) goto end;
	
	if(h->wr_lock){/*	if someone's writing (and reading)
				to the handle, you cannot read and/or write
				to/from the handle
			*/
		while(h->wr_lock & (lock)){
			_wait_cond(&h->lock_cond, &h->lock_mx);
		}
	} else {/*	there is not writes, but there maybe readers,
			do not allow writers...
		*/
		if(lock_type == JDB_LK_WR){
			while(h->wr_lock & (lock)){
				_wait_cond(&h->lock_cond, &h->lock_mx);
			}
		}
	}
	
	if(lock_type == JDB_LK_WR){
		h->wr_lock |= (lock);
	}
	if(lock_type == JDB_LK_RD){
		h->rd_lock |= (lock);
	}
end:
	_unlock_mx(&h->lock_mx);	
}

void _jdb_unlock_handle(struct jdb_handle* h, jdb_hlock_t lock, int lock_type){
	_lock_mx(&h->lock_mx);
	
	if(lock_type == JDB_LK_RD){
		assert(h->rd_ref);
		h->rd_ref--;
		//yes, waste a few cycles on LK_NONE!
		h->rd_lock &= ~lock;
	}
	if(lock_type == JDB_LK_WR){
		assert(h->wr_ref);
		h->wr_ref--;
		h->wr_lock &= ~lock;
	}
	_signal_cond(&h->lock_cond);
	_unlock_mx(&h->lock_mx);
}

int _jdb_init_table_lock(struct jdb_table *table){

#ifndef NDEBUG
	jmxattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_setkind_np(&attr, PTHREAD_MUTEX_ERRORCHECK_NP);
#endif

#ifndef NDEBUG	
	_init_mx(&table->lock_mx, &attr);
#else
	_init_mx(&table->lock_mx, NULL);
#endif

	_init_cond(&table->lock_cond, NULL);
	
	table->lock_wr = 0;
	table->lock_rd = 0;
}

int _jdb_destroy_table_lock(struct jdb_table *table){
	_destroy_mx(&table->lock_mx);
	_destroy_cond(&table->lock_cond);
}

int _jdb_lock_table(	struct jdb_handle* h, wchar_t* tname,
			int lock, struct jdb_table** table
		){

	struct jdb_table* table;

	_lock_mx(&h->lock_mx);

	ret = _jdb_table_handle(h, tname, table);

	if(!ret){
		_lock_mx(&(*table)->lock_mx);

		if(lock == JDB_LK_WR){
			h->wr_ref++;
			(*table)->lock_wr++;
		}
		if(lock == JDB_LK_RD){
			h->rd_ref++;
			(*table)->lock_rd++;
		}

		_unlock_mx(&h->lock_mx);

		/*
			if WR is set, someone's writing to the table,
			wait to finish.

			if only RD is set, someone's reading from the table,
			don't block the other readers (with RD request),
			but avoid WR locks.
		*/

		if((*table)->lock_wr){ 
			while((*table)->lock_wr){
				_wait_cond(&(*table)->lock_cond, &(*table)->lock_mx);
			}			
		} else if((*table)->lock_rd){
			if(lock == JDB_LK_WR){
				while((*table)->lock_wr){
					_wait_cond(	&(*table)->lock_cond,
							&(*table)->lock_mx);
				}
			}
		}
		
		_unlock_mx(&(*table)->lock_mx);		
	} else {
		_unlock_mx(&h->lock_mx);
	}

	return ret;
}

int _jdb_unlock_table(	struct jdb_handle* h, wchar_t* tname,
			int lock, struct jdb_table** table
		){
	struct jdb_table* table;
	
	_lock_mx(&h->lock_mx);
	
	ret = _jdb_table_handle(h, tname, table);
	
	if(!ret){
		_lock_mx(&(*table)->lock_mx);		

		if(lock == JDB_LK_WR && (*table)->lock_wr){
			assert(h->wr_ref);
			h->wr_ref--;
			(*table)->lock_wr--;
		}

		if(lock == JDB_LK_RD && (*table)->lock_rd){
			assert(h->rd_ref);
			h->rd_ref--;
			(*table)->lock_rd--;		
		}
		
		_unlock_mx(&h->lock_mx);
		
		_signal_cond(&(*table)->lock_cond);
		_unlock_mx(&(*table)->lock_mx);
	} else {
		_unlock_mx(&h->lock_mx);
	}

	return ret;	
}

