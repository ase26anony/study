/* Compile with: gcc -O2 -flto -fuse-linker-plugin -fno-semantic-interposition target_func.c main.c -o test */

/* Force generation of multi-versioned function with specific attributes */
#ifdef __cplusplus
extern "C" {
#endif

/* This function will trigger the creation of artificial resolver/helper functions */
__attribute__((target_clones("default", "avx2", "avx512f"), 
               noinline, 
               noclone,
               used,
               visibility("hidden")))
int compute_with_features(int x, int y) {
    /* Use operations that might be optimized differently per target */
    volatile int result = 0;
    
    /* Prevent constant folding with volatile */
    result = x * y;
    
    /* Use __builtin_cpu_supports to potentially generate helper functions */
    if (__builtin_cpu_supports("avx2")) {
        result += 1;
    }
    
    /* Atomic operation that might need runtime support */
    int atomic_var = 0;
    __atomic_compare_exchange_n(&atomic_var, &result, x, 0, 
                                __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    return result;
}

/* Another approach: Function with explicit target attribute */
__attribute__((target("arch=x86-64-v3"), 
               noinline,
               used,
               visibility("hidden")))
static int specialized_compute(int x) {
    /* Artificial function that compiler might clone */
    return x * 2 + __builtin_popcount(x);
}

/* External declaration to force different linkage */
extern int specialized_compute(int x);

#ifdef __cplusplus
}
#endif
