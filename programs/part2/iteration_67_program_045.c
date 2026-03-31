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

/* CPU feature checks that trigger __builtin_cpu_init */
static void init_cpu_features(void) {
    /* This forces GCC driver to initialize CPU detection */
    __builtin_cpu_init();
    
    /* Volatile to prevent optimization */
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_sse3 = __builtin_cpu_supports("sse3");
    volatile int has_ssse3 = __builtin_cpu_supports("ssse3");
    volatile int has_sse4_1 = __builtin_cpu_supports("sse4.1");
    volatile int has_sse4_2 = __builtin_cpu_supports("sse4.2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    volatile int has_avx512f = __builtin_cpu_supports("avx512f");
    
    /* Cache-specific feature checks */
    volatile int has_clflushopt = __builtin_cpu_supports("clflushopt");
    volatile int has_clwb = __builtin_cpu_supports("clwb");
    volatile int has_prefetchwt1 = __builtin_cpu_supports("prefetchwt1");
    
    /* Use results to influence function pointer */
    if (has_avx512f) {
        func_ptr = (compute_func_t)0x1;
    } else if (has_avx2) {
        func_ptr = (compute_func_t)0x2;
    } else if (has_avx) {
        func_ptr = (compute_func_t)0x3;
    } else if (has_sse4_2) {
        func_ptr = (compute_func_t)0x4;
    } else {
        func_ptr = (compute_func_t)0x5;
    }
}

/* Cache-sensitive computation with varying strides */
static uint64_t cache_sensitive_compute(int stride) {
    uint64_t sum = 0;
    const int size = sizeof(array_a) / sizeof(array_a[0]);
    
    /* Different access patterns based on stride */
    for (int i = 0; i < size; i += stride) {
        array_a[i] = i * 3;
        array_b[i] = i * 7;
        sum += array_a[i] + array_b[i];
    }
    
    /* Secondary computation with different array */
    const int c_size = sizeof(array_c) / sizeof(array_c[0]);
    for (int i = 0; i < c_size; i += (stride * 2)) {
        array_c[i] = i * 11;
        sum += array_c[i];
    }
    
    return sum;
}

/* Different computation paths based on CPU features */
#ifdef __OPTIMIZE__
static int compute_sse_path(int a, int b) {
    volatile int result = 0;
    if (__builtin_cpu_supports("sse2")) {
        result = a * b + (a >> 4) - (b << 2);
    }
    return result;
}

static int compute_avx_path(int a, int b) {
    volatile int result = 0;
    if (__builtin_cpu_supports("avx")) {
        result = (a * 3) + (b * 7) - (a & b);
    }
    return result;
}

static int compute_generic_path(int a, int b) {
    return a + b * 2;
}
#endif

int main(void) {
    uint64_t checksum = 0;
    
    /* Initialize CPU detection - triggers driver's CPUID logic */
    init_cpu_features();
    
    /* Choose stride based on (volatile) function pointer */
    int stride = 1;
    if ((uintptr_t)func_ptr == 0x1) stride = 16;  /* AVX512 */
    else if ((uintptr_t)func_ptr == 0x2) stride = 8;   /* AVX2 */
    else if ((uintptr_t)func_ptr == 0x3) stride = 4;   /* AVX */
    else if ((uintptr_t)func_ptr == 0x4) stride = 2;   /* SSE4.2 */
    else stride = 1;                                   /* Generic */
    
    /* Perform cache-sensitive computation */
    checksum = cache_sensitive_compute(stride);
    
#ifdef __OPTIMIZE__
    /* Conditional compilation that requires CPU feature evaluation */
    volatile int feature_test = 0;
    
    #if defined(__SSE2__)
    if (__builtin_cpu_supports("sse2")) {
        feature_test += compute_sse_path(checksum & 0xFF, (checksum >> 8) & 0xFF);
    }
    #endif
    
    #if defined(__AVX__)
    if (__builtin_cpu_supports("avx")) {
        feature_test += compute_avx_path(checksum & 0xFF, (checksum >> 16) & 0xFF);
    }
    #endif
    
    checksum += feature_test;
#endif
    
    /* Use result to prevent dead code elimination */
    printf("CPU Cache Test Checksum: %llu\n", (unsigned long long)checksum);
    
    /* Additional CPUID-triggering checks */
    volatile int cache_line_check = 0;
    
    /* Check for cache line size related features */
    if (__builtin_cpu_supports("clflushopt")) {
        cache_line_check |= 0x01;
    }
    if (__builtin_cpu_supports("clwb")) {
        cache_line_check |= 0x02;
    }
    if (__builtin_cpu_supports("prefetchwt1")) {
        cache_line_check |= 0x04;
    }
    
    printf("Cache Features: 0x%02x\n", cache_line_check);
    
    return (int)(checksum % 256);
}
