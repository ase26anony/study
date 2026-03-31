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

/* Different computation functions for different CPU features */
static int compute_sse(int a, int b) {
    volatile int result = 0;
    /* Simple SSE-like operation */
    for (int i = 0; i < 1024; i++) {
        result += (a * i) + (b * (i % 16));
    }
    return result;
}

static int compute_avx(int a, int b) {
    volatile int result = 0;
    /* AVX-style wider operations */
    for (int i = 0; i < 2048; i += 8) {
        result += (a * i) + (b * (i % 32));
    }
    return result;
}

static int compute_basic(int a, int b) {
    volatile int result = 0;
    for (int i = 0; i < 512; i++) {
        result += (a * i) + (b * (i % 8));
    }
    return result;
}

/* Cache-sensitive memory access patterns */
static uint64_t cache_sensitive_access(int stride, int iterations) {
    volatile uint64_t sum = 0;
    const int array_size = sizeof(array_a) / sizeof(array_a[0]);
    
    /* Non-constant stride access to prevent optimization */
    for (int i = 0; i < iterations; i++) {
        int idx = (i * stride) % array_size;
        sum += array_a[idx] + array_b[idx] + array_c[idx % (512 * 512)];
        
        /* Prevent compiler from optimizing away the loop */
        array_a[idx] = (int)(sum & 0xFFFF);
        array_b[idx] = (int)((sum >> 16) & 0xFFFF);
    }
    
    return sum;
}

/* Initialize arrays with pseudo-random data */
static void init_arrays(void) {
    for (size_t i = 0; i < sizeof(array_a)/sizeof(array_a[0]); i++) {
        array_a[i] = (int)(i * 1103515245ULL + 12345) & 0x7FFF;
    }
    for (size_t i = 0; i < sizeof(array_b)/sizeof(array_b[0]); i++) {
        array_b[i] = (int)(i * 1664525ULL + 1013904223) & 0x7FFF;
    }
    for (size_t i = 0; i < sizeof(array_c)/sizeof(array_c[0]); i++) {
        array_c[i] = (int)(i * 214013ULL + 2531011) & 0x7FFF;
    }
}

int main(void) {
    /* Force CPU initialization - this triggers CPUID in GCC driver */
    __builtin_cpu_init();
    
    /* Store CPU feature checks in volatile to prevent optimization */
    volatile int has_sse = __builtin_cpu_supports("sse");
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_sse3 = __builtin_cpu_supports("sse3");
    volatile int has_ssse3 = __builtin_cpu_supports("ssse3");
    volatile int has_sse4_1 = __builtin_cpu_supports("sse4.1");
    volatile int has_sse4_2 = __builtin_cpu_supports("sse4.2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    volatile int has_avx512f = __builtin_cpu_supports("avx512f");
    
    /* Choose function based on CPU features */
    if (has_avx512f) {
        func_ptr = compute_avx;
    } else if (has_avx || has_avx2) {
        func_ptr = compute_avx;
    } else if (has_sse4_1 || has_sse4_2) {
        func_ptr = compute_sse;
    } else {
        func_ptr = compute_basic;
    }
    
    /* Initialize arrays */
    init_arrays();
    
    /* Choose stride based on CPU features to test different cache behaviors */
    int stride = 1;
    if (has_sse2) stride = 2;
    if (has_sse4_1) stride = 4;
    if (has_avx) stride = 8;
    if (has_avx2) stride = 16;
    if (has_avx512f) stride = 32;
    
    /* Perform cache-sensitive computation */
    uint64_t cache_sum = cache_sensitive_access(stride, 100000);
    
    /* Compute using selected function */
    int compute_result = 0;
    if (func_ptr) {
        compute_result = func_ptr((int)(cache_sum & 0xFF), 
                                 (int)((cache_sum >> 8) & 0xFF));
    }
    
    /* Final result that depends on all computations */
    volatile uint64_t final_result = cache_sum + compute_result;
    
    /* Use result to prevent dead code elimination */
    printf("CPU Feature Check - SSE:%d SSE2:%d SSE3:%d SSE4.1:%d AVX:%d AVX2:%d AVX512F:%d\n",
           has_sse, has_sse2, has_sse3, has_sse4_1, has_avx, has_avx2, has_avx512f);
    printf("Computed checksum: 0x%016llx\n", (unsigned long long)final_result);
    
    return (int)(final_result % 256);
}

/* Additional compilation-time CPU feature checks */
/* These force the driver to evaluate CPUID during compilation */

#ifdef __OPTIMIZE__
/* Force evaluation of various CPU feature macros */
static volatile int compile_time_features = 0;

void __attribute__((constructor)) init_compile_time_check(void) {
    /* These checks happen at compile time, forcing driver CPUID evaluation */
#if USE_SSE_FEATURES
    compile_time_features |= 0x01;
#endif
#if USE_AVX_FEATURES
    compile_time_features |= 0x02;
#endif
#if USE_AVX512_FEATURES
    compile_time_features |= 0x04;
#endif
    
    /* Additional architecture-specific checks */
#ifdef __tune_core2__
    compile_time_features |= 0x10;
#endif
#ifdef __tune_nehalem__
    compile_time_features |= 0x20;
#endif
#ifdef __tune_sandybridge__
    compile_time_features |= 0x40;
#endif
#ifdef __tune_skylake__
    compile_time_features |= 0x80;
#endif
#ifdef __tune_znver1__
    compile_time_features |= 0x100;
#endif
#ifdef __tune_znver2__
    compile_time_features |= 0x200;
#endif
}
#endif /* __OPTIMIZE__ */
