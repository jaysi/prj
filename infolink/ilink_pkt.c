#include "include/ilink_intern.h"

#define _deb_prep1 _NullMsg

#define _ilink_pack_pkt_hdr(hdr, data) \
    MACRO( \
        pack13(data, \
		sizeof(struct ilink_pkt_hdr),\
        ILINK_PKT_HDR_PACKSTR, \
        (hdr).type, \
        (hdr).id, \
        (hdr).datalen, \
        (hdr).datacrc32);\
    )

#define _ilink_unpack_pkt_hdr(data, hdr) \
    MACRO( \
        unpack13(data, \
		sizeof(struct ilink_pkt_hdr),\
        ILINK_PKT_HDR_PACKSTR, \
        &((hdr).type), \
        &((hdr).id), \
        &((hdr).datalen), \
        &((hdr).datacrc32)); \
    )

ilink_ppkt_flag_t _ilink_flags_to_ppkt(ilink_flag_t flags){

    ilink_ppkt_flag_t ret = 0;

    if(flags & ILINK_FLAG_ASYM) ret |= ILINK_PPKT_ASYM;
    if(flags & ILINK_FLAG_NOPACK) ret |= ILINK_PPKT_NOPACK;
    if(flags & ILINK_FLAG_NOCRC) ret |= ILINK_PPKT_NOCRC;

    return ret;

}

error13_t ilink_prepare_pkt(struct infolink* link,
                            ilink_data_t* data,
                            ilink_datalen_t datalen,
                            ilink_pkt_type_t type,
                            ilink_ppkt_flag_t flags){

    struct crypt13* c;
    struct ilink_pkt_hdr hdr;
    error13_t ret;
    struct ilink_buf* ibuf;

    _deb_prep1("buf: %s, datalen: %u", data, datalen);

    if(flags & ILINK_PPKT_ASYM){
        _deb_prep1("asym crypt");
        c = &link->asym_crypt;
    } else {
        _deb_prep1("sym crypt");
        c = &link->sym_crypt;
    }

    _deb_prep1("crypt13 ptr address: %p", c);

    ibuf = (struct ilink_buf*)m13_malloc(sizeof(struct ilink_buf));
    if(!ibuf){
        return e13_error(E13_NOMEM);
    }

    ibuf->next = NULL;
    ibuf->pos = 0UL;

    ibuf->buf = (ilink_data_t*)m13_malloc(datalen + ILINK_PKT_HDR_SIZE);
    if(!ibuf->buf){
        m13_free(ibuf);
        return e13_error(E13_NOMEM);
    }

    hdr.type = type;
    hdr.datalen = datalen;
    if(!(flags & ILINK_PPKT_NOCRC)){
        hdr.datacrc32 = datalen?h13_crc32(data, datalen):0;
    } else {
        hdr.datacrc32 = 0UL;
    }
    //TODO: TEMP!
    hdr.id = hdr.datacrc32;

    if(!(flags & ILINK_PPKT_NOPACK)){
        _ilink_pack_pkt_hdr(hdr, (ilink_udata_t*)ibuf->buf);
    }

    _deb_prep1("hdr.datalen = %u, hdr.type = %u, hdr.id = %u", hdr.datalen, hdr.type, hdr.id);

    if(datalen) memcpy(ibuf->buf + ILINK_PKT_HDR_SIZE, data, datalen);

    if((ret = crypt13_encrypt(c, (ilink_udata_t*)ibuf->buf,
                              datalen + ILINK_PKT_HDR_SIZE,
                              (ilink_udata_t*)ibuf->buf,
                              &ibuf->bufsize)) != E13_OK){
        _deb_prep1("crypt ret %i", ret);
        m13_free(ibuf->buf);
        m13_free(ibuf);
        return ret;
    }

    if(!link->send_list.first){
        link->send_list.first = ibuf;
        link->send_list.last = ibuf;
        link->send_list.n = 1UL;
    } else {
        link->send_list.last->next = ibuf;
        link->send_list.last = ibuf;
        link->send_list.n++;
    }

    return E13_OK;

}

