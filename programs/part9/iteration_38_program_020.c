/* Force generation of artificial helper functions with specific attributes */
#include <stdint.h>
#include <string.h>

/* Option 1: Multi-versioned function with target clones */
__attribute__((target_clones("default", "avx2", "avx512f"), 
               noinline, 
               noclone,
               used,
               visibility("hidden")))
static uint64_t multi_version_func(uint64_t a, uint64_t b) {
    /* Complex enough to prevent constant folding */
    uint64_t result = a;
    for (int i = 0; i < 100; i++) {
        result = (result * 1103515245 + 12345) ^ b;
    }
    return result;
}

/* Option 2: Function with explicit target attribute that may need resolver */
__attribute__((target("arch=core-avx2"), 
               noinline,
               visibility("hidden")))
extern uint64_t avx2_specific_func(uint64_t a, uint64_t b);

/* The actual definition with different target */
__attribute__((target("arch=core-avx2"),
               noinline,
               visibility("hidden")))
uint64_t avx2_specific_func(uint64_t a, uint64_t b) {
    /* Use some operations that might benefit from AVX2 */
    uint64_t array[4] = {a, b, a ^ b, a + b};
    uint64_t sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += array[i];
    }
    return sum;
}

/* Option 3: Function using builtins that may generate helpers */
__attribute__((noinline, visibility("hidden")))
static uint64_t atomic_helper(uint64_t *ptr, uint64_t val) {
    /* __atomic builtins often generate internal helper calls */
    uint64_t expected = *ptr;
    uint64_t desired = val;
    __atomic_compare_exchange(ptr, &expected, &desired, 
                              0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return expected;
}

/* Public interface function */
__attribute__((visibility("default")))
uint64_t target_operations(uint64_t seed) {
    uint64_t a = seed;
    uint64_t b = seed * 3 + 1;
    
    /* Call multi-versioned function */
    uint64_t r1 = multi_version_func(a, b);
    
    /* Call AVX2-specific function */
    uint64_t r2 = avx2_specific_func(r1, b);
    
    /* Use atomic operations */
    static uint64_t counter = 0;
    uint64_t r3 = atomic_helper(&counter, r2);
    
    return r1 ^ r2 ^ r3;
}
