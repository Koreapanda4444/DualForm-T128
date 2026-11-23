#include "dualform.h"
#include "dualform_internal.h"
#include "df_memory.h"
#include <assert.h>
#include <string.h>
static void reset(long n) {df_test_fail_after=n;df_test_calls=0;}
int main(void) {
    uint8_t k[32]={0},t[16]={0},p[16]={0},a[16],b[16];dualform_key_ctx *key=NULL;dualform_tweak_ctx *ctx=NULL;
    reset(-1);assert(dualform_key_ctx_create(&key,k,32)==0);size_t calls=df_test_calls;dualform_key_ctx_free(key);key=NULL;
    for(size_t i=0;i<calls;i++) {
        reset((long)i);dualform_status s=dualform_key_ctx_create(&key,k,32);
        assert(s==(i==0?DUALFORM_ENOMEM:DUALFORM_ECRYPTO));assert(!key && df_test_live==0);
        assert(!memcmp(k,(uint8_t[32]){0},32));
    }
    reset(-1);assert(dualform_key_ctx_create(&key,k,32)==0);
    reset(-1);assert(dualform_tweak_ctx_create(&ctx,key,t,16)==0);calls=df_test_calls;dualform_tweak_ctx_free(ctx);ctx=NULL;
    for(size_t i=0;i<calls;i++) {
        reset((long)i);dualform_status s=dualform_tweak_ctx_create(&ctx,key,t,16);
        assert(s==(i==0?DUALFORM_ENOMEM:DUALFORM_ECRYPTO));assert(!ctx && df_test_live==1);
        assert(!memcmp(t,(uint8_t[16]){0},16));
    }
    reset(-1);assert(dualform_tweak_ctx_create(&ctx,key,t,16)==0);
    assert(dualform_encrypt_block(ctx,p,16,a,16)==0);dualform_key_ctx_free(key);
    assert(dualform_encrypt_block(ctx,p,16,b,16)==0 && !memcmp(a,b,16));
    dualform_tweak_ctx_free(ctx);assert(df_test_live==0);return 0;
}
