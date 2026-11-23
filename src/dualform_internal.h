#ifndef DUALFORM_INTERNAL_H
#define DUALFORM_INTERNAL_H
#include <stdint.h>
#include <stddef.h>
#ifndef UINTPTR_MAX
#error "DualForm requires uintptr_t"
#endif
static inline uint32_t df_rol32(uint32_t x, unsigned n) {
    n &= 31u; return (x << n) | (x >> ((32u-n)&31u));
}
static inline uint64_t df_rol64(uint64_t x, unsigned n) {
    n &= 63u; return (x << n) | (x >> ((64u-n)&63u));
}
static inline uint64_t df_load64(const uint8_t *p) {
    uint64_t x=0; for(unsigned i=0;i<8;i++) x |= (uint64_t)p[i]<<(8*i); return x;
}
static inline uint32_t df_load32(const uint8_t *p) {
    uint32_t x=0; for(unsigned i=0;i<4;i++) x |= (uint32_t)p[i]<<(8*i); return x;
}
static inline void df_store64(uint8_t *p,uint64_t x) {
    for(unsigned i=0;i<8;i++) p[i]=(uint8_t)(x>>(8*i));
}
static inline void df_store32(uint8_t *p,uint32_t x) {
    for(unsigned i=0;i<4;i++) p[i]=(uint8_t)(x>>(8*i));
}
#endif
