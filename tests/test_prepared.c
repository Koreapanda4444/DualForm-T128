#include "dualform.h"
#include "dualform_internal.h"
#include "shake256.h"
#include <assert.h>
#include <string.h>
int main(void) {
    uint8_t k[32]={0},t[16]={0},d[192];uint64_t sk[24];dualform_key_ctx *key=NULL;dualform_tweak_ctx *ctx=NULL;
    assert(dualform_key_ctx_create(&key,k,32)==0);
    assert(dualform_tweak_ctx_create(&ctx,key,t,16)==0);
    assert(ctx->sk[0]==UINT64_C(0xbdd7239a08f69e71));assert(ctx->sk[1]==UINT64_C(0xe22dc2ee713134df));
    df_span spans[]={{"DualForm-T128/derived",sizeof("DualForm-T128/derived")},{k,32},{t,16}};
    assert(df_shake(spans,3,d,192));
    for(unsigned i=2;i<24;i++) assert(ctx->sk[i]==(key->base[i]^df_load64(d+8*i)));
    assert(ctx->sk[0]!=(ctx->sk[0]^df_load64(d)));
    assert(ctx->sk[1]!=(ctx->sk[1]^df_load64(d+8)));
    memcpy(sk,ctx->sk,sizeof(sk));dualform_key_ctx_free(key);memset(k,255,sizeof(k));memset(t,255,sizeof(t));
    assert(!memcmp(sk,ctx->sk,sizeof(sk)));dualform_tweak_ctx_free(ctx);dualform_tweak_ctx_free(NULL);return 0;
}
