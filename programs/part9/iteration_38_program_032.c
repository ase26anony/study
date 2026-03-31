/* Compile with: gcc -O2 -flto -fuse-linker-plugin -fno-semantic-interposition target_func.c main.c -o test */

/* Force generation of multi-versioned function with internal helper */
__attribute__((target_clones("default", "avx2", "avx512f")))
__attribute__((noinline, noclone, used, visibility("hidden")))
static int multi_version_func(int x, int y) {
    /* Use operations that might benefit from different ISA extensions */
    int result = x * y;
    
    /* Add some computation to prevent optimization */
    for (int i = 0; i < 100; i++) {
        result += (x ^ y) & i;
    }
    
    /* Use atomic operation that might need helper function */
    int atomic_var = 0;
    __atomic_compare_exchange_n(&atomic_var, &x, y, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    return result + atomic_var;
}

/* Another function with explicit target attribute */
__attribute__((target("avx512f"), noinline, used, visibility("hidden")))
extern int avx512_specific(int x) {
    /* Use __builtin_cpu_supports to potentially trigger helper generation */
    if (__builtin_cpu_supports("avx512f")) {
        return x * x + 12345;
    }
    return x;
}

/* Function that might trigger exception handling helpers */
__attribute__((target("arch=armv8-a+crc"), noinline, used))
static int crc_checksum(const char *data, int len) {
    int sum = 0;
    for (int i = 0; i < len; i++) {
        sum = __builtin_ia32_crc32qi(sum, data[i]);
    }
    return sum;
}

/* Public declaration to satisfy external linkage */
int get_multi_version_result(int x, int y) {
    return multi_version_func(x, y) + avx512_specific(x);
}
