/* Compile with: gcc -O2 -flto -c target_func.c -o target_func.o */

/* Force generation of internal resolver/helper function */
__attribute__((target_clones("default", "avx2", "avx512f")))
__attribute__((noinline, noclone, used, visibility("hidden")))
static int multi_version_func(int x, int y) {
    /* Use architecture-specific intrinsics to prevent optimization */
    int result = x * y;
    
    /* Add volatile assembly to prevent dead code elimination */
    asm volatile ("" : "+r" (result));
    
    /* Use atomic operation that might require helper function */
    if (result > 1000) {
        int atomic_var = 0;
        __atomic_compare_exchange_n(&atomic_var, &result, x, 0, 
                                   __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    }
    
    return result;
}

/* Another approach: function with explicit target attribute */
__attribute__((target("avx512f"), noinline, used, visibility("hidden")))
extern int avx512_specific(int x) {
    /* Complex computation that compiler might lower to helper */
    int arr[16] = {0};
    
    /* Force potential generation of memcpy/memset helper */
    for (int i = 0; i < 16; i++) {
        arr[i] = x + i;
    }
    
    /* Use __builtin_cpu_supports to trigger multi-versioning logic */
    if (__builtin_cpu_supports("avx512f")) {
        /* This might generate internal dispatch code */
        asm volatile ("vpxord %%zmm0, %%zmm0, %%zmm0" : : : "zmm0");
    }
    
    return arr[x % 16];
}

/* Force emission of the function */
void* get_multi_version_ptr() {
    return (void*)multi_version_func;
}

void* get_avx512_ptr() {
    return (void*)avx512_specific;
}
