/* Compile with: gcc -O2 -flto -fuse-linker-plugin -c target_func.c */
/* This function uses target_clones to force generation of resolver/helper functions */
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
__attribute__((noinline, noclone, used, visibility("hidden")))
static int multi_version_func(int x, int y) {
    /* Complex enough to not be optimized away, but simple for demonstration */
    volatile int result = 0;
    
    /* Use operations that might benefit from different instruction sets */
    for (int i = 0; i < x; i++) {
        result += y * i;
        result ^= (result << 3);
        result ^= (result >> 5);
    }
    
    /* Force volatile access to prevent optimization */
    asm volatile("" : "+r" (result) : : "memory");
    
    return result;
}

/* Another approach: function with explicit target attribute */
__attribute__((target("avx512f"), noinline, noclone, used, visibility("hidden")))
static int avx512_specific_func(int x) {
    /* This might trigger creation of helper functions */
    volatile int sum = 0;
    
    /* Simulate some computation */
    for (int i = 0; i < x; i++) {
        sum += i * i;
    }
    
    /* Use __builtin_cpu_supports to potentially trigger internal helpers */
    if (__builtin_cpu_supports("avx512f")) {
        sum += 1000;
    }
    
    asm volatile("" : "+r" (sum) : : "memory");
    return sum;
}

/* Force generation of atomic helper functions */
__attribute__((noinline, used))
static long long atomic_helper_test(void) {
    /* Large atomic operation that might need helper functions */
    struct large_struct {
        long long a, b, c, d;
    } shared = {0};
    
    struct large_struct desired = {1, 2, 3, 4};
    struct large_struct expected = {0};
    
    /* This complex atomic operation may trigger compiler helpers */
    __atomic_compare_exchange(&shared, &expected, &desired, 0, 
                             __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    return shared.a + shared.b + shared.c + shared.d;
}

/* Export the functions */
int get_multi_version_result(int x, int y) {
    return multi_version_func(x, y) + avx512_specific_func(x);
}

long long get_atomic_result(void) {
    return atomic_helper_test();
}
