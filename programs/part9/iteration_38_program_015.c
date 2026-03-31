/* Compile with: gcc -O2 -flto -fuse-linker-plugin -fno-semantic-interposition target_func.c main.c -o test */

/* Force generation of multi-versioned function with specific attributes */
__attribute__((target_clones("default", "avx2", "avx512f")))
__attribute__((noinline, noclone, used, visibility("hidden")))
static int multi_version_func(int x, int y) {
    /* Use atomic operation that might require helper function */
    long long atomic_var = 0;
    long long expected = 0;
    long long desired = 1;
    
    /* This may trigger internal helper generation */
    __atomic_compare_exchange_n(&atomic_var, &expected, desired, 
                                0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    /* Computation that varies with architecture */
    int result = x * y;
    
    /* Use CPU feature detection that may require runtime dispatch */
    if (__builtin_cpu_supports("avx2")) {
        result += 1000;
    }
    if (__builtin_cpu_supports("avx512f")) {
        result += 2000;
    }
    
    return result;
}

/* Another approach: function with explicit target attributes */
__attribute__((target("avx2"), noinline, used, visibility("hidden")))
extern int avx2_func(int x) {
    /* Force generation of exception handling helpers in C++ mode */
    volatile int result = x * 2;
    
    /* Use inline assembly to prevent optimization */
    asm volatile("" : "+r" (result) : : "memory");
    
    return result;
}

/* Function that will be called from main */
int get_computation(int x, int y) {
    return multi_version_func(x, y) + avx2_func(x);
}
