#include "dualform.h"
#include "research_helpers.h"
#include "timer.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static volatile sig_atomic_t stop=0;
static void interrupt(int sig) { (void)sig;stop=1; }
static int sample(const char *label,uint64_t j,uint32_t r,uint64_t i,uint8_t *out,size_t n) {
    char domain[96];uint8_t indices[20];
    int len=snprintf(domain,sizeof(domain),"DualForm-T128/test/%s",label);
    if(len<0 || (size_t)len>=sizeof(domain)) return 0;
    df_store64(indices,j);df_store32(indices+8,r);df_store64(indices+12,i);
    df_span spans[]={{domain,(size_t)len+1},{indices,20}};return df_shake(spans,2,out,n);
}
int main(int argc,char **argv) {
    if(argc!=8) return 2;
    int related=!strcmp(argv[1],"related");
    if(!related && strcmp(argv[1],"bias")) return 2;
    unsigned j=(unsigned)strtoul(argv[2],NULL,10),r=(unsigned)strtoul(argv[3],NULL,10),n=(unsigned)strtoul(argv[4],NULL,10);
    unsigned v=(unsigned)strtoul(argv[5],NULL,10);double seconds=strtod(argv[6],NULL);
    if(j>=8 || r>24 || n>32768 || v>3 || (!related && v) || seconds<0 || strcmp(argv[7],"v1")) return 2;
    signal(SIGTERM,interrupt);signal(SIGINT,interrupt);
    double start=df_now();if(start<0) return 3;
    uint8_t k[32],t[16],t2[16],p[16],p2[16],c[16],c2[16];
    unsigned counts[128]={0},executed=0;
    dualform_key_ctx *key=NULL;dualform_tweak_ctx *ctx=NULL,*ctx2=NULL;
    if(!sample(related?"related-key":"bias-key",j,0,0,k,32)) goto fail;
    if(dualform_key_ctx_create(&key,k,32)) goto fail;
    if(!related && (!sample("bias-tweak",j,0,0,t,16) || dualform_tweak_ctx_create(&ctx,key,t,16))) goto fail;
    const uint64_t aa[]={1,0,1,UINT64_C(0x8000000000000000)},bb[]={0,1,1,UINT64_C(0x8000000000000000)};
    for(unsigned i=0;i<n;i++) {
        if((i&63u)==0 && (stop || df_now()-start>=seconds)) break;
        if(related) {
            if(!sample("related-tweak",4*j+v,r,i,t,16) || !sample("related-plain",4*j+v,r,i,p,16)) goto fail;
            df_store64(t2,df_load64(t)^aa[v]);df_store64(t2+8,df_load64(t+8)^bb[v]);
            df_store64(p2,df_load64(p)^bb[v]);df_store64(p2+8,df_load64(p+8)^aa[v]);
            if(dualform_tweak_ctx_create(&ctx,key,t,16) || dualform_tweak_ctx_create(&ctx2,key,t2,16)) goto fail;
        } else {
            if(!sample("bias-plain",j,r,i,p,16)) goto fail;
            memcpy(p2,p,16);p2[3]^=0x80;
        }
        df_prefix(ctx->sk,p,c,r,0);df_prefix(related?ctx2->sk:ctx->sk,p2,c2,r,0);
        if(related) {
            for(unsigned bit=0;bit<128;bit++) counts[bit]+=((c[bit/8]^c2[bit/8])>>(bit%8))&1u;
            dualform_tweak_ctx_free(ctx);ctx=NULL;dualform_tweak_ctx_free(ctx2);ctx2=NULL;
        } else counts[0]+=((c[0]^c2[0])>>1)&1u;
        executed++;
    }
    dualform_tweak_ctx_free(ctx);dualform_key_ctx_free(key);
    printf("{\"requested\":%u,\"executed\":%u,\"counts\":[",n,executed);
    for(unsigned i=0;i<(related?128u:1u);i++) printf("%s%u",i?",":"",counts[i]);
    printf("],\"rounds\":%u,\"key_index\":%u,\"delta_index\":%u,\"elapsed_seconds\":%.9f,\"status\":\"%s\"}\n",r,j,v,df_now()-start,executed==n?"completed":"incomplete");return 0;
fail:
    dualform_tweak_ctx_free(ctx);dualform_tweak_ctx_free(ctx2);dualform_key_ctx_free(key);return 3;
}
