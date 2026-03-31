#ifndef HEADER_WITH_HELPERS_H
#define HEADER_WITH_HELPERS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Inline function that uses 128-bit operations - will be included in multiple TUs */
static inline unsigned __int128 perform_128bit_division(unsigned __int128 a, unsigned __int128 b) {
    /* Force generation of __umodti3 helper */
    return a % b;
}

/* Function with nothrow attribute */
int process_value(unsigned __int128 val) __attribute__((nothrow));

/* Atomic operation helper */
void atomic_update(unsigned __int128 *ptr, unsigned __int128 val);

#ifdef __cplusplus
}
#endif

#endif /* HEADER_WITH_HELPERS_H */
