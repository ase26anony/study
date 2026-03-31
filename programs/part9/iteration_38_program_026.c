/* Compile with: gcc -O2 -flto -fuse-linker-plugin -fno-semantic-interposition target_func.c main.c -o test */

/* Force generation of multi-versioned function with specific attributes */
__attribute__((target_clones("default", "avx2", "avx512f"), 
               noinline, 
               noclone,
               used,
               visibility("hidden")))
static int multi_version_func(int x, int y) {
    /* Complex enough to not be optimized away, but simple for demonstration */
    volatile int result = 0;
    
    /* Use operations that might benefit from different architectures */
    for (int i = 0; i < x; i++) {
        result += y * i;
        result ^= (result << 3);
        result ^= (result >> 5);
    }
    
    /* Force dependency on arguments to prevent constant folding */
    return result ^ (x * y);
}

/* Another approach: Use __builtin_cpu_supports which may generate helpers */
__attribute__((target("avx512f"), noinline, used))
static int avx512_specific(int *data, int size) {
    int sum = 0;
    /* This might trigger generation of helper functions */
    for (int i = 0; i < size; i++) {
        sum += data[i];
    }
    return sum;
}

/* Function that will be called from main */
int get_computation_result(int mode, int value);
