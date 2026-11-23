#include "dualform.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
int main(void) {
    const char *expected[]={"3a6831014e21bb35e87d4e4e458ddda0","cab683110ef545617d4e892a779daacf","d653cb3700ff90e79631907cbebe9d8e"};
    for(unsigned v=0;v<3;v++) {
        uint8_t k[32],t[16],p[16],out[18];
        for(unsigned i=0;i<32;i++) k[i]=(uint8_t)(v==0?0:v==1?i:255);
        memcpy(t,k,16);memcpy(p,k,16);
        dualform_key_ctx *key=NULL;dualform_tweak_ctx *ctx=NULL;
        assert(dualform_key_ctx_create(&key,k,32)==0 && dualform_tweak_ctx_create(&ctx,key,t,16)==0);
        dualform_key_ctx_free(key);memset(out,0xa5,sizeof(out));
        assert(dualform_encrypt_block(ctx,p,16,out,18)==0);
        for(unsigned i=0;i<16;i++) { unsigned b;assert(sscanf(expected[v]+2*i,"%2x",&b)==1);assert(out[i]==b); }
        assert(out[16]==0xa5 && out[17]==0xa5);
        assert(dualform_encrypt_block(ctx,p,16,p,16)==0 && !memcmp(p,out,16));
        assert(dualform_encrypt_block(ctx,p,15,out,18)==DUALFORM_ELENGTH);
        assert(!memcmp(p,out,16));
        dualform_tweak_ctx_free(ctx);
    }
    return 0;
}
