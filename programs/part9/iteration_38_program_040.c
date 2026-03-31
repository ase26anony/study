/* Compile with: gcc -O2 -flto -fuse-linker-plugin -fno-semantic-interposition target_func.c main.c -o test */

/* Force generation of multi-versioned function with specific attributes */
__attribute__((target_clones("default", "avx2", "avx512f")))
__attribute__((noinline, used, visibility("hidden")))
static int multi_version_func(int x, int y) {
    /* Use operations that might benefit from different ISA extensions */
    int result = x * y;
    
    /* Use atomic operation that might require helper function */
    __atomic_store_n(&result, x + y, __ATOMIC_RELAXED);
    
    /* Prevent constant folding */
    asm volatile("" : "+r" (result));
    
    return result;
}

/* Another approach: function with explicit target attribute */
__attribute__((target("avx512f"), noinline, used))
extern int avx512_func(int x);

/* Definition with internal linkage but external declaration */
__attribute__((target("avx512f"), noinline, used, visibility("hidden")))
static int avx512_func(int x) {
    /* Complex computation that might require runtime support */
    int arr[16] = {0};
    
    /* Use __builtin_cpu_supports which may generate helper functions */
    if (__builtin_cpu_supports("avx512f")) {
        for (int i = 0; i < 16; i++) {
            arr[i] = x * i;
        }
    }
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += arr[i];
    }
    
    /* Use atomic operation for 128-bit type (may require libcall) */
    __int128 large_val = 0;
    __atomic_compare_exchange_n(&large_val, &large_val, sum, 0, 
                                __ATOMIC_RELAXED, __ATOMIC_RELAXED);
    
    return sum + (int)large_val;
}

/* Function that uses OpenMP to trigger helper generation */
#pragma omp declare target
__attribute__((noinline, used, visibility("hidden")))
static int omp_helper(int x) {
    return x * 2;
}
#pragma omp end declare target

/* Export the multi-versioned function */
int get_multi_version_result(int x, int y) {
    return multi_version_func(x, y) + avx512_func(x);
}
