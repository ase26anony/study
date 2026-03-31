#ifndef TEST_TARGHOOKS_H
#define TEST_TARGHOOKS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Force generation of 128-bit helpers */
static inline unsigned __int128 helper_div128(unsigned __int128 a, unsigned __int128 b) {
    return a / b;  /* Will likely generate __udivti3 call */
}

static inline __int128 helper_mul128(__int128 a, __int128 b) {
    return a * b;  /* Will likely generate __multi3 call */
}

/* Atomic operation on 128-bit */
static inline void atomic_update(__int128 *ptr, __int128 val) {
    __int128 expected = *ptr;
    __int128 desired;
    do {
        desired = expected + val;
    } while (!__atomic_compare_exchange_n(ptr, &expected, desired, 
                                          0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST));
}

/* Nothrow function attribute */
__attribute__((nothrow))
static void process_volatile_128(volatile __int128 *v) {
    /* Access volatile 128-bit */
    __int128 temp = *v;
    *v = temp + 1;
}

#ifdef __cplusplus
}
#endif

#endif /* TEST_TARGHOOKS_H */
