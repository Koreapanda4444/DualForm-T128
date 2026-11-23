#include "research_helpers.h"
#include <assert.h>
#include <string.h>
int main(void) {
    uint8_t k[32]={0},t[16]={0},o[16];uint64_t e[2];
    assert(df_encode_tweak(k,t,e)); assert(e[0]==UINT64_C(0x273e85a05277071f));
    assert(e[1]==UINT64_C(0xc7a02ac7c1bf73a3));
    assert(df_decode_tweak(k,e,o) && !memcmp(t,o,16));
    for(unsigned i=0;i<32;i++) k[i]=(uint8_t)i;
    for(unsigned i=0;i<16;i++) t[i]=(uint8_t)i;
    assert(df_encode_tweak(k,t,e));assert(e[0]==UINT64_C(0x5e844157d4ef0f5a));assert(e[1]==UINT64_C(0xf197f4431d0750f7));
    assert(df_decode_tweak(k,e,o) && !memcmp(t,o,16));return 0;
}
