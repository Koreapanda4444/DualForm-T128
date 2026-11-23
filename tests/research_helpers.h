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
static inline void df_prefix(const uint64_t sk[24],const uint8_t in[16],uint8_t out[16],unsigned q,int inverse) {
    uint64_t l=df_load64(in),r=df_load64(in+8);
    for(unsigned j=0;j<q;j++) {
        unsigned i=inverse?q-1-j:j;
        if(inverse) { uint64_t x=r^((i&1)?df_odd(l^sk[i]):df_even(l^sk[i]));r=l;l=x; }
        else { uint64_t x=l^((i&1)?df_odd(r^sk[i]):df_even(r^sk[i]));l=r;r=x; }
    }
    df_store64(out,l);df_store64(out+8,r);
}
#endif
