/*
 * CPU Cache Detection Test Program
 * Designed to trigger GCC driver's CPUID-based cache detection logic
 * during compilation with various -march options.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large static arrays to encourage cache consideration */
static volatile int array1[1024 * 1024];
static volatile int array2[1024 * 1024];
static volatile int array3[512 * 512];

/* Force driver to evaluate CPU features during compilation */
#ifdef __OPTIMIZE__
/* These conditionals force driver to check CPU features */
#if defined(__SSE__) || defined(__SSE2__) || defined(__SSE3__)
#define USE_SSE_FEATURES 1
#endif

#if defined(__AVX__) || defined(__AVX2__) || defined(__AVX512F__)
#define USE_AVX_FEATURES 1
#endif
#endif

/* Function pointer to prevent optimization */
typedef int (*ComputeFunc)(int*, size_t, int);

/* Different computation patterns based on CPU features */
static int compute_basic(int* data, size_t size, int stride) {
    int sum = 0;
    for (size_t i = 0; i < size; i += stride) {
        sum += data[i];
        data[i] = sum;
    }
    return sum;
}

static int compute_sse_like(int* data, size_t size, int stride) {
    int sum = 0;
    /* Simulate SSE-style access pattern */
    for (size_t i = 0; i < size; i += stride * 4) {
        sum += data[i];
        sum += data[i + stride];
        sum += data[i + stride * 2];
        sum += data[i + stride * 3];
        data[i] = sum;
    }
    return sum;
}

/* Volatile function pointer to prevent constant folding */
static volatile ComputeFunc func_ptr = compute_basic;

int main(void) {
    /* Initialize CPU detection - forces driver to run CPUID */
    __builtin_cpu_init();
    
    /* Check various CPU features - each requires driver to evaluate CPUID */
    volatile int has_sse = __builtin_cpu_supports("sse");
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_sse3 = __builtin_cpu_supports("sse3");
    volatile int has_ssse3 = __builtin_cpu_supports("ssse3");
    volatile int has_sse4_1 = __builtin_cpu_supports("sse4.1");
    volatile int has_sse4_2 = __builtin_cpu_supports("sse4.2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    volatile int has_avx512f = __builtin_cpu_supports("avx512f");
    
    /* Cache-sensitive stride selection based on detected features */
    int stride = 1;
    
    /* Different stride patterns to trigger cache consideration */
    if (has_sse) stride = 2;
    if (has_sse2) stride = 4;
    if (has_sse3) stride = 8;
    if (has_ssse3) stride = 16;
    if (has_sse4_1 || has_sse4_2) stride = 32;
    if (has_avx) stride = 64;
    if (has_avx2) stride = 128;
    if (has_avx512f) stride = 256;
    
    /* Switch function based on features */
    if (has_sse2 || has_sse3) {
        func_ptr = compute_sse_like;
    }
    
    /* Initialize arrays with pattern */
    for (size_t i = 0; i < sizeof(array1)/sizeof(array1[0]); i++) {
        array1[i] = (int)(i * 3 + 1);
    }
    
    for (size_t i = 0; i < sizeof(array2)/sizeof(array2[0]); i++) {
        array2[i] = (int)(i * 5 - 2);
    }
    
    for (size_t i = 0; i < sizeof(array3)/sizeof(array3[0]); i++) {
        array3[i] = (int)(i * 7 + 3);
    }
    
    /* Perform cache-sensitive computations */
    int sum1 = func_ptr((int*)array1, sizeof(array1)/sizeof(array1[0]), stride);
    int sum2 = func_ptr((int*)array2, sizeof(array2)/sizeof(array2[0]), stride * 2);
    int sum3 = func_ptr((int*)array3, sizeof(array3)/sizeof(array3[0]), stride / 2);
    
    /* Use results to prevent dead code elimination */
    volatile int final_result = sum1 + sum2 * 3 - sum3;
    
    /* Additional CPU-specific optimizations */
#ifdef __OPTIMIZE__
    /* Force driver to consider different cache levels */
    if (has_sse2) {
        /* SSE2-specific memory access pattern */
        for (int i = 0; i < 1000; i += stride) {
            array1[i] = array1[i] * 2 + array2[i];
        }
    }
    
    if (has_avx) {
        /* AVX-specific pattern with larger strides */
        for (int i = 0; i < 1000; i += stride * 4) {
            array3[i] = array1[i] + array2[i] * 3;
        }
    }
#endif
    
    printf("CPU Feature Summary:\n");
    printf("  SSE: %d, SSE2: %d, SSE3: %d\n", has_sse, has_sse2, has_sse3);
    printf("  SSSE3: %d, SSE4.1: %d, SSE4.2: %d\n", has_ssse3, has_sse4_1, has_sse4_2);
    printf("  AVX: %d, AVX2: %d, AVX512F: %d\n", has_avx, has_avx2, has_avx512f);
    printf("Selected stride: %d\n", stride);
    printf("Computation result: %d\n", final_result);
    
    return final_result & 0xFF; /* Return non-zero to indicate success */
}

/* Additional functions to create more compilation contexts */
#ifdef __SSE2__
static void sse2_optimized_loop(volatile int* arr, int size) {
    for (int i = 0; i < size; i += 16) {
        arr[i] = arr[i] * 3 + 1;
    }
}
#endif

#ifdef __AVX__
static void avx_optimized_loop(volatile int* arr, int size) {
    for (int i = 0; i < size; i += 32) {
        arr[i] = arr[i] * 5 - 2;
    }
}
#endif
