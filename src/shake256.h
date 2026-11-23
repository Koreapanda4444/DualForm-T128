#ifndef DF_SHAKE256_H
#define DF_SHAKE256_H
#include <stddef.h>
#include <stdint.h>
#include <openssl/evp.h>
typedef struct { const void *data; size_t len; } df_span;
EVP_MD_CTX *df_shake_new(void);
void df_shake_free(EVP_MD_CTX *ctx);
int df_shake_with(EVP_MD_CTX *ctx,const df_span *parts,size_t count,uint8_t *out,size_t len);
int df_shake(const df_span *parts,size_t count,uint8_t *out,size_t len);
#endif
