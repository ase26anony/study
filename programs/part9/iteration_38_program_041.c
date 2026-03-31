/* Compile with: gcc -O2 -flto -fuse-linker-plugin -c target_func.c */
/* This file contains the function designed to trigger the target hooks */

/* Force generation of multi-versioned function with specific attributes */
__attribute__((target_clones("default", "avx2", "avx512f")))
__attribute__((noinline, noclone, used, visibility("hidden")))
static int multi_version_func(int x, int y) {
    /* Use atomic operation that may require helper function */
    long long atomic_var = 0;
    long long expected = 0;
    long long desired = (long long)x * y;
    
    /* This may trigger generation of atomic helper functions */
    __atomic_compare_exchange_n(&atomic_var, &expected, desired, 
                                0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    /* Use CPU feature detection that requires runtime dispatch */
    if (__builtin_cpu_supports("avx2")) {
        return x * y + 1;
    } else if (__builtin_cpu_supports("avx512f")) {
        return x * y + 2;
    }
    
    /* Force use of target-specific builtins */
    #ifdef __SSE2__
    __builtin_ia32_lfence();
    #endif
    
    return x * y;
}

/* Another approach: function with explicit target attribute */
__attribute__((target("avx512f"), noinline, used))
extern int avx512_func(int x) {
    /* Complex computation that compiler might lower to helper */
    int arr[16] = {0};
    
    /* Use OpenMP pragma that requires runtime stubs */
    #pragma omp simd
    for (int i = 0; i < 16; i++) {
        arr[i] = x * i;
    }
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += arr[i];
    }
    
    return sum;
}

/* Function that will be called from main */
__attribute__((noinline))
int get_computation(int a, int b) {
    /* Mix both approaches to increase chance of triggering hooks */
    int result1 = multi_version_func(a, b);
    int result2 = avx512_func(a);
    
    /* Prevent optimization */
    asm volatile("" : "+r"(result1), "+r"(result2));
    
    return result1 + result2;
}
