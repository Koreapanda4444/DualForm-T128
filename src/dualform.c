#include "dualform.h"
#include "dualform_internal.h"
#include "shake256.h"
#include <stdlib.h>
#include <string.h>
#include <openssl/crypto.h>
dualform_status dualform_key_ctx_create(dualform_key_ctx **out,const uint8_t *key,size_t len) {
    if(!out || *out || !key) return DUALFORM_EINVAL;
    if(len!=32) return DUALFORM_ELENGTH;
    dualform_key_ctx *ctx=calloc(1,sizeof(*ctx));
    if(!ctx) return DUALFORM_ENOMEM;
    uint8_t base[192];
    static const char domain[]="DualForm-T128/base";
    df_span spans[]={{domain,sizeof(domain)},{key,32}};
    if(!df_shake(spans,2,base,sizeof(base))) {
        OPENSSL_cleanse(base,sizeof(base)); dualform_key_ctx_free(ctx); return DUALFORM_ECRYPTO;
    }
    memcpy(ctx->key,key,32);
    for(unsigned i=0;i<24;i++) ctx->base[i]=df_load64(base+8*i);
    OPENSSL_cleanse(base,sizeof(base)); *out=ctx; return DUALFORM_OK;
}
void dualform_key_ctx_free(dualform_key_ctx *ctx) {
    if(ctx) { OPENSSL_cleanse(ctx,sizeof(*ctx)); free(ctx); }
}
dualform_status dualform_tweak_ctx_create(dualform_tweak_ctx **out,const dualform_key_ctx *key,const uint8_t *tweak,size_t len) {
    if(!out || *out || !key || !tweak) return DUALFORM_EINVAL;
    if(len!=16) return DUALFORM_ELENGTH;
    dualform_tweak_ctx *ctx=calloc(1,sizeof(*ctx));
    if(!ctx) return DUALFORM_ENOMEM;
    uint8_t derived[192];uint64_t enc[2]={0};
    static const char domain[]="DualForm-T128/derived";
    df_span spans[]={{domain,sizeof(domain)},{key->key,32},{tweak,16}};
    int ok=df_encode_tweak(key->key,tweak,enc) && df_shake(spans,3,derived,192);
    if(ok) {
        ctx->sk[0]=key->base[0]^enc[0];ctx->sk[1]=key->base[1]^enc[1];
        for(unsigned i=2;i<24;i++) ctx->sk[i]=key->base[i]^df_load64(derived+8*i);
    }
    OPENSSL_cleanse(derived,sizeof(derived));OPENSSL_cleanse(enc,sizeof(enc));
    if(!ok) { dualform_tweak_ctx_free(ctx);return DUALFORM_ECRYPTO; }
    *out=ctx;return DUALFORM_OK;
}
void dualform_tweak_ctx_free(dualform_tweak_ctx *ctx) {
    if(ctx) { OPENSSL_cleanse(ctx,sizeof(*ctx));free(ctx); }
}
