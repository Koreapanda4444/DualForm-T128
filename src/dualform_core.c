#include "dualform_internal.h"
/* Bit planes process all sixteen nibbles in parallel. ANF terms use fixed addresses. */
uint64_t df_substitute(uint64_t x) {
    const uint64_t mask=UINT64_C(0x1111111111111111);
    uint64_t a=x&mask,b=(x>>1)&mask,c=(x>>2)&mask,d=(x>>3)&mask;
    uint64_t y0=a^c^(b&c)^d;
    uint64_t y1=b^(a&b&c)^d^(b&d)^(a&b&d)^(c&d)^(a&c&d);
    uint64_t y2=mask^(a&b)^c^d^(a&d)^(b&d)^(a&b&d)^(a&c&d);
    uint64_t y3=mask^a^b^(b&c)^(a&b&c)^d^(a&b&d)^(a&c&d);
    return y0|(y1<<1)|(y2<<2)|(y3<<3);
}
uint64_t df_even(uint64_t x) {
    uint64_t y=df_substitute(x); return y^df_rol64(y,11)^df_rol64(y,37);
}
uint64_t df_odd(uint64_t x) {
    uint32_t a=(uint32_t)x,b=(uint32_t)(x>>32);
    a+=df_rol32(b,7); b^=df_rol32(a,11); a^=df_rol32(b,17); b+=df_rol32(a,5);
    return (uint64_t)a|((uint64_t)b<<32);
}
#include "shake256.h"
#include <openssl/crypto.h>
int df_encode_tweak(const uint8_t key[32],const uint8_t tweak[16],uint64_t enc[2]) {
    uint64_t l=df_load64(tweak),r=df_load64(tweak+8);
    static const char domain[]="DualForm-T128/tweak-preperm";
    uint8_t block[8],digest[8];
    for(uint8_t i=0;i<6;i++) {
        df_store64(block,r); df_span spans[]={{domain,sizeof(domain)},{&i,1},{key,32},{block,8}};
        if(!df_shake(spans,4,digest,8)) {
            OPENSSL_cleanse(block,8);OPENSSL_cleanse(digest,8);return 0;
        }
        uint64_t next=l^df_load64(digest);l=r;r=next;
    }
    enc[0]=l;enc[1]=r;OPENSSL_cleanse(block,8);OPENSSL_cleanse(digest,8);return 1;
}
