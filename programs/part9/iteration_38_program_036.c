/* Compile with: gcc -O2 -flto -fuse-linker-plugin -c target_func.c */

/* Force generation of multi-versioned function with specific attributes */
__attribute__((target_clones("default", "avx2", "avx512f")))
__attribute__((noinline, noclone, used, visibility("hidden")))
static int multi_version_func(int x, int y) {
    /* Complex enough to not be optimized away, but simple for demonstration */
    volatile int result = 0;
    
    /* Use operations that might benefit from different ISA extensions */
    for (int i = 0; i < x; i++) {
        result += y * i;
        result ^= (result << 3);
        result ^= (result >> 5);
    }
    
    /* Force volatile access to prevent optimization */
    asm volatile("" : "+r" (result));
    
    return result;
}

/* Another approach: function with explicit target attribute */
__attribute__((target("avx512f"), noinline, used, visibility("hidden")))
static int avx512_specific_func(float *a, float *b, int n) {
    float sum = 0.0f;
    /* Simulate AVX512-specific computation */
    for (int i = 0; i < n; i++) {
        sum += a[i] * b[i];
    }
    return (int)sum;
}

/* Function that uses atomic builtin which may require helper */
__attribute__((noinline, used))
static long atomic_helper_test(long *ptr, long old_val, long new_val) {
    /* This may generate internal helper for atomic operations */
    return __atomic_compare_exchange_n(ptr, &old_val, new_val, 
                                       0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

/* Export the functions */
int get_multi_version_result(int x, int y) {
    return multi_version_func(x, y);
}

int get_avx512_result(float *a, float *b, int n) {
    return avx512_specific_func(a, b, n);
}

long test_atomic_helper(long *ptr, long old_val, long new_val) {
    return atomic_helper_test(ptr, old_val, new_val);
}
