/* Compile with: gcc -O2 -flto -fuse-linker-plugin target_func.c main.c -o test */

/* Force generation of multi-versioned function with specific attributes */
__attribute__((target_clones("default", "avx2", "avx512f")))
__attribute__((noinline, noclone, used, visibility("hidden")))
static int multi_version_func(int x, int y) {
    /* Use atomic operation that might require helper function */
    long long atomic_var = 0;
    long long expected = 0;
    long long desired = x * y;
    
    /* This may trigger internal helper generation */
    __atomic_compare_exchange_n(&atomic_var, &expected, desired, 
                                0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    /* Use target-specific builtin that requires runtime check */
    if (__builtin_cpu_supports("avx2")) {
        return x * y + 1;
    } else if (__builtin_cpu_supports("avx512f")) {
        return x * y + 2;
    }
    
    return x * y;
}

/* Another approach: function with explicit target attribute */
__attribute__((target("avx512f"), noinline, used, visibility("hidden")))
extern int avx512_specific(int x) {
    /* Complex computation that compiler might lower to helper */
    volatile int result = x;
    for (int i = 0; i < 100; i++) {
        result = result * 1103515245 + 12345;
    }
    return result;
}

/* Function that uses ifunc for indirect dispatch */
static int default_impl(int x) { return x * 2; }
static int avx2_impl(int x) { return x * 3; }

static int (*resolve_multiversion(void))(int) {
    if (__builtin_cpu_supports("avx2")) return avx2_impl;
    return default_impl;
}

/* ifunc resolver - may be marked as artificial */
extern int multiversion_ifunc(int x) 
    __attribute__((ifunc("resolve_multiversion")));

/* Public declaration for the multi-versioned function */
extern int multi_version_func(int x, int y);
