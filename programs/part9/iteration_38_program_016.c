/* Compile with: gcc -O2 -flto -fuse-linker-plugin target_func.c main.c -o test */

/* Force generation of multi-versioned function with specific attributes */
__attribute__((target_clones("default", "avx2", "avx512f")))
__attribute__((noinline, noclone, used, visibility("hidden")))
static int multi_version_func(int x, int y) {
    /* Use atomic operation that might require helper function */
    long long large_val = (long long)x * y;
    __atomic_store_n(&large_val, large_val, __ATOMIC_RELAXED);
    
    /* Use target-specific builtin that may require runtime check */
    if (__builtin_cpu_supports("avx2")) {
        return x + y;
    } else if (__builtin_cpu_supports("avx512f")) {
        return x * y;
    }
    return x - y;
}

/* Another approach: function with explicit target attribute */
__attribute__((target("avx512f"), noinline, used, visibility("hidden")))
static int avx512_specific(int* arr, int n) {
    /* Complex operation that compiler might lower to helper */
    int sum = 0;
    for (int i = 0; i < n; i++) {
        __atomic_fetch_add(&sum, arr[i], __ATOMIC_RELAXED);
    }
    return sum;
}

/* Function that will be called from main */
int get_computation_result(int x, int y, int* data, int n) {
    int r1 = multi_version_func(x, y);
    int r2 = avx512_specific(data, n);
    return r1 + r2;
}
