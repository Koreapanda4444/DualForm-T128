#ifndef DF_SHAKE256_H
#define DF_SHAKE256_H
#include <stddef.h>
#include <stdint.h>
#include <openssl/evp.h>
typedef struct { const void *data; size_t len; } df_span;
int df_shake(const df_span *parts,size_t count,uint8_t *out,size_t len);
#endif
