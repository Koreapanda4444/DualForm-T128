#include "dualform.h"
#include "research_helpers.h"
#include <stdio.h>
#include <string.h>
static int unhex(const char *s,uint8_t *out,size_t n) {
    if(strlen(s)!=n*2) return 0;
    for(size_t i=0;i<n;i++) { unsigned v;if(sscanf(s+2*i,"%2x",&v)!=1) return 0;out[i]=(uint8_t)v; }return 1;
}
static void hex(const uint8_t *b,size_t n) { for(size_t i=0;i<n;i++) printf("%02x",b[i]); }
int main(void) {
    char line[256],kh[65],th[33],ph[33];
    while(fgets(line,sizeof(line),stdin)) {
        uint8_t k[32],t[16],p[16],e[16],c[16],d[16]; uint64_t enc[2];
        if(sscanf(line,"%64s %32s %32s",kh,th,ph)!=3 || !unhex(kh,k,32) || !unhex(th,t,16) || !unhex(ph,p,16)) return 2;
        dualform_key_ctx *key=NULL;dualform_tweak_ctx *ctx=NULL;
        if(dualform_key_ctx_create(&key,k,32) || dualform_tweak_ctx_create(&ctx,key,t,16) || !df_encode_tweak(k,t,enc)) return 3;
        df_store64(e,enc[0]);df_store64(e+8,enc[1]);
        printf("{\"encoded\":\"");hex(e,16);printf("\",\"sk\":[");
        for(unsigned i=0;i<24;i++) printf("%s\"%016llx\"",i?",":"",(unsigned long long)ctx->sk[i]);
        printf("],\"trace\":[");
        for(unsigned q=0;q<=24;q++) {df_prefix(ctx->sk,p,c,q,0);printf("%s\"",q?",":"");hex(c,16);printf("\"");}
        printf("],\"inverse\":[");
        for(unsigned q=0;q<=24;q++) {df_prefix(ctx->sk,p,c,q,0);df_prefix(ctx->sk,c,d,q,1);printf("%s\"",q?",":"");hex(d,16);printf("\"");}
        if(dualform_encrypt_block(ctx,p,16,c,16) || dualform_decrypt_block(ctx,c,16,d,16)) return 4;
        printf("],\"public_decrypted\":\"");hex(d,16);printf("\"}\n");
        dualform_tweak_ctx_free(ctx);dualform_key_ctx_free(key);
    }
    return ferror(stdin)?5:0;
}
