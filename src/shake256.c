#include "shake256.h"
int df_shake(const df_span *parts,size_t count,uint8_t *out,size_t len) {
    int ok=0;
    EVP_MD_CTX *ctx=EVP_MD_CTX_new();
    if(!ctx) return 0;
    if(EVP_DigestInit_ex(ctx,EVP_shake256(),NULL)!=1) goto done;
    for(size_t i=0;i<count;i++) {
        if(parts[i].len && (!parts[i].data || EVP_DigestUpdate(ctx,parts[i].data,parts[i].len)!=1)) goto done;
    }
    if(EVP_DigestFinalXOF(ctx,out,len)!=1) goto done;
    ok=1;
done:
    EVP_MD_CTX_free(ctx);
    return ok;
}
