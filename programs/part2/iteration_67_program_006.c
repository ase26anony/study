/* test_cpuid_cache.c - Trigger GCC driver CPUID cache detection */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Large arrays to encourage cache consideration */
static volatile int array1[1024 * 1024];
static volatile int array2[1024 * 1024];
static volatile int array3[512 * 512];

/* Function pointers to prevent optimization */
typedef int (*compute_func_t)(int);
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

#if defined(__AVX512F__) || defined(__AVX512VL__)
#define USE_AVX512_FEATURES 1
#else
#define USE_AVX512_FEATURES 0
#endif
#endif /* __OPTIMIZE__ */

/* Different computation patterns based on CPU features */
int compute_simple(int iterations) {
    int sum = 0;
    /* Stride-1 access pattern */
    for (int i = 0; i < iterations; i++) {
        sum += array1[i & 0xFFFF];
    }
    return sum;
}

int compute_stride2(int iterations) {
    int sum = 0;
    /* Stride-2 access pattern */
    for (int i = 0; i < iterations; i += 2) {
        sum += array2[i & 0x7FFF];
    }
    return sum;
}

int compute_stride4(int iterations) {
    int sum = 0;
    /* Stride-4 access pattern */
    for (int i = 0; i < iterations; i += 4) {
        sum += array3[i & 0x3FFF];
    }
    return sum;
}

int compute_stride8(int iterations) {
    int sum = 0;
    /* Stride-8 access pattern */
    for (int i = 0; i < iterations; i += 8) {
        sum += array1[i & 0x1FFF];
    }
    return sum;
}

int compute_stride16(int iterations) {
    int sum = 0;
    /* Stride-16 access pattern */
    for (int i = 0; i < iterations; i += 16) {
        sum += array2[i & 0xFFF];
    }
    return sum;
}

/* Initialize arrays with pseudo-random data */
void init_arrays(void) {
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
    /* Force CPU initialization - this triggers driver's __builtin_cpu_init */
    __builtin_cpu_init();
    
    /* Volatile variables to prevent optimization */
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
    
    /* Initialize arrays */
    init_arrays();
    
    /* Choose computation pattern based on detected features */
    int iterations = 1000000;
    int result = 0;
    
    /* Complex conditional to force driver to evaluate all paths */
    if (has_sse2) {
        func_ptr = compute_simple;
    }
#ifdef __OPTIMIZE__
    /* Force driver to consider these during compilation */
    if (has_sse3 && USE_SSE_FEATURES) {
        func_ptr = compute_stride2;
    }
    if (has_ssse3 && USE_SSE_FEATURES) {
        func_ptr = compute_stride4;
    }
    if (has_sse4_1 && USE_SSE_FEATURES) {
        func_ptr = compute_stride8;
    }
    if (has_sse4_2 && USE_SSE_FEATURES) {
        func_ptr = compute_stride16;
    }
#endif
    
    if (has_avx) {
#ifdef __OPTIMIZE__
        if (USE_AVX_FEATURES) {
            /* Mix patterns for AVX */
            result += compute_simple(iterations / 2);
            result += compute_stride4(iterations / 2);
        }
#endif
        func_ptr = compute_stride2;
    }
    
    if (has_avx2) {
#ifdef __OPTIMIZE__
        if (USE_AVX_FEATURES) {
            result += compute_stride8(iterations / 3);
            result += compute_stride16(iterations / 3);
        }
#endif
        func_ptr = compute_stride4;
    }
    
    if (has_avx512f) {
#ifdef __OPTIMIZE__
        if (USE_AVX512_FEATURES) {
            /* Use all patterns for AVX512 */
            result += compute_simple(iterations / 5);
            result += compute_stride2(iterations / 5);
            result += compute_stride4(iterations / 5);
            result += compute_stride8(iterations / 5);
            result += compute_stride16(iterations / 5);
        }
#endif
        func_ptr = compute_stride8;
    }
    
    /* Execute the selected function if any */
    if (func_ptr != NULL) {
        result += func_ptr(iterations);
    } else {
        /* Default computation */
        result = compute_simple(iterations);
    }
    
    /* Additional feature checks for more CPUID paths */
    volatile int has_aes = __builtin_cpu_supports("aes");
    volatile int has_pclmul = __builtin_cpu_supports("pclmul");
    volatile int has_rdrand = __builtin_cpu_supports("rdrand");
    volatile int has_fma = __builtin_cpu_supports("fma");
    volatile int has_bmi = __builtin_cpu_supports("bmi");
    volatile int has_bmi2 = __builtin_cpu_supports("bmi2");
    
    /* Use results to prevent dead code elimination */
    result += has_aes + has_pclmul + has_rdrand + has_fma + has_bmi + has_bmi2;
    
    printf("Result: %d\n", result);
    return 0;
}
