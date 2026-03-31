/* Compile with: gcc -O2 -flto -fuse-linker-plugin -c target_func.c */
#include <stdint.h>

/* Force generation of multi-versioned function with specific attributes */
__attribute__((target_clones("default", "avx2", "avx512f"), 
               noinline, 
               noclone,
               used,
               visibility("hidden")))
static uint64_t multi_version_func(uint64_t x, uint64_t y) {
    /* Use operations that benefit from different instruction sets */
    uint64_t result = x;
    
    /* Mix of operations that could use different SIMD extensions */
    for (int i = 0; i < 64; i++) {
        result = (result * 1103515245 + 12345) ^ y;
        y = (y * 1664525 + 1013904223) ^ result;
    }
    
    return result ^ (x + y);
}

/* Force generation of atomic helper with large type */
typedef struct { uint64_t data[8]; } large_atomic_t;

__attribute__((noinline, target("default")))
static uint64_t atomic_helper(void) {
    large_atomic_t var = {0};
    large_atomic_t expected = {0};
    large_atomic_t desired = {{1, 2, 3, 4, 5, 6, 7, 8}};
    
    /* This may generate internal helper functions */
    __atomic_compare_exchange(&var, &expected, &desired, 
                              0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    return var.data[0];
}

/* Function with explicit target attribute that might need a clone */
__attribute__((target("arch=core-avx2"), noinline, used))
uint64_t avx2_specific(uint64_t x) {
    /* Use __builtin_cpu_supports to potentially generate runtime dispatch */
    if (__builtin_cpu_supports("avx2")) {
        return x * 3 + 7;
    }
    return x * 2 + 5;
}

/* Export the functions */
uint64_t get_multi_version(uint64_t x, uint64_t y) {
    return multi_version_func(x, y);
}

uint64_t get_atomic_result(void) {
    return atomic_helper();
}

uint64_t get_avx2_result(uint64_t x) {
    return avx2_specific(x);
}
