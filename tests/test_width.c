#include <stdint.h>
#include <limits.h>
#include <openssl/opensslv.h>
#ifndef UINTPTR_MAX
#error "DualForm requires uintptr_t on a flat-address platform"
#endif
_Static_assert(CHAR_BIT == 8, "8-bit bytes required");
_Static_assert(sizeof(uint64_t) == 8 && sizeof(uint32_t) == 4, "integer widths");
int main(void) { return OPENSSL_VERSION_MAJOR < 3; }