error13_t ilink_pkt_hdr(struct infolink* link,
                        ilink_data_t* data,
                        ilink_datalen_t datalen,
                        struct ilink_pkt_hdr* hdr,
                        ilink_ppkt_flag_t flags){

    struct crypt13* c;
    ilink_datalen_t hlen;
    ilink_data_t hbuf[ILINK_PKT_HDR_SIZE];

    if(flags & ILINK_PPKT_ASYM){
		_deb_prep1("asym crypt");
        c = &link->asym_crypt;
    } else {
    	_deb_prep1("sym crypt");
        c = &link->sym_crypt;
    }

    _deb_prep1("crypt13 ptr address: %p", c);

    hlen = crypt13_enc_size(c, ILINK_PKT_HDR_SIZE);

    if(datalen < hlen){
        _deb_prep1("datalen < hlen (%u < %u)", datalen, hlen);
        return e13_error(E13_TOOSMALL);
    }

    crypt13_decrypt(c, (ilink_udata_t*)data, hlen,
                    (ilink_udata_t*)hbuf, &hlen);

    if(!(flags & ILINK_PPKT_NOPACK)){
        _ilink_unpack_pkt_hdr((ilink_udata_t*)hbuf, *hdr);
    }

    _deb_prep1("hdr.datalen = %u, hdr.type = %u, hdr.id = %u", hdr->datalen, hdr->type, hdr->id);

    return E13_OK;

}

error13_t ilink_explode_pkt(struct infolink* link,
                            ilink_data_t** data,
                            ilink_datalen_t* datalen,
                            ilink_pkt_type_t* type,
                            ilink_ppkt_flag_t flags){

    struct crypt13* c;
    struct ilink_pkt_hdr hdr;
    uint32_t datacrc32;
    error13_t ret;
    struct ilink_buf* del;

    _deb_prep1("link->sock = %i", link->sock);

    if(!link->rcvd_list.first) {
//        link->rcvd_list.first = (struct ilink_buf*)m13_malloc(sizeof(struct ilink_buf));
//        link->rcvd_list.first->next = NULL;
//        link->rcvd_list.last = link.rcvd_list.first;
        _deb_prep1("list empty");
        return e13_error(E13_EMPTY);
    }

    if(flags & ILINK_PPKT_ASYM){
		_deb_prep1("asym crypt");
        c = &link->asym_crypt;
    } else {
    	_deb_prep1("sym crypt");
        c = &link->sym_crypt;
    }

    _deb_prep1("crypt13 ptr address: %p", c);

    /*the recvbuf is already allocated for enc_size(datalen + PKT_HDR_LEN)

    link->recvbuf = (ilink_data_t*)m13_malloc(datalen + ILINK_PKT_HDR_SIZE);
    if(!link->recvbuf){
        return e13_error(E13_NOMEM);
    }
    */

    if((ret = crypt13_decrypt(c, (ilink_udata_t*)link->rcvd_list.first->buf,
                              link->rcvd_list.first->bufsize,
                              (ilink_udata_t*)link->rcvd_list.first->buf,
                              &link->rcvd_list.first->bufsize)) != E13_OK){
        _deb_prep1("decrypt failed ret = %i", ret);
        return ret;
    }

    _deb_prep1("data = %s", link->rcvd_list.first->buf + ILINK_PKT_HDR_SIZE);

    if(!(flags & ILINK_PPKT_NOPACK)){
        _ilink_unpack_pkt_hdr((ilink_udata_t*)link->rcvd_list.first->buf, hdr);
    }

    *datalen = hdr.datalen;
    if(*datalen) *data = link->rcvd_list.first->buf + ILINK_PKT_HDR_SIZE;
    else *data = NULL;

    if(!(flags & ILINK_PPKT_NOCRC)){
        datacrc32 = (*datalen)?h13_crc32(*data, *datalen):0;
        if(datacrc32 != hdr.datacrc32){
            _deb_prep1("E13_CORRUPT");
            return e13_error(E13_CORRUPT);
        }
    }

    _deb_prep1("hdr.type = %u", hdr.type);

    *type = hdr.type;

    if(/*flags & ILINK_PPKT_COPY && TODO: always copy?*/ *datalen){
        *data = m13_malloc(*datalen);
        if(!(*data)) return e13_error(E13_NOMEM);
        memcpy(*data, link->rcvd_list.first->buf + ILINK_PKT_HDR_SIZE, *datalen);
    }

    m13_free(link->rcvd_list.first->buf);
    link->rcvd_list.n--;
    del = link->rcvd_list.first;
    link->rcvd_list.first = link->rcvd_list.first->next;
    //TODO: needed? if(del == link->rcvd_list.last) link->rcvd_list.last = NULL;
    m13_free(del);

    return E13_OK;

}
