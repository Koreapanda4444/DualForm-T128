#include "workload.h"
#include "timer.h"
#include "openssl_compare.h"
#include <openssl/crypto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
int main(int argc,char **argv) {
    if(argc!=3) return 2;
    int aes=!strcmp(argv[1],"aes256_prepared") || !strcmp(argv[1],"aes256_key_block");
    unsigned mode=6;
    for(unsigned i=0;i<6;i++) if(!strcmp(argv[1],df_modes[i])) mode=i;
    char *end=NULL;unsigned long long raw=strtoull(argv[2],&end,10);
    if((mode==6 && !aes) || !end || *end || !raw || raw>50000000) return 2;
    size_t n=(size_t)raw;
    if(aes)return df_aes_sample(argv[1],n,!strcmp(argv[1],"aes256_key_block"));
    uint8_t *keys=NULL,*tweaks=NULL;
    if(mode==1 || mode==4) { keys=calloc(n,32);if(!keys)return 3; }
    if(mode>=2 && mode<=4) { tweaks=calloc(n,16);if(!tweaks){free(keys);return 3;} }
    for(size_t i=0;i<n;i++) for(unsigned b=0;b<8;b++) {
        if(keys)keys[32*i+b]=(uint8_t)((uint64_t)i>>(8*b));
        if(tweaks)tweaks[16*i+b]=(uint8_t)((uint64_t)i>>(8*b));
    }
    uint8_t zero[32]={0},input[16]={0},output[16]={0};uint64_t checksum=0;
    dualform_key_ctx *base=NULL;dualform_tweak_ctx *prepared=NULL;
    if(dualform_key_ctx_create(&base,zero,32) || dualform_tweak_ctx_create(&prepared,base,zero,16)) goto fail;
    if(mode==5 && dualform_encrypt_block(prepared,zero,16,input,16)) goto fail;
    for(size_t i=0;i<(n<128?n:128);i++)
        if(df_bench_op(mode,base,prepared,keys?keys+32*i:zero,tweaks?tweaks+16*i:zero,input,output,&checksum)) goto fail;
    double start=df_now();if(start<0) goto fail;
    for(size_t i=0;i<n;i++)
        if(df_bench_op(mode,base,prepared,keys?keys+32*i:zero,tweaks?tweaks+16*i:zero,input,output,&checksum)) goto fail;
    double elapsed=df_now()-start;
    if(!(elapsed>0) || !isfinite(elapsed)) goto fail;
    printf("{\"mode\":\"%s\",\"iterations\":%zu,\"seconds\":%.9f,\"ns_per_op\":%.6f,\"ops_per_second\":%.6f,\"checksum\":%llu,\"clock\":\"%s\",\"openssl_runtime\":\"%s\"",df_modes[mode],n,elapsed,elapsed*1e9/(double)n,(double)n/elapsed,(unsigned long long)checksum,DF_CLOCK,OpenSSL_version(OPENSSL_VERSION));
    if(mode==0 || mode>=3) printf(",\"mib_per_second\":%.6f",(double)n*16.0/1048576.0/elapsed);
    printf("}\n");dualform_tweak_ctx_free(prepared);dualform_key_ctx_free(base);free(keys);free(tweaks);return 0;
fail:
    dualform_tweak_ctx_free(prepared);dualform_key_ctx_free(base);free(keys);free(tweaks);return 3;
}
