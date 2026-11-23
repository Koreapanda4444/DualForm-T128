#ifndef DF_RESEARCH_HELPERS_H
#define DF_RESEARCH_HELPERS_H
#include "dualform_internal.h"
#include "shake256.h"
static inline int df_decode_tweak(const uint8_t key[32],const uint64_t enc[2],uint8_t out[16]) {
    uint64_t l=enc[0],r=enc[1]; uint8_t b[8],h[8];
    static const char domain[]="DualForm-T128/tweak-preperm";
    for(int j=5;j>=0;j--) {
        uint8_t i=(uint8_t)j;df_store64(b,l);
        df_span s[]={{domain,sizeof(domain)},{&i,1},{key,32},{b,8}};
        if(!df_shake(s,4,h,8)) return 0;
        uint64_t prev=r^df_load64(h);r=l;l=prev;
    }
    df_store64(out,l);df_store64(out+8,r);return 1;
}
#endif
