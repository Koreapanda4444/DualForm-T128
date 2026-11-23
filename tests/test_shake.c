#include "shake256.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
int main(void) {
    const char *hex="46b9dd2b0ba88d13233b3feb743eeb243fcd52ea62b81b82b50c27646ed5762fd75dc4ddd8c0f200cb05019d67b592f6fc821c49479ab48640292eacb3b7c4be";
    uint8_t a[192],b[192];
    assert(df_shake(NULL,0,a,64));
    for(unsigned i=0;i<64;i++) { unsigned v; assert(sscanf(hex+2*i,"%2x",&v)==1); assert(a[i]==v); }
    assert(df_shake(NULL,0,b,8)); assert(!memcmp(a,b,8));
    df_span parts[]={{"ab",2},{"c",1}}, one={"abc",3};
    assert(df_shake(parts,2,a,192) && df_shake(&one,1,b,192));
    assert(!memcmp(a,b,192)); return 0;
}
