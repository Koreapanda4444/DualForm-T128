#include "shake256.h"
#include "df_memory.h"
int df_shake(const df_span *parts,size_t count,uint8_t *out,size_t len) {
    int ok=0;
#ifdef DF_TEST_HOOKS
    df_test_shake_calls++;
#endif
    if(!df_test_gate()) return 0;
    EVP_MD_CTX *ctx=EVP_MD_CTX_new();
    if(!ctx) return 0;
    if(!df_test_gate() || EVP_DigestInit_ex(ctx,EVP_shake256(),NULL)!=1) goto done;
    for(size_t i=0;i<count;i++) {
        if(parts[i].len && (!parts[i].data || !df_test_gate() || EVP_DigestUpdate(ctx,parts[i].data,parts[i].len)!=1)) goto done;
    }
    if(!df_test_gate() || EVP_DigestFinalXOF(ctx,out,len)!=1) goto done;
    ok=1;
done:
    EVP_MD_CTX_free(ctx);
    return ok;
}
