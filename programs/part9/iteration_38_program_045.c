/* Compile with: gcc -O2 -flto -fuse-linker-plugin -fno-semantic-interposition target_func.c main.c -o test */

/* Force generation of artificial helper functions through multi-versioning */
__attribute__((target_clones("default", "avx2", "avx512f")))
__attribute__((noinline, used, visibility("hidden")))
static int multi_version_func(int x, int y) {
    /* Use atomic operation that may require helper function */
    long long large_val = (long long)x * y;
    __atomic_store_n(&large_val, x + y, __ATOMIC_SEQ_CST);
    
    /* Use builtin that may require runtime check */
    if (__builtin_cpu_supports("avx2")) {
        return x * y + 1;
    } else if (__builtin_cpu_supports("avx512f")) {
        return x * y + 2;
    }
    return x * y;
}

/* Another approach: function with explicit target attribute */
__attribute__((target("avx512f"), noinline, used))
extern int avx512_func(int x);

/* Definition with static linkage but external declaration */
__attribute__((target("avx512f"), noinline, used, visibility("hidden")))
static int avx512_func(int x) {
    /* Complex computation that can't be optimized away */
    volatile int result = x;
    for (int i = 0; i < 100; i++) {
        result = result * 1103515245 + 12345;
    }
    return result & 0x7fffffff;
}

/* Function that will be called from main */
__attribute__((noinline))
int compute_value(int input) {
    int a = multi_version_func(input, input + 1);
    int b = avx512_func(input);
    return a + b;
}
