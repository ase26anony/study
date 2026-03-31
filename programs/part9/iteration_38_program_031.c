/* Compile with: gcc -O2 -flto -fuse-linker-plugin -c target_func.c */
#include <stdint.h>

/* Force generation of multi-versioned function with hidden resolver */
__attribute__((target_clones("default", "avx2", "avx512f"),
               noinline,
               noclone,
               used,
               visibility("hidden")))
static uint64_t multi_version_func(uint64_t a, uint64_t b) {
    /* Complex enough to not be optimized away, simple enough to compile */
    uint64_t result = a;
    
    /* Use operations that might benefit from different ISA extensions */
    for (int i = 0; i < 64; i++) {
        if (b & (1ULL << i)) {
            result ^= (result << i) | (result >> (64 - i));
        }
    }
    
    /* Use atomic operation that might need helper */
    __atomic_store_n(&result, result ^ b, __ATOMIC_RELAXED);
    
    return result;
}

/* Function that will be called from main */
__attribute__((noinline))
uint64_t public_entry_point(uint64_t a, uint64_t b) {
    /* Call the multi-versioned function */
    return multi_version_func(a, b);
}
