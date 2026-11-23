#include <dualform.h>
#include <string.h>
int main(void) {
    uint8_t zero[32]={0},cipher[16],plain[16];
    const uint8_t kat[16]={0x3a,0x68,0x31,0x01,0x4e,0x21,0xbb,0x35,0xe8,0x7d,0x4e,0x4e,0x45,0x8d,0xdd,0xa0};
    dualform_key_ctx *key=NULL;dualform_tweak_ctx *ctx=NULL;
    int result=1;
    if(dualform_key_ctx_create(&key,zero,32)!=DUALFORM_OK) goto done;
    if(dualform_tweak_ctx_create(&ctx,key,zero,16)!=DUALFORM_OK) goto done;
    dualform_key_ctx_free(key);key=NULL;
    if(dualform_encrypt_block(ctx,zero,16,cipher,16)!=DUALFORM_OK || memcmp(cipher,kat,16)) goto done;
    if(dualform_decrypt_block(ctx,cipher,16,plain,16)!=DUALFORM_OK || memcmp(plain,zero,16)) goto done;
    result=0;
done:
    dualform_tweak_ctx_free(ctx);dualform_key_ctx_free(key);return result;
}
