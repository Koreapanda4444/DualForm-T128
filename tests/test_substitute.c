#include "dualform_internal.h"
#include <assert.h>
int main(void) {
    const unsigned s[16]={12,5,6,11,9,0,10,13,3,14,15,8,4,7,1,2};
    for(unsigned p=0;p<16;p++) for(unsigned n=0;n<16;n++) {
        uint64_t x=(uint64_t)n<<(4*p), expected=UINT64_C(0xcccccccccccccccc);
        expected=(expected&~(UINT64_C(15)<<(4*p)))|((uint64_t)s[n]<<(4*p));
        assert(df_substitute(x)==expected);
    }
    return 0;
}
