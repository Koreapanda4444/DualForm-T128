#include "dualform.h"
#include <assert.h>
#include <string.h>
typedef dualform_status (*block_fn)(const dualform_tweak_ctx*,const uint8_t*,size_t,uint8_t*,size_t);
int main(void) {
    uint8_t k[32]={0},t[16]={0};dualform_key_ctx *key=NULL;dualform_tweak_ctx *ctx=NULL;
    assert(dualform_key_ctx_create(&key,k,32)==0 && dualform_tweak_ctx_create(&ctx,key,t,16)==0);
    block_fn functions[]={dualform_encrypt_block,dualform_decrypt_block};
    for(unsigned f=0;f<2;f++) {
        block_fn fn=functions[f];uint8_t b[64],before[64],in[32],saved[32];
        memset(in,0xa6,sizeof(in));memcpy(saved,in,sizeof(in));
        size_t lengths[]={0,1,15,17,32};
        for(unsigned i=0;i<5;i++) {
            memset(b,0x5b,sizeof(b));memcpy(before,b,sizeof(b));
            assert(fn(ctx,in,lengths[i],b+1,32)==DUALFORM_ELENGTH);assert(!memcmp(b,before,64));assert(!memcmp(in,saved,32));
        }
        for(size_t cap=0;cap<16;cap++) {
            assert(fn(ctx,in,16,b+1,cap)==DUALFORM_ECAPACITY);assert(!memcmp(b,before,64));
        }
        assert(fn(NULL,in,16,b+1,32)==DUALFORM_EINVAL);
        assert(fn(ctx,NULL,16,b+1,32)==DUALFORM_EINVAL);
        assert(fn(ctx,in,16,NULL,32)==DUALFORM_EINVAL);
        for(unsigned d=1;d<16;d++) {
            memset(b,0x7b,sizeof(b));memcpy(before,b,64);
            assert(fn(ctx,b+16,16,b+16+d,16)==DUALFORM_EOVERLAP);assert(!memcmp(b,before,64));
            assert(fn(ctx,b+16+d,16,b+16,16)==DUALFORM_EOVERLAP);assert(!memcmp(b,before,64));
        }
        memset(b,0x5b,64);assert(fn(ctx,in,16,b+1,32)==0);
        assert(b[0]==0x5b);for(unsigned i=17;i<64;i++) assert(b[i]==0x5b);
        memset(b,0x5b,64);assert(fn(ctx,b+1,16,b+1,16)==0);
        assert(b[0]==0x5b);for(unsigned i=17;i<64;i++) assert(b[i]==0x5b);
        assert(fn(ctx,b+1,16,b+17,16)==0);
    }
    dualform_tweak_ctx *slot=NULL;
    assert(dualform_tweak_ctx_create(&slot,key,t,15)==DUALFORM_ELENGTH && !slot);
    assert(dualform_tweak_ctx_create(&slot,NULL,t,16)==DUALFORM_EINVAL && !slot);
    assert(dualform_tweak_ctx_create(&slot,key,NULL,16)==DUALFORM_EINVAL && !slot);
    slot=ctx;assert(dualform_tweak_ctx_create(&slot,key,t,16)==DUALFORM_EINVAL && slot==ctx);
    dualform_tweak_ctx_free(ctx);dualform_key_ctx_free(key);return 0;
}
