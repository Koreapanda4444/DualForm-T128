#include "dualform_internal.h"
#include <assert.h>
static uint64_t even_reference(uint64_t x) {
    unsigned s[]={12,5,6,11,9,0,10,13,3,14,15,8,4,7,1,2}; uint64_t y=0;
    for(unsigned j=0;j<16;j++) y|=(uint64_t)s[(x>>(4*j))&15]<<(4*j);
    return y^df_rol64(y,11)^df_rol64(y,37);
}
static uint64_t odd_reference(uint64_t x) {
    uint64_t a=x&UINT32_MAX,b=x>>32;
    a=(a+df_rol32((uint32_t)b,7))&UINT32_MAX; b^=df_rol32((uint32_t)a,11);
    a^=df_rol32((uint32_t)b,17); b=(b+df_rol32((uint32_t)a,5))&UINT32_MAX;
    return a|(b<<32);
}
int main(void) {
    uint64_t v[]={0,1,UINT64_MAX,UINT32_MAX,UINT64_C(0x8000000080000000),UINT64_C(0xffffffff00000001)};
    for(unsigned i=0;i<sizeof(v)/sizeof(*v);i++) {
        assert(df_even(v[i])==even_reference(v[i])); assert(df_odd(v[i])==odd_reference(v[i]));
    }
    return 0;
}
