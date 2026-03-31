/* Compile with: gcc -O2 -flto -fuse-linker-plugin -c target_func.c */
#include <stdint.h>

/* Force generation of multi-versioned function with internal helper */
__attribute__((target_clones("default", "avx2", "avx512f"), 
               noinline, 
               noclone,
               used,
               visibility("hidden")))
static uint64_t multi_version_func(uint64_t a, uint64_t b) {
    /* Use operations that benefit from different ISA extensions */
    uint64_t result = a;
    
    /* Mix of operations that could use different instructions */
    for (int i = 0; i < 64; i++) {
        if (b & (1ULL << i)) {
            result ^= (result << i) | (result >> (64 - i));
        }
    }
    
    /* Prevent constant folding */
    asm volatile("" : "+r"(result));
    return result;
}

/* Force generation of atomic helper - may create artificial function */
__attribute__((noinline, target("arch=x86-64-v3")))
static uint64_t atomic_helper(uint64_t *ptr) {
    /* __atomic builtin may generate internal helpers */
    uint64_t expected = *ptr;
    uint64_t desired = expected + 1;
    
    while (!__atomic_compare_exchange_n(ptr, &expected, desired, 
                                        0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
        desired = expected + 1;
    }
    
    return desired;
}

/* Public interface that uses both functions */
__attribute__((visibility("default"), noinline))
uint64_t public_entry_point(uint64_t seed) {
    uint64_t a = seed;
    uint64_t b = seed ^ 0xDEADBEEF;
    
    uint64_t r1 = multi_version_func(a, b);
    uint64_t r2 = atomic_helper(&a);
    
    return r1 ^ r2;
}
