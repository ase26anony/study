/* Header file to be included in multiple translation units */
#ifndef TRIGGER_HELPERS_H
#define TRIGGER_HELPERS_H

#include <stdint.h>

/* Inline function that uses 128-bit operations */
static inline unsigned __int128 trigger_128bit_ops(unsigned __int128 a, 
                                                   unsigned __int128 b) 
    __attribute__((nothrow));

static inline unsigned __int128 trigger_128bit_ops(unsigned __int128 a, 
                                                   unsigned __int128 b) {
    /* 128-bit division - may require __udivti3 helper */
    unsigned __int128 div_result = a / b;
    
    /* 128-bit modulo - may require __umodti3 helper */
    unsigned __int128 mod_result = a % b;
    
    /* Mix operations */
    return div_result + mod_result;
}

/* Function with atomic operations on 128-bit */
static void atomic_128bit_op(unsigned __int128 *ptr) 
    __attribute__((nothrow));

static void atomic_128bit_op(unsigned __int128 *ptr) {
    unsigned __int128 expected = *ptr;
    unsigned __int128 desired = expected + 1;
    
    /* Atomic compare-exchange on 128-bit */
    __atomic_compare_exchange_n(ptr, &expected, desired, 
                                0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

#endif /* TRIGGER_HELPERS_H */
