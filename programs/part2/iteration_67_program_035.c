/* test_cache_detection.c
 * 
 * This program is designed to trigger GCC's CPUID-based cache detection
 * during compilation, specifically targeting the switch statement in
 * driver-i386.cc lines 127-244 that maps cache descriptor bytes to
 * cache parameters.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large static arrays to encourage cache-sensitive optimizations */
static volatile int array1[1024 * 1024];  /* 4MB */
static volatile int array2[512 * 512];    /* 1MB */
static volatile int array3[256 * 256];    /* 256KB */

/* Function pointers to prevent optimization */
typedef int (*compute_func_t)(int, int);
static compute_func_t func_ptr = NULL;

/* Force CPU initialization at compile-time evaluation */
#ifdef __OPTIMIZE__
/* These conditionals force the driver to evaluate __builtin_cpu_supports */
#if defined(__SSE__) || defined(__SSE2__) || defined(__SSE3__)
#define USE_SSE_FEATURES 1
#endif

#if defined(__AVX__) || defined(__AVX2__)
#define USE_AVX_FEATURES 1
#endif

#if defined(__AVX512F__) || defined(__AVX512BW__)
#define USE_AVX512_FEATURES 1
#endif
#endif /* __OPTIMIZE__ */

/* Different computation functions for different CPU features */
static int compute_sse(int stride, int iterations) {
    int sum = 0;
    for (int i = 0; i < iterations; i += stride) {
        sum += array1[i & 0xFFFFF] * 3;
        sum ^= array2[(i * 7) & 0x3FFFF];
    }
    return sum;
}

static int compute_avx(int stride, int iterations) {
    int sum = 0;
    /* Different access pattern for AVX */
    for (int i = 0; i < iterations; i += stride) {
        sum += array1[i & 0xFFFFF] * 5;
        sum ^= array3[(i * 11) & 0xFFFF];
        if (i % 64 == 0) {
            sum += array2[(i / 64) & 0x3FFFF];
        }
    }
    return sum;
}

static int compute_avx512(int stride, int iterations) {
    int sum = 0;
    /* More complex pattern for AVX512 */
    for (int i = 0; i < iterations; i += stride) {
        int idx1 = i & 0xFFFFF;
        int idx2 = (i * 13) & 0x3FFFF;
        int idx3 = (i * 17) & 0xFFFF;
        
        sum += array1[idx1] * 7;
        sum ^= array2[idx2] * 3;
        sum += array3[idx3] * 11;
        
        /* Cache line boundary crossing */
        if ((i & 63) == 0) {
            sum += array1[(idx1 + 64) & 0xFFFFF];
        }
    }
    return sum;
}

static int compute_generic(int stride, int iterations) {
    int sum = 0;
    /* Simple but cache-sensitive pattern */
    for (int i = 0; i < iterations; i += stride) {
        sum = sum * 31 + array1[i & 0xFFFFF];
        if (i % 128 == 0) {
            sum ^= array2[(i / 128) & 0x3FFFF];
        }
    }
    return sum;
}

/* Initialize arrays with pseudo-random but deterministic values */
static void init_arrays(void) {
    for (size_t i = 0; i < sizeof(array1)/sizeof(array1[0]); i++) {
        array1[i] = (i * 1103515245 + 12345) & 0x7FFF;
    }
    for (size_t i = 0; i < sizeof(array2)/sizeof(array2[0]); i++) {
        array2[i] = (i * 1664525 + 1013904223) & 0x7FFF;
    }
    for (size_t i = 0; i < sizeof(array3)/sizeof(array3[0]); i++) {
        array3[i] = (i * 214013 + 2531011) & 0x7FFF;
    }
}

int main(void) {
    /* Initialize CPU detection - this triggers __builtin_cpu_init */
    __builtin_cpu_init();
    
    /* Volatile variables to prevent constant folding */
    volatile int has_sse2 = 0;
    volatile int has_sse3 = 0;
    volatile int has_ssse3 = 0;
    volatile int has_sse4_1 = 0;
    volatile int has_sse4_2 = 0;
    volatile int has_avx = 0;
    volatile int has_avx2 = 0;
    volatile int has_avx512f = 0;
    
    /* Check multiple CPU features to ensure thorough CPUID evaluation */
    has_sse2 = __builtin_cpu_supports("sse2");
    has_sse3 = __builtin_cpu_supports("sse3");
    has_ssse3 = __builtin_cpu_supports("ssse3");
    has_sse4_1 = __builtin_cpu_supports("sse4.1");
    has_sse4_2 = __builtin_cpu_supports("sse4.2");
    has_avx = __builtin_cpu_supports("avx");
    has_avx2 = __builtin_cpu_supports("avx2");
    has_avx512f = __builtin_cpu_supports("avx512f");
    
    /* Initialize arrays */
    init_arrays();
    
    /* Choose computation function based on CPU features */
    if (has_avx512f) {
        func_ptr = compute_avx512;
    } else if (has_avx2) {
        func_ptr = compute_avx;
    } else if (has_sse4_2) {
        func_ptr = compute_sse;
    } else {
        func_ptr = compute_generic;
    }
    
    /* Choose stride based on cache line detection */
    int stride = 1;
    if (has_sse2) stride = 2;
    if (has_sse4_1) stride = 4;
    if (has_avx) stride = 8;
    if (has_avx512f) stride = 16;
    
    /* Perform computation with cache-sensitive access patterns */
    int iterations = 1000000;
    int result = func_ptr(stride, iterations);
    
    /* Additional computations to use different array access patterns */
    int checksum = 0;
    
    /* Pattern 1: Sequential access with varying strides */
    for (int i = 0; i < 65536; i += stride) {
        checksum += array1[i];
    }
    
    /* Pattern 2: Strided access crossing cache lines */
    for (int i = 0; i < 32768; i += 64) {
        checksum ^= array2[i];
    }
    
    /* Pattern 3: Small array with high reuse */
    for (int j = 0; j < 100; j++) {
        for (int i = 0; i < 1024; i++) {
            checksum += array3[i];
        }
    }
    
    /* Combine results */
    result = (result + checksum) & 0xFFFF;
    
    printf("Result: %d\n", result);
    printf("CPU Features: SSE2=%d, SSE4.2=%d, AVX=%d, AVX2=%d, AVX512F=%d\n",
           has_sse2, has_sse4_2, has_avx, has_avx2, has_avx512f);
    
    return 0;
}
