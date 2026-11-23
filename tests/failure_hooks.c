#include "df_memory.h"
#include <assert.h>
long df_test_fail_after=-1;
size_t df_test_live=0,df_test_calls=0,df_test_shake_calls=0;
int df_test_gate(void) {
    df_test_calls++;
    if(df_test_fail_after==0) return 0;
    if(df_test_fail_after>0) df_test_fail_after--;
    return 1;
}
void *df_alloc(size_t n) {
    if(!df_test_gate()) return NULL;
    void *p=calloc(1,n);if(p) df_test_live++;return p;
}
void df_release(void *p,size_t n) {
    const unsigned char *b=p;
    if(p) {for(size_t i=0;i<n;i++) assert(b[i]==0);assert(df_test_live>0);df_test_live--;free(p);}
}
