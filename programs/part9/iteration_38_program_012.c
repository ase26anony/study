/* Compile with: gcc -O2 -flto -fuse-linker-plugin target_func.c main.c -o test */

/* Force generation of multi-versioned function with internal resolver */
__attribute__((target_clones("default", "avx2", "avx512f")))
__attribute__((noinline, used, visibility("hidden")))
int multi_version_func(int x, int y) {
    /* Complex enough to not be optimized away, but simple for example */
    volatile int result = 0;
    
    /* Use operations that might benefit from different architectures */
    for (int i = 0; i < 100; i++) {
        result += x * y + i;
    }
    
    /* Prevent constant folding with volatile */
    return result;
}

/* Another approach: function with explicit target attribute */
__attribute__((target("avx512f"), noinline, used))
static int avx512_specific(int x) {
    /* This might trigger creation of helper/resolver functions */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += x + i;
    }
    return sum;
}

/* Function using builtins that may require runtime support */
__attribute__((noinline, used))
int use_atomic_builtin(long *ptr, long oldval, long newval) {
    /* __atomic_compare_exchange may generate helper functions */
    return __atomic_compare_exchange_n(ptr, &oldval, newval, 
                                       0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}
