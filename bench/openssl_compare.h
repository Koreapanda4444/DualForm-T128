#ifndef DF_OPENSSL_COMPARE_H
#define DF_OPENSSL_COMPARE_H
#include "timer.h"
#include <openssl/evp.h>
#include <openssl/crypto.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
static int df_aes_sample(const char *name,size_t n,int fresh) {
    uint8_t zero[32]={0},out[32];uint8_t *keys=fresh?calloc(n,32):NULL;
    if(fresh && !keys)return 3;
    if(keys) for(size_t i=0;i<n;i++) for(unsigned b=0;b<8;b++) keys[32*i+b]=(uint8_t)((uint64_t)i>>(8*b));
    EVP_CIPHER_CTX *prepared=EVP_CIPHER_CTX_new();
    if(!prepared) {free(keys);return 3;}
    if(EVP_EncryptInit_ex(prepared,EVP_aes_256_ecb(),NULL,zero,NULL)!=1 || EVP_CIPHER_CTX_set_padding(prepared,0)!=1) goto fail;
    uint64_t checksum=0;double start=0;
    for(unsigned pass=0;pass<2;pass++) {
        size_t count=pass?n:(n<128?n:128);
        if(pass){start=df_now();if(start<0)goto fail;}
        for(size_t i=0;i<count;i++) {
            EVP_CIPHER_CTX *ctx=prepared;
            if(fresh) {
                ctx=EVP_CIPHER_CTX_new();if(!ctx)goto fail;
                if(EVP_EncryptInit_ex(ctx,EVP_aes_256_ecb(),NULL,keys+32*i,NULL)!=1 || EVP_CIPHER_CTX_set_padding(ctx,0)!=1) {EVP_CIPHER_CTX_free(ctx);goto fail;}
            }
            int len=0;
            int ok=EVP_EncryptUpdate(ctx,out,&len,zero,16)==1 && len==16;
            if(ok && fresh) {int tail=0;ok=EVP_EncryptFinal_ex(ctx,out+16,&tail)==1 && tail==0;}
            if(fresh)EVP_CIPHER_CTX_free(ctx);
            if(!ok)goto fail;
            checksum+=out[0];
        }
    }
    double sec=df_now()-start;if(!(sec>0) || !isfinite(sec))goto fail;
    printf("{\"mode\":\"%s\",\"iterations\":%zu,\"seconds\":%.9f,\"ns_per_op\":%.6f,\"ops_per_second\":%.6f,\"mib_per_second\":%.6f,\"checksum\":%llu,\"clock\":\"%s\",\"openssl_runtime\":\"%s\",\"boundary\":\"%s\"}\n",name,n,sec,sec*1e9/n,n/sec,n*16.0/1048576/sec,(unsigned long long)checksum,DF_CLOCK,OpenSSL_version(OPENSSL_VERSION),fresh?"EVP context create, AES-256 init, no padding, one Update, Final, context free":"reused AES-256 ECB context; one 16-byte EVP Update; no tweak or authentication");
    EVP_CIPHER_CTX_free(prepared);free(keys);return 0;
fail:
    EVP_CIPHER_CTX_free(prepared);free(keys);return 3;
}
#endif
