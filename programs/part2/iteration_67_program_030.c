/* test_cpuid_cache.c - Trigger GCC driver CPUID cache detection */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large arrays to encourage cache consideration */
static volatile int array_a[1024 * 1024];
static volatile int array_b[1024 * 1024];
static volatile int array_c[512 * 512];

/* Function pointers to prevent optimization */
typedef int (*compute_func_t)(int, int);
static volatile compute_func_t func_ptr = NULL;

/* Force driver to evaluate CPUID during compilation */
#ifdef __OPTIMIZE__
/* These conditionals require driver to check CPU features */
#if defined(__SSE2__) || defined(__AVX__) || defined(__AVX512F__)
#define USE_VECTOR_FEATURES 1
#else
#define USE_VECTOR_FEATURES 0
#endif
#endif

/* CPU feature checks that force __builtin_cpu_init */
static void init_cpu_features(void) {
    /* This forces GCC driver to initialize CPU detection */
    __builtin_cpu_init();
    
    /* Volatile variables prevent constant folding */
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_sse3 = __builtin_cpu_supports("sse3");
    volatile int has_ssse3 = __builtin_cpu_supports("ssse3");
    volatile int has_sse4_1 = __builtin_cpu_supports("sse4.1");
    volatile int has_sse4_2 = __builtin_cpu_supports("sse4.2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    volatile int has_avx512f = __builtin_cpu_supports("avx512f");
    
    /* Cache-specific feature checks */
    volatile int has_clflush = __builtin_cpu_supports("clflush");
    volatile int has_clflushopt = __builtin_cpu_supports("clflushopt");
    volatile int has_clwb = __builtin_cpu_supports("clwb");
    
    (void)has_sse2; (void)has_sse3; (void)has_ssse3;
    (void)has_sse4_1; (void)has_sse4_2; (void)has_avx;
    (void)has_avx2; (void)has_avx512f;
    (void)has_clflush; (void)has_clflushopt; (void)has_clwb;
}

/* Different computation patterns based on CPU features */
static int compute_simple(int stride, int size) {
    int sum = 0;
    /* Non-constant stride to prevent optimization */
    for (int i = 0; i < size; i += stride) {
        array_a[i] = array_b[i] + array_c[i % 512];
        sum += array_a[i];
    }
    return sum;
}

static int compute_with_prefetch(int stride, int size) {
    int sum = 0;
    /* Pattern that might benefit from cache awareness */
    for (int i = 0; i < size; i += stride) {
        /* Access with some locality */
        int idx = (i * 17) % size;  /* Non-linear access */
        array_a[idx] = array_b[i] * array_c[idx % 512];
        sum ^= array_a[idx];  /* Use XOR to prevent easy optimization */
    }
    return sum;
}

/* Conditional compilation that forces driver to evaluate CPUID */
#ifdef __OPTIMIZE__
static compute_func_t select_compute_func(void) {
    /* This requires driver to know CPU features at compile time */
    if (__builtin_cpu_supports("avx512f")) {
        return compute_with_prefetch;
    } else if (__builtin_cpu_supports("avx2")) {
        return compute_with_prefetch;
    } else if (__builtin_cpu_supports("sse4.2")) {
        return compute_with_prefetch;
    } else {
        return compute_simple;
    }
}
#else
static compute_func_t select_compute_func(void) {
    return compute_simple;
}
#endif

/* Main function with cache-sensitive access patterns */
int main(void) {
    int result = 0;
    
    /* Initialize CPU detection - triggers driver's CPUID logic */
    init_cpu_features();
    
    /* Select computation function based on CPU features */
    func_ptr = select_compute_func();
    
    /* Try different strides to exercise different cache behaviors */
    const int sizes[] = {1024, 2048, 4096, 8192, 16384};
    const int strides[] = {1, 2, 4, 8, 16, 32, 64};
    
    for (int s = 0; s < sizeof(strides)/sizeof(strides[0]); s++) {
        for (int n = 0; n < sizeof(sizes)/sizeof(sizes[0]); n++) {
            if (func_ptr) {
                result += func_ptr(strides[s], sizes[n]);
            }
        }
    }
    
    /* Use result to prevent dead code elimination */
    printf("Computed checksum: %d\n", result);
    
    /* Additional architecture-specific conditionals */
#if defined(__tune_core2__) || defined(__tune_nehalem__) || \
    defined(__tune_sandybridge__) || defined(__tune_skylake_avx512__)
    printf("Compiled for specific microarchitecture\n");
#endif
    
    return result != 0 ? 0 : 1;
}

/* Force inclusion of various arch-specific code paths */
#ifdef __SSE2__
void __attribute__((used)) sse2_path(void) {
    volatile int check = __builtin_cpu_supports("sse2");
    (void)check;
}
#endif

#ifdef __AVX__
void __attribute__((used)) avx_path(void) {
    volatile int check = __builtin_cpu_supports("avx");
    (void)check;
}
#endif

#ifdef __AVX512F__
void __attribute__((used)) avx512_path(void) {
    volatile int check = __builtin_cpu_supports("avx512f");
    (void)check;
}
#endif
