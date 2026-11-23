#include "shake256.h"
#include "df_memory.h"
EVP_MD_CTX *df_shake_new(void) {
    if(!df_test_gate()) return NULL;
    return EVP_MD_CTX_new();
}
void df_shake_free(EVP_MD_CTX *ctx) { EVP_MD_CTX_free(ctx); }
int df_shake_with(EVP_MD_CTX *ctx,const df_span *parts,size_t count,uint8_t *out,size_t len) {
#ifdef DF_TEST_HOOKS
    df_test_shake_calls++;
#endif
    if(!ctx || !df_test_gate() || EVP_DigestInit_ex(ctx,EVP_shake256(),NULL)!=1) return 0;
    for(size_t i=0;i<count;i++) {
        if(parts[i].len && (!parts[i].data || !df_test_gate() || EVP_DigestUpdate(ctx,parts[i].data,parts[i].len)!=1)) return 0;
    }
    return df_test_gate() && EVP_DigestFinalXOF(ctx,out,len)==1;
}
int df_shake(const df_span *parts,size_t count,uint8_t *out,size_t len) {
    EVP_MD_CTX *ctx=df_shake_new();if(!ctx) return 0;
    int ok=df_shake_with(ctx,parts,count,out,len);df_shake_free(ctx);return ok;
}
