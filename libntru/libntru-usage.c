#include "ntru.h"

/* key generation */
struct NtruEncParams params = APR2011_439_FAST; /*see encparams.h for more*/
NtruEncKeyPair kp;
if (!ntru_gen_key_pair(&params, &kp, dev_urandom))
    printf("keygen fail\n");

/* encryption */
strcpy(msg, "whatever");
char enc[ntru_enc_len(params.N, params.q)];
if (ntru_encrypt(msg, strlen(msg), &kp.pub, &params, dev_urandom, enc) != 0)
    printf("encrypt fail\n");

/* decryption */
char dec[ntru_max_msg_len(params)];
int dec_len;
if (ntru_decrypt((char*)&enc, &kp, &params, (unsigned char*)&dec, &dec_len) != 0)
    printf("decrypt fail\n");

/* export key to char array */
char pub_arr[ntru_enc_len(params.N, params.q)];
ntru_export_pub(&kp.pub, pub_arr);

/* import key from char array */
NtruEncPubKey pub;
ntru_import_pub(pub_arr, &pub);