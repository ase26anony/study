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
static compute_func_t func_ptr = NULL;

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
static int compute_simple(int a, int b) {
    return a + b;
}

static int compute_complex(int a, int b) {
    return (a * b) + (a ^ b);
}

/* Initialize CPU detection - this triggers the driver's CPUID logic */
static void init_cpu_features(void) {
    /* This builtin forces CPUID initialization in the driver */
    __builtin_cpu_init();
    
    /* Store feature checks in volatile to prevent optimization */
    volatile int has_sse2 = __builtin_cpu_supports("sse2");
    volatile int has_sse3 = __builtin_cpu_supports("sse3");
    volatile int has_ssse3 = __builtin_cpu_supports("ssse3");
    volatile int has_sse4_1 = __builtin_cpu_supports("sse4.1");
    volatile int has_sse4_2 = __builtin_cpu_supports("sse4.2");
    volatile int has_avx = __builtin_cpu_supports("avx");
    volatile int has_avx2 = __builtin_cpu_supports("avx2");
    volatile int has_avx512f = __builtin_cpu_supports("avx512f");
    
    /* Cache-specific feature checks (these may trigger cache detection) */
    volatile int has_xsave = __builtin_cpu_supports("xsave");
    volatile int has_xsaveopt = __builtin_cpu_supports("xsaveopt");
    volatile int has_clflush = __builtin_cpu_supports("clflush");
    volatile int has_clflushopt = __builtin_cpu_supports("clflushopt");
    
    /* Choose function based on detected features */
    if (has_avx512f) {
        func_ptr = compute_complex;
    } else if (has_avx2) {
        func_ptr = compute_complex;
    } else if (has_sse4_2) {
        func_ptr = compute_complex;
    } else {
        func_ptr = compute_simple;
    }
    
    /* Use the volatile variables to prevent dead code elimination */
    if (has_sse2) array_a[0] = 1;
    if (has_avx) array_b[0] = 1;
}

/* Cache-sensitive memory access patterns */
static uint64_t cache_sensitive_access(int stride, int iterations) {
    uint64_t sum = 0;
    const int array_size = sizeof(array_a) / sizeof(array_a[0]);
    
    /* Different access patterns based on stride */
    for (int i = 0; i < iterations; i++) {
        int idx = (i * stride) % array_size;
        
        /* Use function pointer to prevent optimization */
        if (func_ptr) {
            sum += func_ptr(array_a[idx], array_b[idx]);
        }
        
        /* Cross-array access to potentially use different cache levels */
        array_c[idx % (sizeof(array_c)/sizeof(array_c[0]))] = 
            array_a[idx] ^ array_b[idx];
    }
    
    return sum;
}

/* Main function with architecture-specific code paths */
int main(void) {
    uint64_t checksum = 0;
    
    /* Initialize CPU features - triggers driver's CPUID cache detection */
    init_cpu_features();
    
    /* Test different strides to exercise different cache behaviors */
    const int strides[] = {1, 2, 4, 8, 16, 32, 64, 128};
    const int num_strides = sizeof(strides) / sizeof(strides[0]);
    
    /* Architecture-specific compilation blocks */
#ifdef __OPTIMIZE__
    /* These blocks force the driver to evaluate CPU features */
    #if USE_SSE_FEATURES
    /* SSE-specific initialization */
    for (int i = 0; i < 1000; i++) {
        array_a[i] = i;
        array_b[i] = i * 2;
    }
    #endif
    
    #if USE_AVX_FEATURES
    /* AVX-specific pattern */
    for (int i = 0; i < 2000; i += 8) {
        array_a[i] = i * 3;
        array_b[i] = i * 4;
    }
    #endif
    
    #if USE_AVX512_FEATURES
    /* AVX-512 specific pattern */
    for (int i = 0; i < 4000; i += 16) {
        array_a[i] = i * 5;
        array_b[i] = i * 6;
    }
    #endif
#endif /* __OPTIMIZE__ */
    
    /* Perform cache-sensitive computations */
    for (int s = 0; s < num_strides; s++) {
        checksum += cache_sensitive_access(strides[s], 10000);
    }
    
    /* Additional CPU feature checks in main execution path */
    volatile int cpu_features = 0;
    
    /* Check various CPU features that might influence cache detection */
    if (__builtin_cpu_supports("sse")) cpu_features |= 1;
    if (__builtin_cpu_supports("sse2")) cpu_features |= 2;
    if (__builtin_cpu_supports("sse3")) cpu_features |= 4;
    if (__builtin_cpu_supports("ssse3")) cpu_features |= 8;
    if (__builtin_cpu_supports("sse4.1")) cpu_features |= 16;
    if (__builtin_cpu_supports("sse4.2")) cpu_features |= 32;
    if (__builtin_cpu_supports("avx")) cpu_features |= 64;
    if (__builtin_cpu_supports("avx2")) cpu_features |= 128;
    
    /* Use the features to influence computation */
    checksum ^= cpu_features;
    
    printf("CPU Features: 0x%x\n", cpu_features);
    printf("Checksum: 0x%016llx\n", (unsigned long long)checksum);
    
    return (checksum & 0xFF);
}
