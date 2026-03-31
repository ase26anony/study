/* Compile with: gcc -O2 -flto -fuse-linker-plugin -fno-semantic-interposition target_func.c main.c -o test */

/* Force generation of multi-versioned function with specific attributes */
__attribute__((target_clones("default", "avx2", "avx512f"), 
               noinline, 
               noclone,
               used,
               visibility("hidden")))
static int multi_version_func(int x, int y) {
    /* Use operations that might benefit from different architectures */
    int result = x * y;
    
    /* Use atomic operation that might require helper function */
    __atomic_store_n(&result, x + y, __ATOMIC_RELAXED);
    
    /* Prevent constant folding */
    asm volatile("" : "+r" (result));
    
    return result;
}

/* Another approach: function with explicit target attribute */
__attribute__((target("avx512f"), 
               noinline,
               noclone,
               used,
               visibility("hidden")))
extern int avx512_func(int x, int y);

/* Definition with static linkage but extern declaration */
__attribute__((target("avx512f"), 
               noinline,
               noclone,
               used,
               visibility("hidden")))
static int avx512_func(int x, int y) {
    /* Complex enough to not be optimized away */
    int sum = 0;
    for (int i = 0; i < y; i++) {
        sum += x;
        /* Memory barrier to prevent optimization */
        __sync_synchronize();
    }
    return sum;
}

/* Function that uses __builtin_cpu_supports - may generate internal helpers */
__attribute__((noinline, noclone, used))
int cpu_dispatch_func(int x) {
    if (__builtin_cpu_supports("avx2")) {
        return x * 2;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return x + 1;
    }
    return x;
}

/* Export the functions */
int get_multi_version(int x, int y) {
    return multi_version_func(x, y);
}

int get_avx512_result(int x, int y) {
    return avx512_func(x, y);
}
