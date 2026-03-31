/* test_cpuid_cache.c - Trigger GCC driver CPUID cache detection */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large static arrays to encourage cache consideration */
static volatile int array1[1024 * 1024];
static volatile int array2[512 * 512];
static volatile double array3[256 * 256];

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

#if defined(__AVX512F__)
#define USE_AVX512_FEATURES 1
#else
#define USE_AVX512_FEATURES 0
#endif
#endif /* __OPTIMIZE__ */

/* Different computation patterns based on CPU features */
static int compute_simple(int stride, int limit) {
    int sum = 0;
    for (int i = 0; i < limit; i += stride) {
        array1[i & 0xFFFFF] = i * 3;
        sum += array1[i & 0xFFFFF];
    }
    return sum;
}

static int compute_sse_like(int stride, int limit) {
    int sum = 0;
    /* Simulate SSE-style access pattern */
    for (int i = 0; i < limit; i += stride * 4) {
        for (int j = 0; j < 4; j++) {
            int idx = (i + j * stride) & 0x7FFFF;
            array2[idx] = idx * 5;
            sum += array2[idx];
        }
    }
    return sum;
}

static int compute_avx_like(int stride, int limit) {
    int sum = 0;
    /* Simulate AVX-style wider access pattern */
    for (int i = 0; i < limit; i += stride * 8) {
        for (int j = 0; j < 8; j++) {
            int idx = (i + j * stride) & 0x3FFFF;
            array3[idx] = idx * 7.0;
            sum += (int)array3[idx];
        }
    }
    return sum;
}

/* Initialize CPU detection - this triggers driver's CPUID logic */
static void init_cpu_features(void) {
    /* Force CPU initialization */
    __builtin_cpu_init();
    
    /* Check various CPU features - each call may influence cache detection */
    volatile int has_sse = __builtin_cpu_supports("sse");
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_sse3 = __builtin_cpu_supports("sse3");
    volatile int has_ssse3 = __builtin_cpu_supports("ssse3");
    volatile int has_sse4_1 = __builtin_cpu_supports("sse4.1");
    volatile int has_sse4_2 = __builtin_cpu_supports("sse4.2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    volatile int has_avx512f = __builtin_cpu_supports("avx512f");
    
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
    
    /* Select computation function based on features */
    if (has_avx512f) {
        func_ptr = compute_avx_like;
    } else if (has_avx || has_avx2) {
        func_ptr = compute_avx_like;
    } else if (has_sse4_1 || has_sse4_2) {
        func_ptr = compute_sse_like;
    } else {
        func_ptr = compute_simple;
    }
}

/* Cache-sensitive computation with variable strides */
static int cache_sensitive_computation(int base_stride) {
    int total = 0;
    
    /* Different stride patterns to test various cache behaviors */
    int strides[] = {1, 2, 4, 8, 16, 32, 64, 128};
    int num_strides = sizeof(strides) / sizeof(strides[0]);
    
    for (int s = 0; s < num_strides; s++) {
        int stride = base_stride * strides[s];
        if (stride > 256) continue;
        
        /* Access pattern that depends on stride */
        for (int i = 0; i < 100000; i += stride) {
            int idx1 = (i * 3) & 0xFFFFF;
            int idx2 = (i * 5) & 0x7FFFF;
            int idx3 = (i * 7) & 0x3FFFF;
            
            array1[idx1] = array2[idx2] + array3[idx3];
            array2[idx2] = array1[idx1] * 2;
            array3[idx3] = array2[idx2] / 3;
            
            total += array1[idx1] + array2[idx2] + (int)array3[idx3];
        }
    }
    
    return total;
}

int main(void) {
    /* Initialize CPU features - triggers driver cache detection */
    init_cpu_features();
    
    /* Perform cache-sensitive computation */
    int result = 0;
    
    /* Base stride depends on CPU features */
    int base_stride = 1;
    
    /* Conditional compilation based on optimization and CPU features */
#ifdef __OPTIMIZE__
    #if USE_SSE_FEATURES
    base_stride = 2;
    #endif
    
    #if USE_AVX_FEATURES
    base_stride = 4;
    #endif
    
    #if USE_AVX512_FEATURES
    base_stride = 8;
    #endif
#endif
    
    /* Main computation */
    if (func_ptr != NULL) {
        result += func_ptr(base_stride, 100000);
    }
    
    result += cache_sensitive_computation(base_stride);
    
    /* Use result to prevent dead code elimination */
    printf("CPU Cache Test Result: %d\n", result);
    
    /* Additional CPU feature reporting */
    printf("Compiled with: ");
    
#ifdef __SSE__
    printf("SSE ");
#endif
#ifdef __SSE2__
    printf("SSE2 ");
#endif
#ifdef __SSE3__
    printf("SSE3 ");
#endif
#ifdef __SSSE3__
    printf("SSSE3 ");
#endif
#ifdef __SSE4_1__
    printf("SSE4.1 ");
#endif
#ifdef __SSE4_2__
    printf("SSE4.2 ");
#endif
#ifdef __AVX__
    printf("AVX ");
#endif
#ifdef __AVX2__
    printf("AVX2 ");
#endif
#ifdef __AVX512F__
    printf("AVX512F ");
#endif
    printf("\n");
    
    return 0;
}
