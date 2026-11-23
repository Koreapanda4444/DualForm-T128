#ifndef DUALFORM_H
#define DUALFORM_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define DUALFORM_KEY_BYTES 32u
#define DUALFORM_TWEAK_BYTES 16u
#define DUALFORM_BLOCK_BYTES 16u
typedef struct dualform_key_ctx dualform_key_ctx;
typedef struct dualform_tweak_ctx dualform_tweak_ctx;
typedef enum {
 DUALFORM_OK=0, DUALFORM_EINVAL=1, DUALFORM_ELENGTH=2, DUALFORM_ECAPACITY=3,
 DUALFORM_EOVERLAP=4, DUALFORM_ENOMEM=5, DUALFORM_ECRYPTO=6
} dualform_status;
dualform_status dualform_key_ctx_create(dualform_key_ctx **out_ctx,const uint8_t *key,size_t key_len);
void dualform_key_ctx_free(dualform_key_ctx *ctx);
#ifdef __cplusplus
}
#endif
#endif
