/* Function designed to trigger target hook for multi-versioning */
#include <stdint.h>

/* 
 * This function uses target_clones to create multiple versions.
 * The compiler will generate artificial resolver functions with
 * the flags: ARTIFICIAL, IGNORED_P, VISIBILITY_HIDDEN, etc.
 */
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
__attribute__((noinline, used, visibility("hidden")))
static uint64_t target_specific_compute(uint64_t a, uint64_t b) {
    /* Complex enough to not be optimized away, but simple for demonstration */
    uint64_t result = a ^ b;
    
    /* Use some operations that might benefit from different ISAs */
    for (int i = 0; i < 64; i++) {
        if (b & (1ULL << i)) {
            result += (a << i);
        } else {
            result -= (a >> i);
        }
    }
    
    /* Mix in some arithmetic */
    result = (result * 0x5DEECE66DULL) + 0xB;
    result = (result >> 32) | (result << 32);  /* Rotate */
    
    return result;
}

/* 
 * Another approach: Use atomic operations that might generate helpers
 * This could trigger creation of artificial built-in functions
 */
__attribute__((target("arch=core-avx2")))
__attribute__((noinline))
uint64_t atomic_helper(uint64_t *ptr, uint64_t val) {
    /* __atomic_compare_exchange might generate helper functions */
    uint64_t expected = *ptr;
    uint64_t desired = val;
    
    if (__atomic_compare_exchange_n(ptr, &expected, desired, 
                                    0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
        return expected;
    }
    return desired;
}

/* Export the function for main.c */
uint64_t compute_with_target(uint64_t a, uint64_t b) {
    return target_specific_compute(a, b);
}
