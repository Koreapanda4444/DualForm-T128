#ifndef DF_WORKLOAD_H
#define DF_WORKLOAD_H
#include "dualform.h"
static const char *const df_modes[]={"prepared_encrypt","key_setup","tweak_setup","tweak_and_block","key_tweak_and_block","prepared_decrypt"};
static inline dualform_status df_bench_op(unsigned mode,dualform_key_ctx *base,dualform_tweak_ctx *prepared,
 const uint8_t k[32],const uint8_t t[16],const uint8_t input[16],uint8_t output[16],uint64_t *checksum) {
    dualform_key_ctx *key=NULL;dualform_tweak_ctx *ctx=NULL;dualform_status st=DUALFORM_OK;
    if(mode==0) st=dualform_encrypt_block(prepared,input,16,output,16);
    else if(mode==5) st=dualform_decrypt_block(prepared,input,16,output,16);
    else {
        if(mode==1 || mode==4) st=dualform_key_ctx_create(&key,k,32);
        if(!st && mode!=1) st=dualform_tweak_ctx_create(&ctx,mode==4?key:base,t,16);
        if(!st && (mode==3 || mode==4)) st=dualform_encrypt_block(ctx,input,16,output,16);
        if(!st && (mode==1 || mode==2)) *checksum^=(uint64_t)(uintptr_t)(mode==1?(void*)key:(void*)ctx);
    }
    if(mode==0 || mode>=3) *checksum+=output[0];
    dualform_tweak_ctx_free(ctx);dualform_key_ctx_free(key);return st;
}
#endif
