#ifndef TEST_TARGHOOKS_H
#define TEST_TARGHOOKS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Inline function that uses 128-bit division - will be needed in multiple TUs */
static inline unsigned __int128 compute_128bit_hash(unsigned __int128 a, 
                                                    unsigned __int128 b) 
    __attribute__((always_inline, nothrow));

static inline unsigned __int128 compute_128bit_hash(unsigned __int128 a, 
                                                    unsigned __int128 b) 
{
    /* Complex 128-bit operations that may require helper functions */
    unsigned __int128 result = a;
    
    /* Division/modulo operations often need runtime helpers */
    if (b != 0) {
        result = result / b;        /* May call __udivti3 */
        result = result % (b + 1);  /* May call __umodti3 */
    }
    
    /* Multiplication can also need helpers on some targets */
    result = result * 0x123456789ABCDEFULL;
    
    return result;
}

/* Function using atomic operations on 128-bit values */
unsigned __int128 atomic_128bit_op(unsigned __int128 *ptr, 
                                   unsigned __int128 val)
    __attribute__((nothrow));

#ifdef __cplusplus
}
#endif

#endif /* TEST_TARGHOOKS_H */
