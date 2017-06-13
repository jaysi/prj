#include "include/ilink_intern.h"

#define _deb1 _NullMsg

error13_t _ilink_init_asym_crypto(struct infolink* link, uint8_t* key, size_t keylen){

    uint8_t* k;
    size_t l;
    error13_t ret;

    //the key is random generated

    if(!key){
        //generate random key
        l = ILINK_DEF_ASYM_KEY_LEN;
        k = (uint8_t*)m13_malloc(l);
        if(!k) return e13_ierror(link->e, E13_NOMEM, "s", "malloc");
        //TODO: FILL
    } else {
        k = key;
        l = keylen;
    }

    _deb1("alg: %i", link->conf.asym_alg);

    ret = crypt13_init(&link->asym_crypt, link->conf.asym_alg, k, l);

    _deb1("crypt13_init() returns %s, crypt13 ptr address %p", e13_codemsg(ret), &link->asym_crypt);

    if(!key) m13_free(k);
    return ret == E13_OK?e13_cleanup(link->e):e13_ierror(link->e, ret, "s", "crypt init");

}

error13_t _ilink_init_sym_crypto(struct infolink* link, uint8_t* key, size_t keylen){

    uint8_t* k;
    size_t l;
    error13_t ret;

    if(!key){
        //generate random key
        l = ILINK_DEF_SYM_KEY_LEN;
        k = (uint8_t*)m13_malloc(l);
        if(!k) return e13_ierror(link->e, E13_NOMEM, "s", "malloc");
        //TODO: FILL
    } else {
        k = key;
        l = keylen;
    }

    _deb1("alg: %i", link->conf.sym_alg);

    ret = crypt13_init(&link->sym_crypt, link->conf.sym_alg, k, l);

    _deb1("crypt13_init() returns %s, crypt13 ptr address %p", e13_codemsg(ret), &link->sym_crypt);

    if(!key) m13_free(k);
    return ret == E13_OK?e13_cleanup(link->e):e13_ierror(link->e, ret, "s", "crypt init");
}

error13_t _ilink_asym_encrypt(struct infolink* link, ilink_data_t* in, ilink_datalen_t insize, ilink_data_t* out, ilink_datalen_t *outsize){

    return crypt13_encrypt(&link->asym_crypt, (ilink_udata_t*)in, insize, (ilink_udata_t*)out, outsize);

}

error13_t _ilink_sym_encrypt(struct infolink* link, ilink_data_t* in, ilink_datalen_t insize, ilink_data_t* out, ilink_datalen_t *outsize){

    return crypt13_encrypt(&link->asym_crypt, (ilink_udata_t*)in, insize, (ilink_udata_t*)out, outsize);

}

error13_t _ilink_asym_decrypt(struct infolink* link, ilink_data_t* in, ilink_datalen_t insize, ilink_data_t* out, ilink_datalen_t *outsize){

    return crypt13_decrypt(&link->asym_crypt, (ilink_udata_t*)in, insize, (ilink_udata_t*)out, outsize);

}

error13_t _ilink_sym_decrypt(struct infolink* link, ilink_data_t* in, ilink_datalen_t insize, ilink_data_t* out, ilink_datalen_t *outsize){

    return crypt13_decrypt(&link->asym_crypt, (ilink_udata_t*)in, insize, (ilink_udata_t*)out, outsize);

}

