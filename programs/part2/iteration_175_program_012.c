#ifndef HEADER_H
#define HEADER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Inline function that uses 128-bit operations
// Will be included in multiple translation units
static inline __int128 perform_128bit_division(__int128 a, __int128 b) {
    // Complex 128-bit operation that likely requires helper functions
    return a / b;
}

// Function with nothrow attribute
void atomic_128bit_update(__int128 *ptr, __int128 val) __attribute__((nothrow));

// Function using volatile 128-bit
volatile __int128 process_volatile_128bit(volatile __int128 *v);

#ifdef __cplusplus
}
#endif

#endif // HEADER_H
