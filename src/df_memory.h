#ifndef DF_MEMORY_H
#define DF_MEMORY_H
#include <stdlib.h>
#ifdef DF_TEST_HOOKS
extern long df_test_fail_after;
extern size_t df_test_live,df_test_calls,df_test_shake_calls;
int df_test_gate(void);
void *df_alloc(size_t n);
void df_release(void *p,size_t n);
#else
static inline int df_test_gate(void) { return 1; }
static inline void *df_alloc(size_t n) { return calloc(1,n); }
static inline void df_release(void *p,size_t n) { (void)n; free(p); }
#endif
#endif
