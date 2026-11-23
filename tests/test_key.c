#include "dualform.h"
#include "dualform_internal.h"
#include "shake256.h"
#include <assert.h>
#include <string.h>
int main(void) {
    uint8_t key[32]={0},base[192]; dualform_key_ctx *ctx=NULL;
    assert(dualform_key_ctx_create(NULL,key,32)==DUALFORM_EINVAL);
    assert(dualform_key_ctx_create(&ctx,NULL,32)==DUALFORM_EINVAL);
    assert(dualform_key_ctx_create(&ctx,key,31)==DUALFORM_ELENGTH && !ctx);
    assert(dualform_key_ctx_create(&ctx,key,32)==DUALFORM_OK);
    dualform_key_ctx *saved=ctx;
    assert(dualform_key_ctx_create(&ctx,key,32)==DUALFORM_EINVAL && ctx==saved);
    df_span s[]={{"DualForm-T128/base",18},{key,32}};
    assert(df_shake(s,2,base,192));
    for(unsigned i=0;i<24;i++) assert(ctx->base[i]==df_load64(base+8*i));
    memset(key,255,32); assert(ctx->key[0]==0);
    dualform_key_ctx_free(ctx);dualform_key_ctx_free(NULL);return 0;
}
