/* Compile with: gcc -O2 -flto -fuse-linker-plugin -fno-semantic-interposition target_func.c main.c -o test */

/* Force generation of multi-versioned function with specific attributes */
__attribute__((target_clones("default", "avx2", "avx512f"), 
               noinline, 
               noclone,
               used,
               visibility("hidden")))
static int multi_version_func(int x, int y) {
    /* Use operations that might benefit from different ISA extensions */
    int result = x * y;
    
    /* Add some computation to prevent constant folding */
    for (int i = 0; i < (x & 0xFF); i++) {
        result += (result >> 2);
    }
    
    return result;
}

/* Another approach: Use atomic operations that might generate helper functions */
__attribute__((target("avx512f"), noinline, used))
static long long atomic_helper(long long *ptr, long long val) {
    /* __atomic_compare_exchange with large type may need helper */
    long long expected = *ptr;
    __atomic_compare_exchange(ptr, &expected, &val, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return expected;
}

/* Function using CPU dispatch */
__attribute__((target("default"), noinline, used))
int dispatch_func(int x) {
    if (__builtin_cpu_supports("avx2")) {
        return x * 2;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return x + x;
    }
    return x;
}

/* The function that will be called from main */
int get_computation_result(int input) {
    /* Mix different approaches to increase chance of triggering the hook */
    int a = multi_version_func(input, input + 1);
    long long counter = 0;
    long long b = atomic_helper(&counter, a);
    int c = dispatch_func(a);
    
    return (int)(a + c + b);
}
