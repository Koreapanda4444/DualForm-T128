#include "workload.h"
#include "df_memory.h"
#include <assert.h>
int main(void) {
    uint8_t zero[32]={0},out[16]={0};dualform_key_ctx *key=NULL;dualform_tweak_ctx *ctx=NULL;uint64_t checksum=0;
    assert(dualform_key_ctx_create(&key,zero,32)==0 && dualform_tweak_ctx_create(&ctx,key,zero,16)==0);
    const unsigned calls[]={0,1,7,7,8,0};
    for(unsigned mode=0;mode<6;mode++) {
        df_test_shake_calls=0;
        for(unsigned i=0;i<3;i++) assert(df_bench_op(mode,key,ctx,zero,zero,zero,out,&checksum)==0);
        assert(df_test_shake_calls==3*calls[mode]);assert(df_test_live==2);
    }
    dualform_tweak_ctx_free(ctx);dualform_key_ctx_free(key);assert(df_test_live==0);return 0;
}
