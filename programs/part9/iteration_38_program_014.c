/* Compile with: gcc -O2 -flto -fuse-linker-plugin -c target_func.c */
#include <stdint.h>

/* This function uses target_clones to force multi-versioning */
__attribute__((target_clones("default", "avx2", "avx512f"), 
               noinline, 
               used, 
               visibility("hidden"),
               noipa))
int target_multiversion(int x, int y) {
    /* Complex enough to not be optimized away, but simple for demo */
    volatile int result = 0;
    
    /* Force computation that might benefit from vectorization */
    for (int i = 0; i < 16; i++) {
        result += x * y + i;
    }
    
    /* Use atomic operation that might need helper functions */
    __atomic_add_fetch(&result, 1, __ATOMIC_SEQ_CST);
    
    return result;
}

/* Another approach: function with explicit target attribute */
__attribute__((target("avx512f"), 
               noinline,
               used,
               visibility("hidden")))
static int avx512_specific(int x) {
    /* Use __builtin_cpu_supports to potentially trigger helper generation */
    if (__builtin_cpu_supports("avx512f")) {
        return x * 2;
    }
    return x;
}

/* Force generation of atomic helper with large type */
__attribute__((noinline, used))
int64_t atomic_large_op(int64_t *ptr, int64_t val) {
    int64_t expected = *ptr;
    int64_t desired = val;
    
    /* Large atomic compare-exchange might need helper */
    __atomic_compare_exchange(ptr, &expected, &desired, 
                              0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    return expected;
}
