/*
 * CPU Cache Detection Test Program
 * Designed to trigger GCC driver's CPUID-based cache detection logic
 * during compilation with various -march and -mtune options.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large static arrays to encourage cache-sensitive optimizations */
static volatile int array1[1024 * 1024];  /* 4MB */
static volatile int array2[1024 * 1024];  /* 4MB */
static volatile int array3[512 * 512];    /* 1MB */

/* Function pointers to prevent optimization */
typedef int (*compute_func_t)(int, int);
static volatile compute_func_t func_ptr = NULL;

/* Force driver to evaluate CPU features during compilation */
#ifdef __OPTIMIZE__
/* These conditionals force driver to check CPU capabilities */
#if defined(__SSE__) || defined(__SSE2__) || defined(__SSE3__)
#define USE_SSE_FEATURES 1
#endif

#if defined(__AVX__) || defined(__AVX2__) || defined(__AVX512F__)
#define USE_AVX_FEATURES 1
#endif
#endif

/* Different computation functions based on CPU features */
static int compute_basic(int stride, int iterations) {
    int sum = 0;
    for (int i = 0; i < iterations; i += stride) {
        array1[i % (1024 * 1024)] = i;
        sum += array1[(i * 7) % (1024 * 1024)];
    }
    return sum;
}

static int compute_sse_like(int stride, int iterations) {
    int sum = 0;
    /* Non-constant stride to prevent optimization */
    int actual_stride = stride | 1;  /* Ensure odd stride */
    for (int i = 0; i < iterations; i += actual_stride) {
        array2[i % (1024 * 1024)] = i * 3;
        sum += array2[(i * 11) % (1024 * 1024)];
        array3[(i / 2) % (512 * 512)] = sum;
    }
    return sum;
}

static int compute_avx_like(int stride, int iterations) {
    int sum = 0;
    /* More complex access pattern */
    for (int i = 0; i < iterations; i += (stride * 2)) {
        int idx1 = (i * 13) % (1024 * 1024);
        int idx2 = (i * 17) % (1024 * 1024);
        int idx3 = (i * 19) % (512 * 512);
        
        array1[idx1] = i;
        array2[idx2] = array1[idx1] * 2;
        array3[idx3] = array2[idx2] + array1[idx1];
        sum += array3[idx3];
    }
    return sum;
}

/* Initialize CPU detection - forces driver to execute CPUID */
static void init_cpu_features(void) {
    /* This builtin forces CPU initialization in the driver */
    __builtin_cpu_init();
    
    /* Check various CPU features to trigger different detection paths */
    volatile int has_sse = __builtin_cpu_supports("sse");
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_sse3 = __builtin_cpu_supports("sse3");
    volatile int has_ssse3 = __builtin_cpu_supports("ssse3");
    volatile int has_sse4_1 = __builtin_cpu_supports("sse4.1");
    volatile int has_sse4_2 = __builtin_cpu_supports("sse4.2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    volatile int has_avx512f = __builtin_cpu_supports("avx512f");
    
    /* Also check cache-related features */
    volatile int has_popcnt = __builtin_cpu_supports("popcnt");
    volatile int has_lzcnt = __builtin_cpu_supports("lzcnt");
    volatile int has_bmi = __builtin_cpu_supports("bmi");
    volatile int has_bmi2 = __builtin_cpu_supports("bmi2");
    
    /* Use volatile results to prevent optimization */
    volatile int feature_mask = 0;
    feature_mask |= has_sse ? 1 : 0;
    feature_mask |= has_sse2 ? 2 : 0;
    feature_mask |= has_sse3 ? 4 : 0;
    feature_mask |= has_ssse3 ? 8 : 0;
    feature_mask |= has_sse4_1 ? 16 : 0;
    feature_mask |= has_sse4_2 ? 32 : 0;
    feature_mask |= has_avx ? 64 : 0;
    feature_mask |= has_avx2 ? 128 : 0;
    feature_mask |= has_avx512f ? 256 : 0;
    
    /* Select function based on detected features */
    if (has_avx2 || has_avx512f) {
        func_ptr = compute_avx_like;
    } else if (has_sse2 || has_sse3) {
        func_ptr = compute_sse_like;
    } else {
        func_ptr = compute_basic;
    }
}

/* Main computation with cache-sensitive access patterns */
static int perform_cache_sensitive_computation(void) {
    int total_sum = 0;
    
    /* Multiple loops with different strides to test various cache behaviors */
    const int iterations = 1000000;
    
    /* Stride 1 - sequential access */
    if (func_ptr) {
        total_sum += func_ptr(1, iterations);
    }
    
    /* Stride 16 - potential cache line size */
    total_sum += compute_basic(16, iterations / 4);
    
    /* Stride 64 - potential L1 cache associativity */
    total_sum += compute_sse_like(64, iterations / 8);
    
    /* Stride 256 - potential L2 cache behavior */
    total_sum += compute_avx_like(256, iterations / 16);
    
    /* Stride 1024 - potential L3 cache behavior */
    for (int i = 0; i < iterations / 32; i += 1024) {
        int idx = (i * 23) % (1024 * 1024);
        array1[idx] = i;
        array2[(idx * 7) % (1024 * 1024)] = array1[idx] * 3;
        total_sum += array2[(idx * 11) % (1024 * 1024)];
    }
    
    return total_sum;
}

int main(void) {
    /* Initialize CPU detection - critical for triggering driver logic */
    init_cpu_features();
    
    /* Perform cache-sensitive computations */
    int result = perform_cache_sensitive_computation();
    
    /* Use result to prevent dead code elimination */
    printf("CPU Cache Test Result: %d\n", result);
    printf("Array checksum: %d\n", 
           (array1[0] + array2[100] + array3[1000]) & 0xFF);
    
    return 0;
}

/* Additional compilation-time conditionals to force driver evaluation */
#ifdef __FAST_MATH__
/* When fast-math is enabled, check for FMA support */
static volatile int has_fma = __builtin_cpu_supports("fma");
#endif

#ifdef __OPTIMIZE__
/* Optimization-specific CPU feature checks */
#if __OPTIMIZE__ > 0
static volatile int opt_level_specific_check = 
    __builtin_cpu_supports("sse") ? 1 : 0;
#endif
#endif
