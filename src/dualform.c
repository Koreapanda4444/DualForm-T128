#include "dualform.h"
#include "dualform_internal.h"
#include "shake256.h"
#include "df_memory.h"
#include <string.h>
#include <openssl/crypto.h>
dualform_status dualform_key_ctx_create(dualform_key_ctx **out,const uint8_t *key,size_t len) {
    if(!out || *out || !key) return DUALFORM_EINVAL;
    if(len!=32) return DUALFORM_ELENGTH;
    dualform_key_ctx *ctx=df_alloc(sizeof(*ctx));
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
    if(ctx) { OPENSSL_cleanse(ctx,sizeof(*ctx)); df_release(ctx,sizeof(*ctx)); }
}
dualform_status dualform_tweak_ctx_create(dualform_tweak_ctx **out,const dualform_key_ctx *key,const uint8_t *tweak,size_t len) {
    if(!out || *out || !key || !tweak) return DUALFORM_EINVAL;
    if(len!=16) return DUALFORM_ELENGTH;
    dualform_tweak_ctx *ctx=df_alloc(sizeof(*ctx));
    if(!ctx) return DUALFORM_ENOMEM;
    uint8_t derived[192];uint64_t enc[2]={0};
    static const char domain[]="DualForm-T128/derived";
    df_span spans[]={{domain,sizeof(domain)},{key->key,32},{tweak,16}};
    EVP_MD_CTX *hash=df_shake_new();
    int ok=hash && df_encode_tweak_with(hash,key->key,tweak,enc) && df_shake_with(hash,spans,3,derived,192);
    df_shake_free(hash);
    if(ok) {
        ctx->sk[0]=key->base[0]^enc[0];ctx->sk[1]=key->base[1]^enc[1];
        for(unsigned i=2;i<24;i++) ctx->sk[i]=key->base[i]^df_load64(derived+8*i);
    }
    OPENSSL_cleanse(derived,sizeof(derived));OPENSSL_cleanse(enc,sizeof(enc));
    if(!ok) { dualform_tweak_ctx_free(ctx);return DUALFORM_ECRYPTO; }
    *out=ctx;return DUALFORM_OK;
}
void dualform_tweak_ctx_free(dualform_tweak_ctx *ctx) {
    if(ctx) { OPENSSL_cleanse(ctx,sizeof(*ctx));df_release(ctx,sizeof(*ctx)); }
}
static int df_overlap(const void *a,size_t an,const void *b,size_t bn) {
    uintptr_t x=(uintptr_t)a,y=(uintptr_t)b;
    if(x>UINTPTR_MAX-an || y>UINTPTR_MAX-bn) return 1;
    return x<y+bn && y<x+an;
}
static dualform_status df_check_block(const dualform_tweak_ctx *ctx,const uint8_t *in,size_t len,uint8_t *out,size_t cap) {
    if(!ctx || !in || !out) return DUALFORM_EINVAL;
    if(len!=16) return DUALFORM_ELENGTH;
    if(cap<16) return DUALFORM_ECAPACITY;
    if(in!=out && df_overlap(in,16,out,16)) return DUALFORM_EOVERLAP;
    if(df_overlap(in,16,ctx,sizeof(*ctx)) || df_overlap(out,16,ctx,sizeof(*ctx))) return DUALFORM_EOVERLAP;
    return DUALFORM_OK;
}
dualform_status dualform_encrypt_block(const dualform_tweak_ctx *ctx,const uint8_t *in,size_t len,uint8_t *out,size_t cap) {
    dualform_status st=df_check_block(ctx,in,len,out,cap);if(st) return st;
    uint64_t l=df_load64(in),r=df_load64(in+8);
    for(unsigned i=0;i<24;i++) {
        uint64_t next=l^((i&1)?df_odd(r^ctx->sk[i]):df_even(r^ctx->sk[i])); l=r;r=next;
    }
    uint8_t result[16];df_store64(result,l);df_store64(result+8,r);memcpy(out,result,16);
    OPENSSL_cleanse(result,sizeof(result));return DUALFORM_OK;
}
dualform_status dualform_decrypt_block(const dualform_tweak_ctx *ctx,const uint8_t *in,size_t len,uint8_t *out,size_t cap) {
    dualform_status st=df_check_block(ctx,in,len,out,cap);if(st) return st;
    uint64_t l=df_load64(in),r=df_load64(in+8);
    for(int i=23;i>=0;i--) {
        uint64_t prev=r^((i&1)?df_odd(l^ctx->sk[i]):df_even(l^ctx->sk[i])); r=l;l=prev;
    }
    uint8_t result[16];df_store64(result,l);df_store64(result+8,r);memcpy(out,result,16);
    OPENSSL_cleanse(result,sizeof(result));return DUALFORM_OK;
}
