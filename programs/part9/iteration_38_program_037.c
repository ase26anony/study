/* Compile with: gcc -O2 -flto -fuse-linker-plugin -c target_func.c */
/* This file contains the function with target attributes */

/* Force the function to be emitted with hidden visibility and used */
__attribute__((target_clones("default", "avx2", "avx512f"), 
               noinline, 
               used, 
               visibility("hidden")))
int multi_version_func(int x, int y) {
    /* Simple computation that can't be constant-folded */
    volatile int seed = x;
    int result = seed + y;
    
    /* Add some computation that might benefit from vectorization */
    for (int i = 0; i < 100; i++) {
        result = (result * 1103515245 + 12345) & 0x7fffffff;
    }
    
    return result;
}

/* Another approach: function with explicit target attribute */
__attribute__((target("avx2"), noinline, used, visibility("hidden")))
static int avx2_specific_func(int x) {
    /* Use __builtin_cpu_supports to potentially trigger helper generation */
    if (__builtin_cpu_supports("avx2")) {
        return x * 3;
    }
    return x * 2;
}

/* Force generation of atomic helper - might create artificial functions */
long atomic_helper(void) {
    long large_var = 0;
    long expected = 0;
    long desired = 1;
    
    /* This may generate internal helper functions for atomic operations */
    __atomic_compare_exchange(&large_var, &expected, &desired, 
                              0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    return large_var;
}
