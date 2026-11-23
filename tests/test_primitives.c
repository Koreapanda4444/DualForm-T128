#include "dualform_internal.h"
#include <assert.h>
int main(void) {
    uint8_t b[10]={0}; unsigned ns[]={0,1,31,32,63,64};
    df_store64(b+1,UINT64_C(0x8070605040302010));
    assert(df_load64(b+1)==UINT64_C(0x8070605040302010));
    assert(b[1]==0x10 && b[8]==0x80 && b[0]==0 && b[9]==0);
    df_store32(b+1,UINT32_C(0xfedcba98)); assert(df_load32(b+1)==UINT32_C(0xfedcba98));
    for(unsigned i=0;i<6;i++) {
        uint64_t x=UINT64_C(0x8123456789abcdef);
        assert(df_rol64(df_rol64(x,ns[i]),64-(ns[i]&63))==x);
        assert(df_rol32(df_rol32((uint32_t)x,ns[i]),32-(ns[i]&31))==(uint32_t)x);
    }
    return 0;
}
