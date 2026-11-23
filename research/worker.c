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
    if(argc!=8 || strcmp(argv[1],"bias")) return 2;
    unsigned j=(unsigned)strtoul(argv[2],NULL,10),r=(unsigned)strtoul(argv[3],NULL,10),n=(unsigned)strtoul(argv[4],NULL,10);
    unsigned v=(unsigned)strtoul(argv[5],NULL,10);double seconds=strtod(argv[6],NULL);
    if(j>=8 || r>24 || n>32768 || v || seconds<0 || strcmp(argv[7],"v1")) return 2;
    signal(SIGTERM,interrupt);signal(SIGINT,interrupt);
    double start=df_now();if(start<0) return 3;
    uint8_t k[32],t[16],p[16],p2[16],c[16],c2[16];
    if(!sample("bias-key",j,0,0,k,32) || !sample("bias-tweak",j,0,0,t,16)) return 3;
    dualform_key_ctx *key=NULL;dualform_tweak_ctx *ctx=NULL;
    if(dualform_key_ctx_create(&key,k,32) || dualform_tweak_ctx_create(&ctx,key,t,16)) {dualform_key_ctx_free(key);return 3;}
    unsigned count=0,executed=0;
    for(unsigned i=0;i<n;i++) {
        if((i&255u)==0 && (stop || df_now()-start>=seconds)) break;
        if(!sample("bias-plain",j,r,i,p,16)) return 3;
        memcpy(p2,p,16);p2[3]^=0x80;
        df_prefix(ctx->sk,p,c,r,0);df_prefix(ctx->sk,p2,c2,r,0);
        count+=((c[0]^c2[0])>>1)&1u;executed++;
    }
    dualform_tweak_ctx_free(ctx);dualform_key_ctx_free(key);
    printf("{\"requested\":%u,\"executed\":%u,\"counts\":[%u],\"rounds\":%u,\"key_index\":%u,\"elapsed_seconds\":%.9f,\"status\":\"%s\"}\n",n,executed,count,r,j,df_now()-start,executed==n?"completed":"incomplete");return 0;
}
