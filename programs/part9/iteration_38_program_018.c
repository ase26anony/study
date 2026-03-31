/* Compile with: gcc -O2 -flto -fuse-linker-plugin target_func.c main.c -o test */

/* Force generation of multi-versioned function with specific attributes */
__attribute__((target_clones("default", "avx2", "avx512f"), 
               noinline, 
               noclone,
               used,
               visibility("hidden")))
static int multi_version_func(int x, int y) {
    /* Complex enough to not be optimized away, but simple for demonstration */
    volatile int result = 0;
    
    /* Use operations that might benefit from different instruction sets */
    for (int i = 0; i < 100; i++) {
        result += x * y + i;
    }
    
    /* Force potential use of SIMD operations */
    int arr[4] = {x, y, x*y, x+y};
    for (int i = 0; i < 4; i++) {
        result += arr[i];
    }
    
    return result;
}

/* Another approach: function with explicit target attribute */
__attribute__((target("avx512f"), 
               noinline,
               used,
               visibility("hidden")))
int avx512_specific(int x) {
    /* Use __builtin_cpu_supports to potentially trigger helper generation */
    if (__builtin_cpu_supports("avx512f")) {
        return x * 2;
    }
    return x;
}

/* Force generation of atomic helper function */
__attribute__((noinline, used))
long atomic_helper(void) {
    /* Large atomic operation that might need runtime support */
    long long large_var = 0;
    long long expected = 0;
    long long desired = 42;
    
    /* This may trigger generation of internal helper functions */
    __atomic_compare_exchange(&large_var, &expected, &desired, 
                              0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    return large_var;
}

/* Function that will be called from main */
__attribute__((target_clones("default", "sse4.2", "avx", "avx2"),
               noinline,
               used,
               visibility("hidden")))
int compute_value(int a, int b) {
    /* Mix of operations to encourage multi-versioning */
    int sum = 0;
    for (int i = 0; i < a; i++) {
        sum += b + i;
        sum *= 3;
    }
    return sum;
}
