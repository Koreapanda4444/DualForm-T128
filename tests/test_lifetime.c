#include "dualform.h"
#include <assert.h>
#include <string.h>
int main(void) {
    for(unsigned n=0;n<1000;n++) {
        uint8_t k[32]={0},t[16]={0},p[16]={0},c[16],d[16];
        k[0]=(uint8_t)n;k[1]=(uint8_t)(n>>8);t[0]=(uint8_t)(n*7);p[1]=(uint8_t)n;
        dualform_key_ctx *key=NULL;dualform_tweak_ctx *ctx=NULL;
        assert(dualform_key_ctx_create(&key,k,32)==0 && dualform_tweak_ctx_create(&ctx,key,t,16)==0);
        dualform_key_ctx_free(key);
        assert(dualform_encrypt_block(ctx,p,16,c,16)==0 && dualform_decrypt_block(ctx,c,16,d,16)==0);
        assert(!memcmp(p,d,16));dualform_tweak_ctx_free(ctx);
    }
    return 0;
}
