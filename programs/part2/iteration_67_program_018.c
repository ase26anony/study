/* test_cpuid_cache.c - Trigger GCC driver CPUID cache detection */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large arrays to encourage cache consideration */
static volatile int array1[1024 * 1024];
static volatile int array2[1024 * 1024];
static volatile int array3[512 * 512];

/* Function pointers to prevent optimization */
typedef int (*compute_func_t)(int, int);
static volatile compute_func_t func_ptr = NULL;

/* Force driver to evaluate CPUID during compilation */
#ifdef __OPTIMIZE__
/* These conditionals force driver to check CPU features */
#if defined(__SSE__) || defined(__SSE2__) || defined(__SSE3__)
#define USE_SSE_FEATURES 1
#else
#define USE_SSE_FEATURES 0
#endif

#if defined(__AVX__) || defined(__AVX2__)
#define USE_AVX_FEATURES 1
#else
#define USE_AVX_FEATURES 0
#endif

#if defined(__AVX512F__) || defined(__AVX512BW__)
#define USE_AVX512_FEATURES 1
#else
#define USE_AVX512_FEATURES 0
#endif
#endif /* __OPTIMIZE__ */

/* Different computation functions for different CPU features */
static int compute_sse_like(int stride, int limit) {
    volatile int sum = 0;
    for (int i = 0; i < limit; i += stride) {
        array1[i] = array2[i] + array3[i % (512 * 512)];
        sum += array1[i];
    }
    return sum;
}

static int compute_avx_like(int stride, int limit) {
    volatile int sum = 0;
    /* Different access pattern */
    for (int i = 0; i < limit; i += stride * 2) {
        array1[i] = array2[i] * 2 - array3[i % (512 * 512)];
        sum ^= array1[i];
    }
    return sum;
}

static int compute_generic(int stride, int limit) {
    volatile int sum = 0;
    for (int i = 0; i < limit; i += stride) {
        array1[i] = i * 3 + array2[i];
        sum += array1[i];
    }
    return sum;
}

/* Initialize arrays with pseudo-random data */
static void init_arrays(void) {
    for (size_t i = 0; i < sizeof(array1)/sizeof(array1[0]); i++) {
        array1[i] = (i * 1103515245 + 12345) & 0x7FFF;
        array2[i] = (i * 1664525 + 1013904223) & 0x7FFF;
    }
    for (size_t i = 0; i < sizeof(array3)/sizeof(array3[0]); i++) {
        array3[i] = (i * 134775813 + 1) & 0x7FFF;
    }
}

int main(void) {
    /* Force CPU initialization - this triggers CPUID in driver */
    __builtin_cpu_init();
    
    volatile int has_sse2 = 0;
    volatile int has_sse3 = 0;
    volatile int has_ssse3 = 0;
    volatile int has_sse4_1 = 0;
    volatile int has_sse4_2 = 0;
    volatile int has_avx = 0;
    volatile int has_avx2 = 0;
    volatile int has_avx512f = 0;
    
    /* Check various CPU features - each call may trigger cache detection */
    has_sse2 = __builtin_cpu_supports("sse2");
    has_sse3 = __builtin_cpu_supports("sse3");
    has_ssse3 = __builtin_cpu_supports("ssse3");
    has_sse4_1 = __builtin_cpu_supports("sse4.1");
    has_sse4_2 = __builtin_cpu_supports("sse4.2");
    has_avx = __builtin_cpu_supports("avx");
    has_avx2 = __builtin_cpu_supports("avx2");
    has_avx512f = __builtin_cpu_supports("avx512f");
    
    /* Use results to prevent dead code elimination */
    volatile int total_features = 
        has_sse2 + has_sse3 + has_ssse3 + 
        has_sse4_1 + has_sse4_2 + 
        has_avx + has_avx2 + has_avx512f;
    
    init_arrays();
    
    /* Choose stride based on detected features - encourages cache optimization */
    int stride = 1;
    if (has_avx512f) stride = 16;
    else if (has_avx2) stride = 8;
    else if (has_avx) stride = 4;
    else if (has_sse4_2) stride = 2;
    
    /* Choose computation function based on features */
    if (has_avx || has_avx2) {
        func_ptr = compute_avx_like;
    } else if (has_sse2 || has_sse3) {
        func_ptr = compute_sse_like;
    } else {
        func_ptr = compute_generic;
    }
    
    /* Perform computation with cache-sensitive access pattern */
    int limit = sizeof(array1)/sizeof(array1[0]);
    if (limit > 100000) limit = 100000; /* Reasonable limit */
    
    volatile int result = func_ptr(stride, limit);
    
    /* Additional cache-sensitive patterns */
    volatile int sum = 0;
    
    /* Pattern 1: Sequential with stride */
    for (int i = 0; i < 65536; i += stride) {
        sum += array1[i];
    }
    
    /* Pattern 2: Reverse access */
    for (int i = 65535; i >= 0; i -= stride) {
        sum += array2[i];
    }
    
    /* Pattern 3: Random-like access (but deterministic) */
    for (int i = 0; i < 32768; i++) {
        int idx = (i * 97) & 0xFFFF;
        sum += array3[idx % (512 * 512)];
    }
    
    printf("Result: %d, Sum: %d, Features: %d\n", result, sum, total_features);
    
    /* Force evaluation of all feature checks */
    #ifdef __OPTIMIZE__
    if (__builtin_cpu_supports("sse") && 
        __builtin_cpu_supports("sse2") &&
        __builtin_cpu_supports("mmx")) {
        /* This block ensures driver evaluates multiple CPUID checks */
        volatile int dummy = has_sse2 + has_sse3;
        (void)dummy;
    }
    #endif
    
    return 0;
}
