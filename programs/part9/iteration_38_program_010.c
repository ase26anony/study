/* target_func.c - Contains functions designed to trigger target hooks */

/* Technique 1: Multi-versioned function with target_clones */
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
__attribute__((noinline, used, visibility("hidden")))
int multi_version_func(int x, int y) {
    /* Complex enough to not be optimized away, but simple for demonstration */
    volatile int result = 0;
    
    /* Use operations that might benefit from different instruction sets */
    for (int i = 0; i < 100; i++) {
        result += x * y + i;
    }
    
    /* Use __atomic operation which may generate helper functions */
    __atomic_store_n(&result, result, __ATOMIC_RELAXED);
    
    return result;
}

/* Technique 2: Function with explicit target attribute */
__attribute__((target("avx512f"), noinline, used))
static float avx512_vector_op(float* a, float* b, int n) {
    float sum = 0.0f;
    /* This should encourage AVX512 code generation */
    for (int i = 0; i < n; i++) {
        sum += a[i] * b[i];
    }
    return sum;
}

/* Technique 3: Function using CPU feature detection */
__attribute__((noinline))
int cpu_feature_demo(void) {
    /* This builtin may trigger internal helper generation */
    if (__builtin_cpu_supports("avx2")) {
        return 1;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return 2;
    }
    return 0;
}

/* Technique 4: Atomic operation with large type (may need helper) */
typedef struct {
    long long a;
    long long b;
    long long c;
    long long d;
} large_type;

__attribute__((noinline))
int atomic_large_op(large_type* ptr) {
    large_type expected = {0};
    large_type desired = {1, 2, 3, 4};
    
    /* Large atomic compare-exchange may need runtime helper */
    return __atomic_compare_exchange(ptr, &expected, &desired, 
                                     0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

/* Technique 5: External declaration to match TREE_PUBLIC=1, DECL_EXTERNAL=1 pattern */
extern void external_helper(void);

/* The actual definition with static linkage */
__attribute__((target("default"), used, visibility("hidden")))
static void external_helper(void) {
    /* Empty - the declaration itself is what matters */
}
